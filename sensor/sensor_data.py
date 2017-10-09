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

basedir = os.environ['IOVPATH']
#log_path = os.path.join(basedir, 'raspi_IOV.log')
log_path = '/home/pi/data_iov/log/raspi_IOV.log'
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
					continue
				nums = rawdata.split(" ")[1:-1]
				if len(nums) != 16:
					continue
				ticks = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime()) 
				for i in range(0,16):
					nums[i] = float(nums[i])
				 
				if is_start == 1:
					nums16 = 1
				else:
					nums16 = 0		
				jsonFormat = {'acx': nums[0],'acy':nums[1] ,'acz':nums[2] ,'grx':nums[3] ,\
								'gry':nums[4] ,'grz':nums[5] ,'agx':nums[6] ,'agy':nums[7] ,\
								'agz':nums[8] ,'mag':nums[9] ,'pap':nums[10] ,'php':nums[11] ,\
								'lng':nums[12] ,'lat':nums[13] ,'gph':nums[14] ,'gpv':nums[15] ,\
								'is_start':nums16,'t':ticks}
				message = {"vid":configdata['vid'], "result":str(jsonFormat), "tag":1}
				print message
				if self.offlineMode:
					fw.write(str(jsonFormat))
					fw.write('\n')
				else:
					self.start(str(message))
				is_start = 0
				time.sleep(0.8)
		except KeyboardInterrupt:
			if self.offlineMode:
				fw.close()
		return

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='capture sensor via serial from MCU and push them to server side!')
	parser.add_argument('-s', dest='serialport', type =str, help='serialport of the MCU')
	parser.add_argument('-host', dest = 'hostip', type = str, help = 'host IP of the server', default = '101.200.181.242')
	args = parser.parse_args()

	factory = sensor_data("iov", "iovpro", args.hostip, "normal_message", args.serialport)
	factory.processing_data()
