#!/usr/bin/env python
import pika

class rpc_server(object):
    def __init__(self, username, passwd, hostip, queuename):
        self.rabbit_user = username
        self.rabbit_passwd = passwd
        self.hostip = hostip
        self.queuename = queuename

        credential = pika.PlainCredentials(self.rabbit_user, self.rabbit_passwd)
        
        connection = pika.BlockingConnection(pika.ConnectionParameters(
                host=self.hostip , port=5671 ,credentials=credential , ssl = True))
        
        #connection = pika.BlockingConnection(pika.ConnectionParameters(
        #        host=self.hostip ,credentials=credential))
        
        self.channel = connection.channel()
        
        self.channel.queue_declare(queue=self.queuename)
    
    def do_something(self, response):
        return (result, response)
    
    def on_request(self, ch, method, props, body):
        (result, response) = self.do_something(body)
        ch.basic_publish(exchange='',
                         routing_key=props.reply_to,
                         properties=pika.BasicProperties(correlation_id = \
                                                             props.correlation_id),
                         body=response)
        ch.basic_ack(delivery_tag = method.delivery_tag)
        return result

    def start(self):
        self.channel.basic_qos(prefetch_count=1)
        self.channel.basic_consume(self.on_request, queue=self.queuename)
        print(" [x] Awaiting RPC requests")
        self.channel.start_consuming()
        return 
