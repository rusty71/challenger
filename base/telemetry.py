# -*- coding: utf-8 -*-

# Form implementation generated from reading ui file 'telemetry.ui'
#
# Created by: PyQt4 UI code generator 4.11.4
#
# WARNING! All changes made in this file will be lost!

from PyQt4 import QtCore, QtGui

try:
	_fromUtf8 = QtCore.QString.fromUtf8
except AttributeError:
	def _fromUtf8(s):
		return s

try:
	_encoding = QtGui.QApplication.UnicodeUTF8
	def _translate(context, text, disambig):
		return QtGui.QApplication.translate(context, text, disambig, _encoding)
except AttributeError:
	def _translate(context, text, disambig):
		return QtGui.QApplication.translate(context, text, disambig)

class Ui_MainWindow(object):
	def setupUi(self, MainWindow):
		MainWindow.setObjectName(_fromUtf8("MainWindow"))
		MainWindow.resize(993, 837)
		self.centralwidget = QtGui.QWidget(MainWindow)
		self.centralwidget.setObjectName(_fromUtf8("centralwidget"))
		self.horizontalLayout = QtGui.QHBoxLayout(self.centralwidget)
		self.horizontalLayout.setObjectName(_fromUtf8("horizontalLayout"))
		self.frame = QtGui.QFrame(self.centralwidget)
		self.frame.setFrameShape(QtGui.QFrame.NoFrame)
		self.frame.setFrameShadow(QtGui.QFrame.Plain)
		self.frame.setObjectName(_fromUtf8("frame"))
		self.verticalLayout = QtGui.QVBoxLayout(self.frame)
		self.verticalLayout.setMargin(0)
		self.verticalLayout.setObjectName(_fromUtf8("verticalLayout"))
		self.label = QtGui.QLabel(self.frame)
		self.label.setObjectName(_fromUtf8("label"))
		self.verticalLayout.addWidget(self.label)
		self.verticalLayout_3 = QtGui.QVBoxLayout()
		self.verticalLayout_3.setObjectName(_fromUtf8("verticalLayout_3"))
		self.grFFT = PlotWidget(self.frame)
		self.grFFT.setObjectName(_fromUtf8("grFFT"))
		self.verticalLayout_3.addWidget(self.grFFT)
		self.horizontalLayout_2 = QtGui.QHBoxLayout()
		self.horizontalLayout_2.setObjectName(_fromUtf8("horizontalLayout_2"))

		self.sliders=[]
		for i in range(100):
			slide = QtGui.QSlider(self.frame)
			slide.setOrientation(QtCore.Qt.Vertical)
			slide.setObjectName(_fromUtf8("verticalSlider_%d" %(i)))
			self.sliders.append(slide)
		
		for slider in self.sliders:
			self.horizontalLayout_2.addWidget(slider)

		#~ self.verticalSlider_3 = QtGui.QSlider(self.frame)
		#~ self.verticalSlider_3.setOrientation(QtCore.Qt.Vertical)
		#~ self.verticalSlider_3.setObjectName(_fromUtf8("verticalSlider_3"))
		#~ self.horizontalLayout_2.addWidget(self.verticalSlider_3)

		#~ self.verticalSlider_4 = QtGui.QSlider(self.frame)
		#~ self.verticalSlider_4.setOrientation(QtCore.Qt.Vertical)
		#~ self.verticalSlider_4.setObjectName(_fromUtf8("verticalSlider_4"))
		#~ self.horizontalLayout_2.addWidget(self.verticalSlider_4)

		#~ self.verticalSlider = QtGui.QSlider(self.frame)
		#~ self.verticalSlider.setOrientation(QtCore.Qt.Vertical)
		#~ self.verticalSlider.setObjectName(_fromUtf8("verticalSlider"))
		#~ self.horizontalLayout_2.addWidget(self.verticalSlider)

		self.verticalLayout_3.addLayout(self.horizontalLayout_2)
		self.verticalLayout.addLayout(self.verticalLayout_3)

		self.horizontalLayout.addWidget(self.frame)
		MainWindow.setCentralWidget(self.centralwidget)

		self.retranslateUi(MainWindow)
		QtCore.QMetaObject.connectSlotsByName(MainWindow)

	def retranslateUi(self, MainWindow):
		MainWindow.setWindowTitle(_translate("MainWindow", "MainWindow", None))
		self.label.setText(_translate("MainWindow", "Telemetry", None))

from pyqtgraph import PlotWidget
