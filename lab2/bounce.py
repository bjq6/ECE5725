import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
from threading import Thread

pygame.init()

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)


size = width, height = 320, 240
speed = [2,2]
black = 0, 0, 0

screen = pygame.display.set_mode(size)
ball = pygame.image.load("magic_ball.png")
ballrect = ball.get_rect()

def update():
	global ballrect
	ballrect = ballrect.move(speed)
	if ballrect.left < 0 or ballrect.right > width:
		speed[0] = -speed[0]
	if ballrect.top < 0 or ballrect.bottom > height:
		speed[1] = -speed[1]
		
	screen.fill(black) # Erase the Work space
	screen.blit(ball, ballrect) # Combine Ball surface with workspace surface
	pygame.display.flip() # display workspace on screen 

while (gpio.input(27)): 
	for e in pygame.event.get():
		if (e.type == QUIT):
			pygame.quit()
			sys.exit()
	
	update()


