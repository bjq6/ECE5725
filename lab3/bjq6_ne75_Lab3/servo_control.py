# ----------------
# servo_control.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Mar 6, 2017
# ----------------

import RPi.GPIO as gpio
import time

## constants ##
p_SERVO1 = 6

CENTER_T = 1.5
T = 20

CENTER_DC = (CENTER_T/T)*100

SPEED_DELAY = 3
####

gpio.setmode(gpio.BCM)
gpio.setwarnings(False)
gpio.setup(p_SERVO1, gpio.OUT)

p = gpio.PWM(p_SERVO1, 50)
p.start(CENTER_T/T*100)

def DCFromPeriod(period):
	return (period/T)*100

t = 1.5
dc = DCFromPeriod(t)
print("speed: 0.0%\tfrq: 50Hz\tpw: " + str(t) + "ms\tdc: " + str(dc) + "%")
time.sleep(SPEED_DELAY)

while (t < 1.7):
	t = t + 0.02
	dc = DCFromPeriod(t)
	print("speed: " + str((dc-CENTER_DC)*100) + "%\tfrq: 50Hz\tpw: " + str(t) + "ms\tdc: " + str(dc) + "%")
	p.ChangeDutyCycle(dc)
	time.sleep(SPEED_DELAY)

while (t > 1.3):
	t = t - 0.02
	dc = DCFromPeriod(t)
	print("speed: " + str((dc-CENTER_DC)*100) + "%\tfrq: 50Hz\tpw: " + str(t) + "ms\tdc: " + str(dc) + "%")
	p.ChangeDutyCycle(dc)
	time.sleep(SPEED_DELAY)

t = CENTER_T
dc = DCFromPeriod(t)
print("speed: " + str((dc-CENTER_DC)*100) + "%\tfrq: 50Hz\tpw: " + str(t) + "ms\tdc: " + str(dc) + "%")
p.ChangeDutyCycle(dc)
time.sleep(SPEED_DELAY)
