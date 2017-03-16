import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
import os
import time
import numpy
import sys

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)

#os.putenv('SDL_VIDEODRIVER', 'fbcon') # Display on piTFT
#os.putenv('SDL_FBDEV', '/dev/fb1')
#os.putenv('SDL_MOUSEDRV', 'TSLIB') # Track mouse clicks on piTFT
#os.putenv('SDL_MOUSEDEV', '/dev/input/touchscreen')

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
			pygame.quit()
			sys.exit()



