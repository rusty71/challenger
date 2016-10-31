from PyQt4 import QtGui,QtCore, uic
import sys
import numpy as np
import pylab
import time
import pyqtgraph
import threading
import serial
import struct

SEQ_FMT = "BHhhhhII"

UP_FMT = "BHhhhhII"

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
			self.chunk = bytes(self.comport.read(200))
			print("received", [ord(x) for x in self.chunk])
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

class SequenceSlider(QtGui.QSlider):
	sequence = 0

class TelemetryApp(QtGui.QMainWindow):
	@QtCore.pyqtSlot(int)
	def valueChangedSlot(self,value):
		print(self,"QSlider Value Changed!","QSlider Value: "+ value.__str__())
		print(type(self.sender().sequence))
		self.comm.send(struct.pack("BBB", 3, self.sender().sequence, value))

	def __init__(self, parent=None):
		self.comm = LLCom("/dev/ttyUSB1", self.update)
		pyqtgraph.setConfigOption('background', 'w') #before loading widget
		super(TelemetryApp, self).__init__(parent)
		uic.loadUi('./telemetry.ui', self)
		#Add sliders
		self.sliders=[]
		for i in range(40):
			#~ slide = QtGui.QSlider(self.frame)
			slide = SequenceSlider(self.frame)
			slide.sequence = i	# for reference
			slide.setOrientation(QtCore.Qt.Vertical)
			slide.setObjectName(("verticalSlider_%d" %(i)))
			self.sliders.append(slide)
		for slider in self.sliders:
			#~ slider.valueChanged.connect(self.sliderchange)
			slider.connect(slider,QtCore.SIGNAL("valueChanged(int)"), self,QtCore.SLOT("valueChangedSlot(int)"))
			slider.setMaximum(255)
			self.SequenceSliders.addWidget(slider)
		#set grid
#		self.SequencePlotter.plotItem.showGrid(True, True, 0.7)
		self.bufferSize = 50
		#data store
		self.blade_data = np.zeros(self.bufferSize)
		self.x_data = np.zeros(self.bufferSize)
		self.y_data = np.zeros(self.bufferSize)
		#curves
		self.blade_curve = self.SequencePlotter.plot(pen=pyqtgraph.hsvColor(1.0, sat=1.0, val=1.0, alpha=1.0))
		self.x_curve = self.SequencePlotter.plot(pen=pyqtgraph.hsvColor(0.8, sat=1.0, val=0.5, alpha=1.0))
		self.y_curve = self.SequencePlotter.plot(pen=pyqtgraph.hsvColor(0.6, sat=1.0, val=1.0, alpha=1.0))
		#vertical follow line
		self.line = self.SequencePlotter.addLine(x=0)
		#self.SequencePlotter.setRange(xRange=[0, self.bufferSize], yRange=[-50, 50])
		self.SequencePlotter.setMouseEnabled(x=False, y=False)
		self.SequencePlotter.setRange(rect=None, xRange=[0, 40], yRange=[-50, 50], padding=0.0, update=True, disableAutoRange=True)
		self.comm.start()

	def sliderchange(self, value):
		print("changed", value)
		print self.sender()

	def update(self, msg):
		if msg[0] == 0:		#SEQ update message
			if len(msg) > 0:
				try:
					(msg_type, segment, blade, x, y, z, ticks, millis) = struct.unpack(SEQ_FMT, msg.__str__())
					self.blade_data[segment] = blade
					self.x_data[segment] = x
					self.y_data[segment] = y
					self.blade_curve.setData(self.blade_data)
					self.x_curve.setData(self.x_data)
					self.y_curve.setData(self.y_data)
					self.line.setValue(segment)
					print(msg_type, segment, blade, x, y, z, ticks, millis)
				except:
					print("exception", len(msg), msg)



if __name__=="__main__":
	app = QtGui.QApplication(sys.argv)
	form = TelemetryApp()
	form.show()
	#~ form.update("") #start with something
	app.exec_()

