# ----------------
# more_video_control_cb.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Feb 20, 2017
# ----------------

import RPi.GPIO as gpio
import sys
import os
import time

fifo = "/home/pi/lab2/video_fifo"
f = open(fifo, 'w', 1)
f.flush()
f.write("\n")

CMD_LIST = ["pause\n", "seek 10, 0\n", "seek -10 0\n", "seek 30, 0\n", "seek -30, 0\n", "quit\n"]
BTN_LIST = [17, 22, 23, 19, 26, 27]

gpio.setmode(gpio.BCM)

gpio.setup(BTN_LIST, gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state_list = [1, 1, 1, 1, 1, 1]

def btn_pause(p):
	f.write(CMD_LIST[0])

def btn_s10f(p):
	f.write(CMD_LIST[1])

def btn_s10b(p):
	f.write(CMD_LIST[2])

def btn_s30f(p):
	f.write(CMD_LIST[3])

def btn_s30b(p):
	f.write(CMD_LIST[4])

def btn_quit(p):
	f.write(CMD_LIST[5])
	f.close()
	gpio.cleanup()
	sys.exit(1)

CB_LIST = [btn_pause, btn_s10f, btn_s10b, btn_s30f, btn_s30b, btn_quit]

[gpio.add_event_detect(BTN_LIST[i], gpio.FALLING, callback=CB_LIST[i], bouncetime=30) for i in range(len(CB_LIST))]

time.sleep(10)
sys.exit(1)	
