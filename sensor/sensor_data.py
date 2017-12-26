#!/usr/bin/env python

import serial
from time import gmtime, strftime
import os
import time
import sys
import commands 
import argparse
import json
from rabbitMQ.rpc_client import rpc_client
import logging
import logcreator
from subprocess import call

info_path = '/home/pi/data_iov/info/driver_id.txt'
basedir = os.environ['IOVPATH']
log_path = '/home/pi/data_iov/log/raspi_IOV.log'
storage_path = '/home/pi/data_iov/sensor'
logger = logcreator.createLogger("SensorLogger", log_path, logging.INFO)
json_data=open('/home/pi/data_iov/iov.config', 'r').read()
configdata = json.loads(json_data)
class sensor_data(rpc_client):
	"""docstring for sensor_data"""
	def __init__(self, username, passwd, hostip, queuename, serialport):
		self.offlineMode = False

		#try:
		super(sensor_data, self).__init__(username, passwd, hostip, queuename, False)
		#except BaseException:
		#	self.offlineMode = True
		#	logger.critical('fail to connect server')

		if serialport:
			self.serialport = serialport
		else:
			self.checkSerialPort()

	def checkSerialPort(self):
		res, info = commands.getstatusoutput('dmesg | grep ch341-uart.*ttyUSB')
		if res:
			logger.critical("sensor MCU not found!")
			raise Exception('sensor MCU not found')
		for i in range(len(info)):
			if info[i] =='y' and info[i+1] == 'U' and info[i+2] == 'S' and info[i+3] == 'B':
				port = info[i-2:i+5]
		
		self.serialport = '/dev/'+port
		logger.info("set sensor MCU serial port to " + self.serialport)
		return 0

	def processing_data(self):

		ser=serial.Serial(self.serialport, baudrate=115200, timeout=.1, rtscts=0)
		is_start = 1
		passtime = 0
		if self.offlineMode:
			fn = strftime("%Y%m%d%H%M%S.sen", gmtime())
			fw = open(fn, 'w')
			print 'openfile ',fn
		try:
			while 1:
				rawdata = ser.read(256)
				if len(rawdata) == 0:
					continue
				if (rawdata[0] != 's' or rawdata[-3] != 'e'):
					#print "broken data!"
					#print rawdata
					continue
				nums = rawdata.split(" ")[1:-1]
				if len(nums) != 16:
					continue
				ticks = time.strftime("%Y-%m-%d_%H.%M.%S", time.localtime()) #get timestamp
				for i in range(0,16):
					nums[i] = float(nums[i])
				 
				if is_start == 1:
					nums16 = 1
				else:
					nums16 = 0		
				fn = str(configdata['vid']) + "_" + ticks + "_SENSOR" #  FileName generate:  e.g. 168804_2017-12-20 11:18:27_SENSOR
				ff = ''
				for k in fn:# translate to shell formate
					if k == ' ' or k == ':':
						ff = ff + '\\' + k
					else:
						ff = ff + k
				if nums[0] == 0.001 and nums[1] == 0.001 and nums[2] == 0.001 : #if power off then shutdown
					should_shut_down = 1
					print "shutdown now!!"
					call("sudo shutdown -h now", shell=True)
					time.sleep(40)
					continue
				else:
					should_shut_down = 0
				if passtime == 1:
					driver_id = open(info_path, "r").readline().rstrip()
					fw = open(fn+".txt", 'w') # write value to file
					fw.write(str(configdata['vid'])+'\n'+\
						ticks+'\n'+\
						driver_id +'\n'+ \
						str(nums[0])+'\n'+str(nums[1])+'\n'+str(nums[2])+'\n'+\
						str(nums[3])+'\n'+str(nums[4])+'\n'+str(nums[5])+'\n'+\
						str(nums[6])+'\n'+str(nums[7])+'\n'+str(nums[8])+'\n'+str(nums[9])+'\n'+\
						str(nums[10])+'\n'+str(nums[11])+'\n'+\
						str(nums[12])+'\n'+str(nums[13])+'\n'+str(nums[14])+'\n'+str(nums[15]))
					fw.close() 
					cmd = "tar -cvf tmp.tar " + ff+".txt" #tar the text file and move it to storage path
					res, info = commands.getstatusoutput(cmd) 
					cmd = "mv tmp.tar " + ff + ".tar"
					res, info = commands.getstatusoutput(cmd)
					cmd = "mv " + ff + ".tar " + storage_path
					res, info = commands.getstatusoutput(cmd)
					cmd = "rm " + ff + ".txt"
					res, info = commands.getstatusoutput(cmd)
					print storage_path + "/"+ fn +".tar file write successfully!"
					passtime = 0
				else :
					passtime = passtime + 1
					print "passtime=",passtime
					continue
					
				
				is_start = 0
				time.sleep(0.8)
		# except KeyboardInterrupt:
		# 	if "fw" in locals():
		# 		fw.close()
		finally: # shutdown process before shutting down program should comfirme that no opened file
			if "fw" in locals():
				fw.close()
			print "close file."
		return

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='capture sensor via serial from MCU')
	parser.add_argument('-s', dest='serialport', type =str, help='serialport of the MCU')
	parser.add_argument('-host', dest = 'hostip', type = str, help = 'host IP of the server', default = '101.200.181.242')
	args = parser.parse_args()
	factory = sensor_data("iov", "iovpro", args.hostip, "normal_message", args.serialport)
	factory.processing_data()
