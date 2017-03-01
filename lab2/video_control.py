import RPi.GPIO as gpio
import sys
import os

fifo = "/home/pi/lab1/video_fifo"
f = open(fifo, "w", 1)
f.flush()

CMD_LIST = ["pause\n", "seek 10, 0\n", "seek -10 0\n", "quit\n"]
BTN_LIST = [17, 22, 23, 27]

gpio.setmode(gpio.BCM)

gpio.setup(BTN_LIST, gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state_list = [1, 1, 1, 1]

def read_btns():
	return [gpio.input(x) for x in BTN_LIST]

while(True):
	state_list = read_btns();
	if (0 in state_list):	
		if (prev_state_list[state_list.index(0)] == 1):
			pressed = BTN_LIST[state_list.index(0)]
			f.write(CMD_LIST[BTN_LIST.index(pressed)])

			if (pressed == BTN_LIST[3]):
				sys.exit(1)
			
	prev_state_list = state_list	
