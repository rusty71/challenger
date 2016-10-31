import threading
import serial
import struct
import random
import time


class LLCom(threading.Thread):
	ESC = 0x7c
	HDR = 0x7d
	FOOT = 0x7e

	WAIT_FOR_HEADER = 1
	IN_MSG = 2
	IN_ESC = 3

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
		packed = []
		packed.append(chr(self.HDR))
		for c in msg:
			if ord(c) in [self.ESC,self.HDR,self.FOOT]:
				packed.append(chr(self.ESC))
			packed.append(c)
		packed.append(chr(self.FOOT))
		self.comport.write(packed)	#show off

	def run(self):
		print("starting thread")
		state = self.WAIT_FOR_HEADER
		msg = bytearray()
		while True:
			self.chunk = bytes(self.comport.read(20))
			#print("received", [ord(x) for x in self.chunk])
			if self.chunk:
				for c in [ord(x) for x in self.chunk]:
					if state == self.WAIT_FOR_HEADER:
						if c == self.HDR:
							state = self.IN_MSG
							idx = 0
					elif state == self.IN_MSG:
						if c == self.ESC:
							state = self.IN_ESC
						elif c == self.FOOT:
							state=self.WAIT_FOR_HEADER
							self.callback(msg)
							msg = bytearray()
						else:
							msg.append(c)
					elif state == self.IN_ESC:
						state = self.IN_MSG
						msg.append(c)

SEQ_FMT = "BHhhhhII"

lastid = 0
def handle_msg(msg):
	global lastid
	if msg[0] == 0:		#SEQ update message
		#~ try:
		(msg_type, segment, blade, x, y, z, ticks, millis) = struct.unpack(SEQ_FMT, msg)
		print(msg_type, segment, blade, x, y, z, ticks, millis)
		if (segment-1) != lastid:
			print("##############################################################################",lastid)
		lastid = segment
		#~ except:
			#~ pass
	elif msg[0] == 1:	#debug string
		print("debug : "),
		print([x for x in msg[1:]])		

comm = LLCom("/dev/ttyS1", handle_msg)
comm.start()

i = 0
while True:
	#~ time.sleep(0.1)
	#~ print("sending..")
	sequence = struct.pack(SEQ_FMT, i, 1,2,3,4,5,6,7)
	comm.send(sequence)
	i = (i + 1)&0xff

comm.join()
