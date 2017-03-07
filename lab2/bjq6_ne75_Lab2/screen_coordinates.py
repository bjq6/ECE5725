# ----------------
# screen_coordinates.py
# Nikita Ermoshkin (ne75) and Brendan Quinn (bjq6)
# Feb 27, 2017
# ----------------

import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
import os
import time
import numpy
import sys

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)

os.putenv('SDL_VIDEODRIVER', 'fbcon') # Display on piTFT
os.putenv('SDL_FBDEV', '/dev/fb1')
os.putenv('SDL_MOUSEDRV', 'TSLIB') # Track mouse clicks on piTFT
os.putenv('SDL_MOUSEDEV', '/dev/input/touchscreen')

pygame.init()
pygame.mouse.set_visible(False)


size = width, height = 320, 240
black = 0, 0, 0

screen = pygame.display.set_mode(size)

myfont = pygame.font.SysFont("monospace", 15)

# render text

label = myfont.render("Quit", 1, (255,255,0))
screen.blit(label, (260, 220))


pygame.display.flip()

while (gpio.input(27)): 
	for e in pygame.event.get():
		if (e.type == pygame.MOUSEBUTTONUP):


			screen.fill(black)	
			pos = pygame.mouse.get_pos()	

			pos_text = "Hit at " + str(pos)
			
			sys.stdout.write(pos_text+"\n")
			sys.stdout.flush()

			print pos_text

			pos_label = myfont.render(pos_text,1,(255,255,0))
			screen.blit(pos_label, (100,135))

			label = myfont.render("Quit", 1, (255,255,0))
			screen.blit(label, (260, 220))

			if (pos[0] > 260 and pos[1] > 220):
				pygame.quit() 
				sys.exit(1)

	pygame.display.flip()
