# ----------------
# blink.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Mar 6, 2017
# ----------------


import RPi.GPIO as gpio
import time

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)
gpio.setup(13, gpio.OUT)
gpio.setwarnings(False)

p = gpio.PWM(13, 2)
p.start(50)

while(gpio.input(27)):
	time.sleep(1)	

p.stop()
