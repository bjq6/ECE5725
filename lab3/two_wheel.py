import RPi.GPIO as gpio
import time

## constants ##

p_BTN1 = 17
p_BTN2 = 22

p_SERVO0 = 6
p_SERVO1 = 5

T = 20
MAX_DT = 0.2

SPEED_DELAY = 3
####

gpio.setmode(gpio.BCM)
gpio.setwarnings(False)
gpio.setup(p_SERVO0, gpio.OUT)
gpio.setup(p_SERVO1, gpio.OUT) 
gpio.setup(p_BTN1, gpio.IN, pull_up_down=gpio.PUD_UP)
gpio.setup(p_BTN2, gpio.IN, pull_up_down=gpio.PUD_UP)

s0 = gpio.PWM(p_SERVO0, 50)
s1 = gpio.PWM(p_SERVO1, 50)

SERVO_LIST = [s0, s1]
SERVO_CENTERS = [1.503, 1.5]

s0.start(SERVO_CENTERS[0]/T*100)
s1.start(SERVO_CENTERS[1]/T*100)

s0_state = 0;
s1_state = 0;

s_state_list = [0, 1, -1]

def DCFromPeriod(period):
	return (period/T)*100

def setServoSpeed(s_num, s_dir):
	global SERVO_LIST
	SERVO_LIST[s_num].ChangeDutyCycle(DCFromPeriod(SERVO_CENTERS[s_num]+s_dir*MAX_DT))

def btn1_cb(p):
	global s0_state
	setServoSpeed(0, s_state_list[s0_state])
	s0_state = (s0_state + 1) % 3	

def btn2_cb(p):
        global s1_state 
	setServoSpeed(1, s_state_list[s1_state])
	s1_state = (s1_state + 1) % 3

gpio.add_event_detect(p_BTN1, gpio.FALLING, callback=btn1_cb, bouncetime=100)
gpio.add_event_detect(p_BTN2, gpio.FALLING, callback=btn2_cb, bouncetime=100)

while (True):
	time.sleep(1)	
