# ----------------
# six_buttons.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Feb 20, 2017
# ----------------

import RPi.GPIO as gpio
import sys

BTN_LIST = [17, 22, 23, 27, 19, 26]

gpio.setmode(gpio.BCM)

gpio.setup(BTN_LIST, gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state_list = [1, 1, 1, 1, 1, 1]

def read_btns():
	return [gpio.input(x) for x in BTN_LIST]

while(True):
	state_list = read_btns();
	if (0 in state_list):	
		if (prev_state_list[state_list.index(0)] == 1):
			pressed = BTN_LIST[state_list.index(0)]
			print("ya pressed " + str(pressed))
			if (pressed == BTN_LIST[3]):
				sys.exit(1)
			
	prev_state_list = state_list	
