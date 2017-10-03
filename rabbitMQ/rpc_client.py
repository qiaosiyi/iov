#!/usr/bin/env python
import pika
import uuid

class rpc_client(object):
#username: rabbitmq user
#passwd: user passwd
#hostip: rabbitmq server ip
#queuename: the queue to connnect
#ack: 
    def __init__(self, username, passwd, hostip, queuename, ack = True):
        self.rabbit_user = username
        self.rabbit_passwd = passwd
        self.hostip = hostip
        self.queuename = queuename
        self.ack = ack

        credential = pika.PlainCredentials(self.rabbit_user , self.rabbit_passwd)

        self.connection = pika.BlockingConnection(pika.ConnectionParameters(
                host=self.hostip, port=5671 , credentials=credential, ssl = True))

        #self.connection = pika.BlockingConnection(pika.ConnectionParameters(
        #        host=self.hostip , credentials=credential))
        
	self.channel = self.connection.channel()

        result = self.channel.queue_declare(exclusive=True)
        self.callback_queue = result.method.queue

        self.channel.basic_consume(self.on_response, no_ack=True,
                                   queue=self.callback_queue)

    def on_response(self, ch, method, props, body):
        if self.corr_id == props.correlation_id:
            self.response = body

    def call(self, message):
        self.response = None
        self.corr_id = str(uuid.uuid4())
        self.channel.basic_publish(exchange='',
                                   routing_key=self.queuename,
                                   properties=pika.BasicProperties(
                                         reply_to = self.callback_queue,
                                         correlation_id = self.corr_id,
                                         ),
                                   body=message)
        if self.ack:
            while self.response is None:
                self.connection.process_data_events()
            return self.response
        else:
            return -1

    def start(self, message =""):
        response = self.call(message)
        return self.do_something(response)

    def do_something(self, response):
        return True




