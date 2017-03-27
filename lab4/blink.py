import RPi.GPIO as gpio
import time

gpio.cleanup()
gpio.setmode(gpio.BCM)
gpio.setup(16, gpio.OUT)

t = 0.00001

while(1):
	gpio.output(16, gpio.HIGH)
	time.sleep(t)	
	gpio.output(16, gpio.LOW)
	time.sleep(t)
