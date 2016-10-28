from PyQt4 import QtGui,QtCore, uic
import sys
import numpy as np
import pylab
import time
import pyqtgraph

#class ExampleApp(QtGui.QMainWindow, telemetry.Ui_MainWindow):
class ExampleApp(QtGui.QMainWindow):
    def __init__(self, parent=None):
		pyqtgraph.setConfigOption('background', 'w') #before loading widget
		super(ExampleApp, self).__init__(parent)
		uic.loadUi('./telemetry.ui', self)
		#Add sliders
		self.sliders=[]
		for i in range(40):
			slide = QtGui.QSlider(self.frame)
			slide.setOrientation(QtCore.Qt.Vertical)
			slide.setObjectName(("verticalSlider_%d" %(i)))
			self.sliders.append(slide)

		for slider in self.sliders:
			self.SequenceSliders.addWidget(slider)
		self.SequencePlotter.plotItem.showGrid(True, True, 0.7)
		#self.show()

    def update(self):
        t1=time.clock()
        points=100 #number of data points
        X=np.arange(points)
        Y=np.sin(np.arange(points)/points*3*np.pi+time.time())
        C=pyqtgraph.hsvColor(time.time()/5%1,alpha=.5)
        pen=pyqtgraph.mkPen(color=C,width=3,alpha=0.1)
        self.SequencePlotter.plot(X,Y,pen=pen,clear=True)
        print("update took %.02f ms"%((time.clock()-t1)*1000))
        #~ if self.chkMore.isChecked():
            #~ QtCore.QTimer.singleShot(1, self.update) # QUICKLY repeat

if __name__=="__main__":
    app = QtGui.QApplication(sys.argv)
    form = ExampleApp()
    form.show()
    form.update() #start with something
    app.exec_()
    print("DONE")


