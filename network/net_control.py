

import serial
import time
import sys
import argparse
import os
import commands
import logging
import logcreator
import RPi.GPIO as GPIO
import network.VPN_connect as vpn
import json

con_time = 10
serverIP= '101.200.181.242'
eth = 'eth1'
basedir = os.environ['IOVPATH']
#log_path = os.path.join(basedir, 'raspi_IOV.log')
log_path = "/home/pi/data_iov/log/raspi_IOV.log"
logger = logcreator.createLogger("NetLogger", log_path, logging.DEBUG)
json_data=open('/home/pi/data_iov/iov.config', 'r').read()
configdata = json.loads(json_data)
vpncert = configdata['vpncert']
class networkControl(object):
	"""docstring for networkControl"""
	def __init__(self, serialport='/dev/ttyUSB0', baudrate=115200, timeout=0.1, rtscts=0):
		super(networkControl, self).__init__()
		self.serialport = serialport
		self.baudrate = baudrate
		self.timeout = timeout
		self.rtscts = rtscts
		self.checkSerialPort()
		logger.info("create serial connect--------\n"+ 
				"\tserial port:"+self.serialport+"\n"+
				"\tbaudrate:"+str(self.baudrate)+"\n"+
				"\ttimeout:"+str(self.timeout)+"\n"+
				"\trtscts:"+str(self.rtscts))
		

	def checkSerialPort(self):
		res, info = commands.getstatusoutput('dmesg | grep GSM.*ttyUSB')
		if res:
			logger.critical("4G module not found!")
			#restart 4G module
			#time.sleep(10)
			self.restart4G()
			return False
			#raise Exception('4G module not found')
		for i in range(len(info)):
			if info[i] =='y' and info[i+1] == 'U' and info[i+2] == 'S' and info[i+3] == 'B':
				port = info[i-2:i+4]+str(int(info[i+4])-2)
		
		self.serialport = '/dev/'+port
		logger.info("set 4G module serial port to " + self.serialport)
		return True

	def checkVPNConnect(self):
		connected = False
		checktime = 1 
		#print 'Vpn check'
		res, info = commands.getstatusoutput('ifconfig | grep tun')
	
		if res != 0:
			logger.debug('cannot find tun interface\n' + info)
			return False
		logger.debug('test VPN ping')
		while (not connected and checktime > 0):
			connected = os.system('ping 10.8.0.1 -qc 3 -I tun0')
			# ping 10.8.0.1 -qc 3 -I tun0
			checktime -=1
		if connected:
			logger.debug('ping VPN fail')
		return not connected
	
	def restart4G(self):
		logger.info('restart the power of 4G')
		
		print 'restart4G().'
		LED = 18 
		GPIO.setmode(GPIO.BCM)
		GPIO.setup(LED, GPIO.OUT)
		GPIO.output(LED, True)
		time.sleep(1)
		GPIO.output(LED, False)
		time.sleep(1)
		GPIO.cleanup()
	
	def checkNetConnect(self):
		connected = False
		checktime = 1 
		while (not connected and checktime > 0):
			connected = os.system('ping '+ serverIP +' -qc 3 -I ' + eth)
			#ping 101.200.181.242 -qc 3 -I eth0
			checktime -=1
		if connected:
			logger.debug('ping fail')
		else:
			logger.debug('ping ok')
		return not connected
	
	def connectVPN(self):
		if self.checkVPNConnect():
			logger.debug('VPN already connect')
			print "VPN already connect"
			return
		else:
			print "Check resault:VPN is down."

		logger.info('try to connect VPN')
		res, info = commands.getstatusoutput('ps -aux | grep raspi.ovpn')
		info = info.split()
		if info[0] == 'pi':#openvpn deamon thread is not running 
			print " waiting for date & time sync...30 seconds..."
			for i in range(30):
				time.sleep(1)
				print i,"waiting for sync.."
			logger.debug('kill all openvpn service')
			os.system('sudo killall openvpn')
			print "clean all openvpn and restart openvpn."
			res, info = commands.getstatusoutput('sudo openvpn --daemon --config ' + vpncert)
			logger.info("connect to VPN")
			print "have try to connect to VPN"
	
		   #if self.checkVPNConnect():
			#	logger.debug('VPN already connect')
			#	print "VPN already connect"
	def connect4G(self):
		#print "serialport:",self.serialport
		#print "baudrate:",self.baudrate
		#print "timeout:",self.timeout
		#print "rtscts:",self.rtscts
		try:
			ser = serial.Serial(self.serialport, baudrate=self.baudrate, timeout=self.timeout, rtscts=self.rtscts)

			ser.write(b'ATE1\r')
			response = ser.read(256)
			# print "print:ATE1:",response
			logger.debug(response)
			
			ser.write(b'AT^SYSINFO\r')
			response = ser.read(256)
			# print "print:AT^SYSINFO",response
			logger.debug(response)
			
			flag = con_time
			while(flag):
				ser.write(b'AT+CGACT=1,1\r')
			#	time.sleep(0.5)
				response = ser.read(256)
				# print "print:AT+CGACT=1,1:",response
				#print response
				if response.find("ERROR") == -1:
					logger.info(response)
					# print "print:ERROR==-1:",response
					flag = 0
				else:
					flag = flag - 1
					# print "print:flag:",flag
			flag = con_time
			while(flag):
				ser.write(b'AT+ZGACT=0,1\r')
				response = ser.read(256)
				# print "print:AT+ZGACT=0,1:",response
				#print response
				ser.write(b'AT+ZGACT=1,1\r')
			#	time.sleep(0.5)
				response = ser.read(256)
				# print "print:AT+ZGACT=1,1:",response
				#print response
				if response.find("+ZCONSTAT: 1,1") != -1:
					logger.debug(response)
					# print "print:+ZCONSTAT: 1,1:",response
					flag = 0
				else:
					flag = flag - 1
		except:
			self.checkSerialPort()
			time.sleep(5)
		#finally:
			#ser.close()
		return self.checkNetConnect()

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='check network status and control network')
	parser.add_argument('-s', dest='serialport', default = '/dev/', type =str, help='serialport to the network device')
	parser.add_argument('-b', dest='baudrate',default=115200,type=int, help='set baudrate to serialport')
	parser.add_argument('-t', dest='timeout',default=.1,type=float, help='set timeout to serialport connection')
	parser.add_argument('-r', dest='rtscts',default=0,type=int, help='set rtscts to serialport')
	parser.add_argument('-i', dest='interval', default=60, type=int, help='time interval to check the network status')
	args = parser.parse_args()

	nc = networkControl(args.serialport, args.baudrate, args.timeout, args.rtscts)
	errortime = 0
	while 1:
		print
		print " [x]check network connection"
		connected = nc.checkNetConnect()
		if connected:
			print	
			print " [.]network is connected"
			errortime = 0	
			nc.connectVPN()
		else:
			print
			print " [x]cannot connect to network, try 4G connection..."
			if nc.connect4G():
				print
				print " [.]connected to 4G network"
				
			else:
				errortime = errortime + 1
				print "errortime:",errortime
				print " [x]failed to connection"
				print "4G is down, try to kill all openvpn"
				res, info = commands.getstatusoutput('ps -aux | grep raspi.ovpn')
				info = info.split()
				if info[0] == 'root':
					os.system('sudo killall openvpn')
					print "killing all openvpn."
				if info[0] == 'pi':
					print "openvpn has stop."
				if errortime > 9:
					nc.restart4G()
					print "restarting 4G module."
					errortime = 0
				continue
		for i in range(args.interval):
			time.sleep(1)
			print i,"main "
