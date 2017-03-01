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

	
class Button: 
	def __init__(self, pos, text):
		self.pos = pos
		self.text = text
		self.label = myfont.render(self.text, 1, (255,255,0))

	def collide(self,click):
		
		return click[0]>self.pos[0] and click[0]<self.pos[0]+self.label.get_size()[0] and click[1]>self.pos[1] and click[1]<self.pos[1]+self.label.get_size()[1]

	def draw(self):
		screen.blit(self.label, self.pos)

mainMenu = [	Button((260,200),"Quit"),
		Button((60,200),"Start")]

playMenu = [	Button((240,200), "Back"),
		Button((40,200), "Pause"),
		Button((120, 200), "Fast"),
		Button((180, 200), "Slow")]

def drawMenu(m):	
	for b in m:
		b.draw()

class Ball:
        def __init__(self,x_speed,y_speed,image,initpos):
                self.speed = [x_speed,y_speed]
                self.ballimage = pygame.image.load(image)
                self.ballrect = self.ballimage.get_rect()
                self.ballrect = self.ballrect.move(initpos)

ballList = [    Ball(3,5,"magic_ball_small.png",[0,0]),
                Ball(-5,-3,"magic_ball_small.png",[50,50])]

def collision(b1, b2):
        return numpy.linalg.norm(numpy.array(b1.ballrect.center)-numpy.array(b2.ballrect.center))<=b1.ballrect.width

def update_location(ballList):

        for ball in ballList:
                if (not pause):
			ball.ballrect = ball.ballrect.move(ball.speed)
                if (ball.ballrect.left < 0 and ball.speed[0] < 0) or (ball.ballrect.right > width and ball.speed[0] > 0):
                        ball.speed[0] = -ball.speed[0]
                if (ball.ballrect.top < 0 and ball.speed[1] < 0) or (ball.ballrect.bottom > height-40 and ball.speed[1] > 0):
                        ball.speed[1] = -ball.speed[1]

                screen.blit(ball.ballimage, ball.ballrect) # Combine Ball surface with workspace surface


        if (collision(ballList[0],ballList[1])):
                b1v = ballList[0].speed
                ballList[0].speed=ballList[1].speed
                ballList[1].speed=b1v

start = False
pause = False
speed = 0.01
while (gpio.input(27)): 

	screen.fill(black)	
	for e in pygame.event.get():
		if (e.type == pygame.MOUSEBUTTONUP):

			pos = pygame.mouse.get_pos()
			print("Hit at " + str(pos))
			if (start):
				if (playMenu[0].collide(pos)):
					start = False
				if (playMenu[1].collide(pos)):
					pause = not pause
				if (playMenu[2].collide(pos)):
					speed = speed/1.5
					if (speed < 0.003):
						speed = 0.003
				if (playMenu[3].collide(pos)):
					speed = speed*1.5
			else:
				if (mainMenu[0].collide(pos)):
					print("Quitting...")
					pygame.quit() 
					sys.exit(1)

				if (mainMenu[1].collide(pos)):
					start = True

	if (start):
		drawMenu(playMenu)
		update_location(ballList)
	else:
		drawMenu(mainMenu)

	pygame.display.flip()
	time.sleep(speed)
	
