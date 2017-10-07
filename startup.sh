#!/bin/sh
# make sure we aren't running already
CURTIME=`date '+%s'`
echo $CURTIME
what=`basename $0`
for p in `ps h -o pid -C $what`; do
        if [ $p != $$ ]; then
                exit 0
        fi  
done

export PYTHONPATH=/home/pi/iov
export IOVPATH=/home/pi/iov
export LIBRARY_PATH=$LIBRARY_PATH:/home/pi/data_iov/lib:/home/pi/iov/image
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/pi/data_iov/lib:/home/pi/iov/image
raspi-gpio set 24 op
interval=0
while [ 1 ]
do
	raspi-gpio set 24 dl
	res=`ps aux | grep net_control | grep python`
	if [ $? != 1 ] 
	then
        	echo 'net control already start'
	else
		echo 'restart net control'
		nohup python /home/pi/iov/network/net_control.py > /home/pi/net_control.log 2>&1 &
	fi
	
	res=`ps aux | grep drivermonitor | grep image`
	if [ $? != 1 ] 
	then
        	echo 'driver monitor already start'
	else
		echo 'restart driver monitor'
		cd /home/pi/iov/image
		#nohup /home/pi/iov/image/drivermonitor 600 > /home/pi/data_iov/log/drivermonitor.log 2>&1 &
		cd
	fi
	raspi-gpio set 24 dh
	sleep 5

	res=`ps aux | grep sensor_data | grep python`
	if [ $? != 1 ] 
	then
        	echo 'sensor data already start'
	else
		echo 'restart sensor data'
		#nohup python /home/pi/iov/sensor/sensor_data.py > /home/pi/data_iov/log/sensor_data.log 2>&1 &
	fi
	LASTTIME=`date '+%s'`
	interval=`expr $LASTTIME - $CURTIME`
done
#nohup python /home/pi/GPIOtest.py > /home/pi/gpio.log 2>&1 &
