import os
import sys
import argparse
import commands
import logging
import logcreator
import time

def VPN_connect():
	res, info = commands.getstatusoutput('ip addr show ppp0')
	if res == 0:
		return 0
	print 'restart  ipsec'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo service ipsec restart')

	print 'restart xl2tpd'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo service xl2tpd restart')

	print 'ipsec up'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo ipsec up myvpn')

	print 'create tunnel'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo /home/pi/L2TP.sh')

def VPN_disconnect():
	print 'delete tunnel'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo /home/pi/deleteConnect.sh')
		print res,info

	print 'ipsec down'
	res = 1
	while(res != 0):
		res, info = commands.getstatusoutput('sudo ipsec down myvpn')
		print res,info

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='VPN network control!')
	parser.add_argument('action', type = str, help = '\'c\' for connect or \'d\' for disconnect')
	args = parser.parse_args()

	if args.action == 'c':
		VPN_connect()
	elif args.action == 'd':
		VPN_disconnect()
	else:
		print 'input error!'
	VPN_connect()
