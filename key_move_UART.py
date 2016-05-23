import RPi.GPIO as GPIO
import time
import sys
import math
import tty, termios
import serial
import struct

#UART serial comm. to Arduino
ser = serial.Serial(port='/dev/ttyAMA0',
               baudrate = 9600,
               parity=serial.PARITY_NONE,
               stopbits=serial.STOPBITS_ONE,
               bytesize=serial.EIGHTBITS,
               timeout=1)
ser.close()
ser.open()


#sends reset signal for the state machine in arduino
def ard_reset():
	d = 0.001
	
	ser.write(struct.pack('>B', 0x01))
	time.sleep(d)
	ser.write(struct.pack('>B', 0x08))


#sends duty cycle, direction for the three wheels
def send_to_ard(dir1, dcyc1, dir2, dcyc2, dir3, dcyc3):
	d = 0.01		#delay between writes
	print dcyc1
	print dcyc2
	print dcyc3
	
	#UART Comm.
	ser.write(struct.pack('>B', 0x01))			#wheel 1 info: Direction, Duty Cycle
	time.sleep(d)
	ser.write(struct.pack('>B', dir1))
	time.sleep(d)
	ser.write(struct.pack('>B', dcyc1))
	time.sleep(d)
	
	ser.write(struct.pack('>B', 0x02))			#wheel 2 info: Direction, Duty Cycle
	time.sleep(d)
	ser.write(struct.pack('>B', dir2))
	time.sleep(d)
	ser.write(struct.pack('>B', dcyc2))
	time.sleep(d)
	
	ser.write(struct.pack('>B', 0x03))			#wheel 3 info: Direction, Duty Cycle
	time.sleep(d)
	ser.write(struct.pack('>B', dir3))
	time.sleep(d)
	ser.write(struct.pack('>B', dcyc3))
	time.sleep(d)
	

def robo_move(x, y, w):
	
	#matrix equation to calc. forces for each of the motors of the holonomic robot
	f1 = (0.58*x) - (0.33*y) + (0.33*w)
	f2 = (-0.58*x) - (0.33*y) + (0.33*w)
	f3 = (0*x) + (0.67*y) + (0.33*w)
	
	
	f1t = abs(f1)
	f2t = abs(f2)
	f3t = abs(f3)
	f_max = f1t if ((f1t>f2t) & (f1t>f3t)) else f2t if (f2t>f3t) else f3t		#finds absolute of max. value for normalization
	
	#normalize to 255 (max. duty cycle value, preserving relative relationship between forces)
	if f_max != 0:				#if max force is non-zero
		f_max_ard = 255/f_max
	else:						#if max force is 0 (i.e STOP)
		f_max_ard = 0
	dc1_ard = int(f1t*f_max_ard)	#normalized duty-cycle1
	dc2_ard = int(f2t*f_max_ard)	#normalized duty-cycle2
	dc3_ard = int(f3t*f_max_ard)	#normalized duty-cycle3
	
	#Send information to arduino. For CW send '5', for CCW send '6'.
	send_to_ard(6 if (f1<0) else 5, dc1_ard, 6 if (f2<0) else 5, dc2_ard, 6 if (f3<0) else 5, dc3_ard)

	

	
print "********************Keyboard Mover for Holonomic Robot********************"
print "Press any of the following keys to control the robot:\n"
print "W - Forward, S - Backward, A - Left, D - Right"
print "N - Rotate CCW, M - Rotate CW"
print "J - Circle CCW, L - Circle CW"
print "K - Stop Moving"
print "Q - Quit Program"


	
ard_reset();		#reset/init. Arduino state
while True:
	print "\nPress a key....\n"
	fd = sys.stdin.fileno()
	old_settings = termios.tcgetattr(fd)
	tty.setraw(sys.stdin.fileno())

	ch = sys.stdin.read(1)		#read character from keyboard
	termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
	print ch + '\n'
	if ch=='q':				#quit program
		ser.close()
		sys.exit(0)
	elif ch=='k':					
		robo_move(0,0,0)			#Stop moving
	elif ch=='w':					
		robo_move(1,0,0)			#move forward
	elif ch=='s':					
		robo_move(-1,0,0)			#move backward
	elif ch=='d':					
		robo_move(0,1,0)			#move left
	elif ch=='a':
		robo_move(0,-1,0)			#move right
	elif ch=='n':
		robo_move(0,0,1)			#rotate counter-clockwise
	elif ch=='m':
		robo_move(0,0,-1)			#rotate clockwise
	elif ch=='j':
		robo_move(1,-1,0)			#circle counter-clockwise
	elif ch=='l':
		robo_move(1,1,0)			#circle clockwise
	
	
	
	
	
	
	
	