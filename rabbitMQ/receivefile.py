#!/usr/bin/env python
import socket
import thread
import struct
import hashlib
import ssl
import pprint
import time
import logging
#import dlib
from rpc_server import rpc_server
import argparse
import os

BUFFER_SIZE = 1024
HEAD_STRUCT = '!128sIq32s'   # Structure of file head
hostip = '127.0.0.1'
logging.basicConfig(level=logging.INFO)
CERTPATH="/root/iov/openssl"
class receivefile(rpc_server):
    def __init__(self, username, passwd, hostip, queuename, dir):
        super(receivefile, self).__init__(username, passwd, hostip, queuename)
        self.dir = dir
    
    def upload_file(self, s):
        context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
        context.load_cert_chain(certfile=os.path.join(CERTPATH,'server.crt'), keyfile=os.path.join(CERTPATH,'server.key') , password="iovpro")
        context.load_verify_locations(os.path.join(CERTPATH,'ca.crt'))
        context.verify_mode = ssl.CERT_REQUIRED
        # print "waiting for connection!"
        client_socket, client_address = s.accept()
        
        try:
            connstream = context.wrap_socket(client_socket, server_side=True) 
        except BaseException:
            print "fail to verify client!"
            return
            # continue
    
        print 'connection address:', client_address
        # print "Socket %s:%d has connect" % client_address
    
        try:  
            # pprint.pprint(connstream.getpeercert())  
    
            # Receive file info
            info_struct = struct.calcsize(HEAD_STRUCT)
            file_info = connstream.recv(info_struct)
            file_name2, filename_size, file_size, md5_recv = struct.unpack(HEAD_STRUCT, file_info)
            file_name = file_name2[:filename_size]
            print "file info:"
            print "file name:",file_name
            print "file size:",file_size
            file_name = os.path.join(self.dir, file_name)
            print file_name
            fw = open(file_name, 'wb')
        
            recv_size = 0
            print "Receiving data..."
            while (recv_size < file_size):
                if(file_size - recv_size < BUFFER_SIZE):
                    file_data = connstream.recv(file_size - recv_size)
                    # recv_size = file_size
                else:
                    file_data = connstream.recv(BUFFER_SIZE)
                    # recv_size += BUFFER_SIZE
                recv_size += len(file_data)
                fw.write(file_data)
            fw.close()
            print "Accept success!"
            print "Calculating MD5..."
            
            fw = open(file_name, 'rb')
            md5_cal = hashlib.md5()
            md5_cal.update(fw.read())
            print "  Recevie MD5 : %s" %md5_recv
            print "Calculate MD5 : %s" % md5_cal.hexdigest()
            fw.close()
            print "save file to %s" %file_name
    
        finally:  
            connstream.shutdown(socket.SHUT_RDWR)  
            connstream.close()  
            s.close()
    
    def do_something(self, response):
        print(" [.] upload request(%s)" % response)

        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)  
        s.bind(("",0))  
        s.listen(1)  
	sock_info = s.getsockname()
	print sock_info
        port = sock_info[1]
        # print("port %d is opened!" % port)
        try:
            thread.start_new_thread( self.upload_file, ( s, ) )
        except:
            print "Error: unable to start thread"
        # s.close()
        return (True, str(port))
    
if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='receive from the client')
    parser.add_argument('dir', help='directory to save the files')
    args = parser.parse_args()

    rpc = receivefile("iov", "iovpro", hostip, "upload_queue", args.dir)
    rpc.start()
