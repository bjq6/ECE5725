import os
import sys

fifo = "/home/pi/lab1/video_fifo"

CMD_DICT = {
	"p" : "pause\n",
	"j" : "seek -10 0\n",
	"k" : "seek 10 0\n",
	"s" : "stop\n",
	"q" : "quit\n" 
}

f = open(fifo, "w", 1)
f.flush()

while(True):
	
	cmd = raw_input("enter command: ");
	print(cmd)
	if (cmd in CMD_DICT):
			
		f.write(CMD_DICT[cmd])
		if (cmd == "q"):
			sys.exit(1)


