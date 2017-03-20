# ----------------
# pwm_calibrate.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Mar 6, 2017
# ----------------

import RPi.GPIO as gpio
import time

## constants ##
p_SERVO1 = 5
p_BTN1 = 17
p_BTN2 = 22

CENTER_T = 1.5
T = 20

CENTER_DC = (CENTER_T/T)*100

UPDATE_DELAY = 0.100
####

gpio.setmode(gpio.BCM)
gpio.setwarnings(False)
gpio.setup(p_SERVO1, gpio.OUT)
gpio.setup(p_BTN1, gpio.IN, pull_up_down=gpio.PUD_UP)
gpio.setup(p_BTN2, gpio.IN, pull_up_down=gpio.PUD_UP)

p = gpio.PWM(p_SERVO1, 50)
p.start(CENTER_T/T*100)

def DCFromPeriod(period):
	return (period/T)*100

t = 1.5
dc = DCFromPeriod(t)

def printdc():
	print("pw: " + str(t) + "ms\tdc: " + str(dc) + "%")

printdc()
while True:
	if (not gpio.input(17)):
		t=t+0.001
		dc = DCFromPeriod(t)
		printdc()
	
	if (not gpio.input(22)):
		t=t-0.001	
		dc = DCFromPeriod(t)
		printdc()	

	p.ChangeDutyCycle(dc)	
	time.sleep(UPDATE_DELAY)

