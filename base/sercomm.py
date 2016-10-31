import threading
import serial
import struct
import random
import time

ESC = 0x7c
HDR = 0x7d
FOOT = 0x7e

WAIT_FOR_HEADER = 1
IN_MSG = 2
IN_ESC = 3

class LLCom(threading.Thread):
	def __init__(self, comport, callback):
		self.callback = callback
		threading.Thread.__init__(self)
		self.comport = serial.Serial(comport,baudrate=38400, timeout=0.1)  # open serial port
		#reset Arduino
		self.comport.setDTR(False)
		time.sleep(0.1)
		self.comport.flushInput()
		self.comport.setDTR(True)
		time.sleep(1.8)		#might vary

	#send bytearray()
	def send(self,msg):
		escape = lambda x: x in [ESC,HDR,FOOT] and [ESC,x] or  [x]
		self.comport.write([HDR]+sum([escape(x) for x in msg],[])+[FOOT])	#show off
		#~ frame = [[HDR]+sum([escape(x) for x in msg],[])+[FOOT]]
		#~ for b in frame:
			#~ time.sleep(0.01)
			#~ self.comport.write(b)

	def run(self):
		print("starting thread")
		state = WAIT_FOR_HEADER
		msg = bytearray()
		while True:
			self.chunk = bytes(self.comport.read(20))
			#print("received", [ord(x) for x in self.chunk])
			if self.chunk:
				for c in [ord(x) for x in self.chunk]:
					if state == WAIT_FOR_HEADER:
						if c == HDR:
							state = IN_MSG
							idx = 0
					elif state == IN_MSG:
						if c == ESC:
							state = IN_ESC
						elif c == FOOT:
							state=WAIT_FOR_HEADER
							self.callback(msg)
							msg = bytearray()
						else:
							msg.append(c)
					elif state == IN_ESC:
						state = IN_MSG
						msg.append(c)

SEQ_FMT = "BHhhhhII"

lastid = 0
def handle_msg(msg):
	global lastid
	if msg[0] == 0:		#SEQ update message
		try:
			(msg_type, segment, blade, x, y, z, ticks, millis) = struct.unpack(SEQ_FMT, msg)
			print(msg_type, segment, blade, x, y, z, ticks, millis)
			if (segment-1) != lastid:
				print("##############################################################################",lastid)
			lastid = segment
		except:
			pass
	elif msg[0] == 1:	#debug string
		print("debug : "),
		print([x for x in msg[1:]])		

comm = LLCom("/dev/ttyUSB1", handle_msg)
comm.start()
while True:
	time.sleep(0.1)
	print("sending..")
#	comm.send(bytes([1,'T','e','s','t',' ','m','e','s','s','a','g','e']))
#	comm.send(bytes([1,2,3,4,5,6,7,8,9,0,2,3,4,5,6,7,8,9]))
#	comm.send(bytes([1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]))
	comm.send([1,1,2,3,4,5,6,7,8,9,0,1,2,3,4,5,6,7,8,9,0])

comm.join()
