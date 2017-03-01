import RPi.GPIO as gpio

gpio.setmode(gpio.BCM)

gpio.setup(26,  gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state = 1;

while(True):
	i = gpio.input(26)
	if (i == 0 and prev_state == 1):
		print("ya pushed it")
	prev_state = i


