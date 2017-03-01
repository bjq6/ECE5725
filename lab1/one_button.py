import RPi.GPIO as gpio

gpio.setmode(gpio.BCM)

gpio.setup(17, gpio.IN, pull_up_down=gpio.PUD_UP)

prev_state = 1;

while(True):
	i = gpio.input(17)
	if (i == 0 and prev_state == 1):
		print("ya pushed 17")
	prev_state = i


