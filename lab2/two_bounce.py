import pygame # Import Library and initialize pygame
import RPi.GPIO as gpio
import os

pygame.init()

gpio.setmode(gpio.BCM)
gpio.setup(27, gpio.IN, pull_up_down=gpio.PUD_UP)


size = width, height = 320, 240
black = 0, 0, 0

screen = pygame.display.set_mode(size)

class Ball: 
	def __init__(self,x_speed,y_speed,image):
		self.speed = [x_speed,y_speed]			
		self.ballimage = pygame.image.load(image)
		self.ballrect = self.ballimage.get_rect()

ballList = [	Ball(2,2,"magic_ball.png"),
		Ball(1,1,"magic_ball.png")]

def update_location(ballList):

	screen.fill(black) # Erase the Work space

	for ball in ballList: 
		ball.ballrect = ball.ballrect.move(ball.speed)
		if ball.ballrect.left < 0 or ball.ballrect.right > width:
			ball.speed[0] = -ball.speed[0]
		if ball.ballrect.top < 0 or ball.ballrect.bottom > height:
			ball.speed[1] = -ball.speed[1]
		
		screen.blit(ball.ballimage, ball.ballrect) # Combine Ball surface with workspace surface
	
	pygame.display.flip() # display workspace on screen 

while (gpio.input(27)): 
	for e in pygame.event.get():
		if (e.type == pygame.QUIT):
			pygame.quit()
			sys.exit()	

	update_location(ballList)


