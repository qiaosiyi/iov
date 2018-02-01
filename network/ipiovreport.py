#!/usr/bin/env python3
#-*- coding:utf-8 -*-

from socket import *
import time
import json
import commands
import re
  
json_data=open('/home/pi/data_iov/iov.config', 'r').read()
configdata = json.loads(json_data)
vid = str(configdata['vid'])
HOST1='101.200.181.242'  
HOST2='115.159.99.74'
PORT=12345  
  
#s1 = socket(AF_INET,SOCK_DGRAM)  
#s1.connect((HOST1,PORT))  
s2 = socket(AF_INET,SOCK_DGRAM)  
s2.connect((HOST2,PORT))  
while True:  
    data = "vid:"+vid
    res, info = commands.getstatusoutput('ifconfig ')
    tmp = re.split('\n',info)
    data = data+"\n"+ tmp[0] +"\n"+ tmp[1] +"\n"+"\n"+tmp[9] +"\n"+ tmp[10] +"\n"    +"\n"+tmp[18] +"\n"+ tmp[19] +"\n"
    
      
 #   s1.sendall(data)  
    s2.sendall(data)
    print data  
    time.sleep(15)
#s1.close()
s2.close()
