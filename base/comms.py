import PyCmdMessenger
import pyqtgraph as pg
import numpy as np
#~ pg.setConfigOption('background', 'w')
#~ pg.setConfigOption('foreground', 'k')
plt = pg.plot()
bufferSize = 100
blade_data = np.zeros(bufferSize)
x_data = np.zeros(bufferSize)
y_data = np.zeros(bufferSize)

blade_curve = plt.plot(pen=pg.hsvColor(1.0, sat=1.0, val=1.0, alpha=1.0))
x_curve = plt.plot(pen=pg.hsvColor(0.8, sat=1.0, val=0.5, alpha=1.0))
y_curve = plt.plot(pen=pg.hsvColor(0.6, sat=1.0, val=1.0, alpha=1.0))

line = plt.addLine(x=0)
plt.setRange(xRange=[0, bufferSize], yRange=[-50, 50])
i = 0

arduino = PyCmdMessenger.ArduinoBoard("/dev/ttyUSB1",baud_rate=115200)

commands = [["seq_cmd","IiiiiLL"]]

# Initialize the messenger
c = PyCmdMessenger.CmdMessenger(arduino,commands)

# called on QTimer
def update():
	msg = c.receive()
	if msg:
		print(msg[1][0])
		blade_data[msg[1][0]] = msg[1][1]*2
		blade_curve.setData(blade_data)
		x_data[msg[1][0]] = msg[1][2]
		x_curve.setData(x_data)
		y_data[msg[1][0]] = msg[1][3]
		y_curve.setData(y_data)
		line.setValue(msg[1][0])

timer = pg.QtCore.QTimer()
timer.timeout.connect(update)
#fast loop
timer.start(0)

if __name__ == '__main__':
	import sys
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		pg.QtGui.QApplication.instance().exec_()
