import RPi.GPIO as GPIO
import time
LED = 24
GPIO.setmode(GPIO.BCM)
GPIO.setup(LED, GPIO.OUT)
print("test!")
try:
	while 1:
		GPIO.output(LED, False)
		time.sleep(1)
		GPIO.output(LED, True)
		time.sleep(1)
except KeyboardInterrupt:
	GPIO.cleanup()
