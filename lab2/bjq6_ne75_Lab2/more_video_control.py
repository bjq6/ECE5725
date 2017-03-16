# ----------------
# more_video_control.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Feb 20, 2017
# ----------------

import RPi.GPIO as gpio
import sys
import os
import time
from threading import Thread

fifo = "/home/pi/lab2/video_fifo"
f = open(fifo, "w", 1)
f.flush()
f.write("\n")

CMD_LIST = ["pause\n", "seek 10, 0\n", "seek -10 0\n", "quit\n", "seek 30, 0\n", "seek -30, 0\n"]
BTN_LIST = [17, 22, 23, 27, 19, 26]

gpio.setmode(gpio.BCM)

gpio.setup(BTN_LIST, gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state_list = [1, 1, 1, 1, 1, 1]

def timer():
	time.sleep(10)

def read_btns():
	return [gpio.input(x) for x in BTN_LIST]

p = Thread(target=timer)
p.start()

while(True):
	#time.sleep(0.00002)
	if (not p.isAlive()):
		sys.exit(1)
	
	state_list = read_btns();
	if (0 in state_list):	
		if (prev_state_list[state_list.index(0)] == 1):
			pressed = BTN_LIST[state_list.index(0)]
			f.write(CMD_LIST[BTN_LIST.index(pressed)])

			if (pressed == BTN_LIST[3]):
				sys.exit(1)
			
	prev_state_list = state_list	
