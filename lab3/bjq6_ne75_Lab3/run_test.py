# ----------------
# run_test.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Mar 13, 2017
# ----------------


import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
import os
import time
import numpy
import sys
from threading import Thread

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
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)

s0 = gpio.PWM(p_SERVO0, 50)
s1 = gpio.PWM(p_SERVO1, 50)

SERVO_LIST = [s0, s1]
SERVO_CENTERS = [1.503, 1.5]

s0.start(SERVO_CENTERS[0]/T*100)
s1.start(SERVO_CENTERS[1]/T*100)

start_time = time.time()

s0_state = 0;
s1_state = 0;

s_state_list = [0, 1, -1]

HISTORY_L = []
HISTORY_R = []	

def DCFromPeriod(period):
	return (period/T)*100

def setServoSpeed(s_num, s_dir):
	global SERVO_LIST
	global start_time

	if (s_num == 1):
		s_dir = s_dir * -1

	SERVO_LIST[s_num].ChangeDutyCycle(DCFromPeriod(SERVO_CENTERS[s_num]+s_dir*MAX_DT))
	
	if (s_num == 0):
		#do left servo stuff
		HISTORY_L.append((s_state_list.index(s_dir), time.time()-start_time))
		if (len(HISTORY_L) > 3):
			del HISTORY_L[0]
	if (s_num == 1):
		#do right servo stuff
		HISTORY_R.append((s_state_list.index(s_dir), time.time()-start_time))
		if (len(HISTORY_R) > 3):
			del HISTORY_R[0]

def pauseServos():
	setServoSpeed(0, 0)
	setServoSpeed(1, 0)

os.putenv('SDL_VIDEODRIVER', 'fbcon') # Display on piTFT
os.putenv('SDL_FBDEV', '/dev/fb1')
os.putenv('SDL_MOUSEDRV', 'TSLIB') # Track mouse clicks on piTFT
os.putenv('SDL_MOUSEDEV', '/dev/input/touchscreen')

pygame.init()
pygame.mouse.set_visible(False)

size = width, height = 320, 240
black = 0, 0, 0
red = (255, 0, 0)
green = (0, 255, 0)

screen = pygame.display.set_mode(size)
myfont = pygame.font.SysFont("monospace", 15)
	
class Button: 
	def __init__(self, pos, text, color):
		self.pos = pos
		self.text = text
		self.label = myfont.render(self.text, 1, color)
		self.r = pygame.Rect(pos, self.label.get_size())

	def collide(self,click):
		return self.r.collidepoint(click)

	def draw(self):
		screen.blit(self.label, self.pos)
		

mainMenu = [	Button((240,200),"QUIT", (255, 255, 255)),
		Button((145,110),"STOP", black),
		Button((135,110), "RESUME", black),
		Button((20, 200), "START", (255, 255, 255))]

def drawMenu(m, stop):	
	m[0].draw()
	if (stop):
		pygame.draw.circle(screen, green, (160, 120), 50)
		m[2].draw()
	else:
		pygame.draw.circle(screen, red, (160, 120), 50)
		m[1].draw()

	m[3].draw()

histLabels = [	myfont.render("Stop", 1, (255,255,255)),
		myfont.render("CCW", 1, (255,255,255)),
		myfont.render("CW", 1, (255,255,255))]

def drawHistory():
	global screen
	start_pos_l = 100
	start_pos_r = 100

	screen.blit(myfont.render("LEFT",1,(255,255,255)),(20,75))
	screen.blit(myfont.render("RIGHT",1,(255,255,255)),(240,75))

	for l in HISTORY_L:
		screen.blit(histLabels[l[0]],(10,start_pos_l))
		screen.blit(myfont.render(str(int(l[1])),1,(255,255,255)),(50,start_pos_l))
		start_pos_l = start_pos_l + 20
	for r in HISTORY_R:
		screen.blit(histLabels[r[0]],(260,start_pos_r))
		screen.blit(myfont.render(str(int(r[1])),1,(255,255,255)),(220,start_pos_r))
		start_pos_r = start_pos_r + 20

estop = False
servos_on = True
test_thread_started = False

def test_routine():
	global test_therad_started;
	test_thread_started = True;
	#forward 1 foot
	setServoSpeed(0, 1)
	setServoSpeed(1, 1)
	time.sleep(1)
	
	# stop	
	pauseServos()
	time.sleep(1)
	
	# back 1 foot
	setServoSpeed(0, -1)
	setServoSpeed(1, -1)
	time.sleep(1)
	
	#stop
	pauseServos()
	time.sleep(1)
	
	#pivot left
	setServoSpeed(0, -1)
	setServoSpeed(1, 1)
	time.sleep(1)
	
	#stop	
	pauseServos()
	time.sleep(1)
		
	#pivot right
	setServoSpeed(0, 1)
	setServoSpeed(1, -1)
	time.sleep(1)

	#stop
	pauseServos()

	test_thread_started = False;

t = Thread(target=test_routine)

while (gpio.input(27)): 

	screen.fill(black)	
	for e in pygame.event.get():
		if (e.type == pygame.MOUSEBUTTONUP):
			pos = pygame.mouse.get_pos()
			if (mainMenu[0].collide(pos)):
				print("Quitting...")
				pygame.quit() 
				sys.exit(1)

			if (mainMenu[1].collide(pos)):
				estop = not estop	
			
			if (mainMenu[3].collide(pos)):
				if (not t.is_alive()):
					t = Thread(target=test_routine)
					t.start()
			
	if (estop):
		servos_on = False
		s0.stop()
		s1.stop()
	else:	
		if (not servos_on):
			s0 = gpio.PWM(p_SERVO0, 50)
			s1 = gpio.PWM(p_SERVO1, 50)
			s0.start(SERVO_CENTERS[0]/T*100)
			s1.start(SERVO_CENTERS[1]/T*100)
			servos_on = True

	drawMenu(mainMenu, estop)
	drawHistory()

	pygame.display.flip()
	time.sleep(0.1)
	
