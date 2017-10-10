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
log_path = "/home/pi/data_iov/log/raspi_IOV.log"
logger = logcreator.createLogger("SendMessageLogger", log_path, logging.INFO)

class send_message(rpc_client):
	"""docstring for sensor_data"""
	def __init__(self, username, passwd, hostip, queuename):
		#try:
		super(send_message, self).__init__(username, passwd, hostip, queuename, False)
		#except BaseException:
		#	self.offlineMode = True
		#	logger.critical('fail to connect server')



if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='sending message to server!')
	parser.add_argument('-m', dest='message', type =str, help='message to send', required = True)
	#parser.add_argument('-f', dest='filepath', type = str, help = 'file to send')
	#parser.add_argument('-t', dest='type', type = int, help = 'type of message', required = True)
	parser.add_argument('-host', dest = 'hostip', type = str, help = 'host IP of the server', default = '101.200.181.242')
	args = parser.parse_args()

	factory = send_message("iov", "iovpro", args.hostip, "normal_message")
	json_data=open('/home/pi/data_iov/iov.config', 'r').read()
	data = json.loads(json_data)
	ticks = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())
	message = eval(args.message)
	print message
	if message['tag'] == 1:
		print 'send message'
		payload = {"vid":data['vid'], "message":message['result'], "time":ticks, "tag":message['tag']}

	elif message['tag'] == 3:
		print 'send driver ID'
		payload = {"did":message['did'], "time":ticks, "vid":data['vid'], 'tag':message['tag']}

	elif message['tag'] == 2:
		print 'send file'
		fr = open(message['filepath'], 'rb')
		content = fr.read()
		fr.close()
		fname = os.path.basename(message['filepath'])
		payload = {'file': content, 'vid':data['vid'], 'filesize':len(content), 'filename':fname,"time":ticks,"tag":message['tag']}

	elif message['tag'] == 4:
		fr = open(message['filepath'], 'rb')
		content = fr.read()
		fr.close()
		fname = os.path.basename(message['filepath'])
		payload = {'file': content, 'vid':data['vid'], 'filesize':len(content), 'filename':fname,"time":ticks,"tag":message['tag'], "fatigue":message['fatigue']}

	elif message['tag'] == 5:
		fr = open(message['filepath'], 'rb')
		content = fr.read()
		fr.close()
		fname = os.path.basename(message['filepath'])
		payload = {'file': content, 'vid':data['vid'], 'filesize':len(content), 'filename':fname,"time":ticks,"tag":message['tag']}

	factory.start(str(payload))
