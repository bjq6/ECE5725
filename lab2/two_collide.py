import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
import os
import time
import numpy

pygame.init()

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)


size = width, height = 320, 240
black = 0, 0, 0

screen = pygame.display.set_mode(size)

class Ball: 
	def __init__(self,x_speed,y_speed,image,initpos):
		self.speed = [x_speed,y_speed]			
		self.ballimage = pygame.image.load(image)
		self.ballrect = self.ballimage.get_rect()
		self.ballrect = self.ballrect.move(initpos)

ballList = [	Ball(3,5,"magic_ball_small.png",[0,0]),
		Ball(-5,-3,"magic_ball_small.png",[50,50])]

def collision(b1, b2):
	return numpy.linalg.norm(numpy.array(b1.ballrect.center)-numpy.array(b2.ballrect.center))<=b1.ballrect.width	

def update_location(ballList):

	screen.fill(black) # Erase the Work space

	for ball in ballList: 
		ball.ballrect = ball.ballrect.move(ball.speed)
		if ball.ballrect.left < 0 or ball.ballrect.right > width:
			ball.speed[0] = -ball.speed[0]
		if ball.ballrect.top < 0 or ball.ballrect.bottom > height:
			ball.speed[1] = -ball.speed[1]
		
		screen.blit(ball.ballimage, ball.ballrect) # Combine Ball surface with workspace surface

	
	if (collision(ballList[0],ballList[1])):
		b1v = ballList[0].speed
		ballList[0].speed=ballList[1].speed
		ballList[1].speed=b1v
	
	pygame.display.flip() # display workspace on screen 

while (gpio.input(27)): 
	for e in pygame.event.get():
		if (e.type == QUIT):
			pygame.quit()
			sys.exit()

	update_location(ballList)
	time.sleep(.01)


