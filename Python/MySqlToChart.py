from PyQt5 import QtCore, QtGui, QtWidgets, uic
from PyQt5.QtWidgets import *
from PyQt5.QtCore import *
from PyQt5.QtGui import *
from pyqtgraph import PlotWidget

import main_ui
import connect_dialog_ui
import pyqtgraph as pg

import time
import sys
import MySQLdb



# Read SQL for update data every 1 seconds
class UpdateThread(QThread):

    log = pyqtSignal(object)
    chart = pyqtSignal(object)

    def __init__(self, run_flag, ip, ssid, password, database):
        super(UpdateThread, self).__init__()
        self.run_flag = run_flag
        self.ip = ip
        self.ssid = ssid
        self.password = password
        self.database = database

    def run(self):
        while self.run_flag:
            global cursor
            dbConn = MySQLdb.connect(self.ip, self.ssid, self.password, self.database)
            cursor = dbConn.cursor()
            cursor.execute("SELECT * FROM test")     
            result = cursor.fetchall()
            self.chart.emit(result)

            time.sleep(1)

    def stop(self):
        self.run_flag = False
        cursor.close
        self.wait()

class Main(QMainWindow, main_ui.Ui_MainWindow):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.log.append("Please connect to your SQL server.")
        self.log.ensureCursorVisible()
        self.clear_button.clicked.connect(self.clear_button_click)
        self.connect_button.clicked.connect(self.connect_button_click)
        self.stop_button.clicked.connect(self.stop_button_click)

        # Initial pyqtGraph Chart
        pg.setConfigOptions(antialias=True)
        win = pg.GraphicsLayoutWidget()
        self.graph_layout.addWidget(win)

        self.p1 = win.addPlot()
        self.p2 = win.addPlot()

        self.p1.setMenuEnabled(False)
        self.p1.setMouseEnabled(True, False)
        self.p1.setRange(yRange = (0, 10), xRange = (0, 4), disableAutoRange=True)
        self.p1.getViewBox().setLimits(xMin=-1, xMax=10)  
        self.p1.setLabel('left', text='Level', color='#ffffff')
        self.p1.setLabel('bottom', text='Time', color='#ffffff')

        self.p1.showGrid(x=False, y=True)  
        self.p1.setLogMode(x=False, y=False)
        self.p1.addLegend() 

        self.p2.setMenuEnabled(False)
        self.p2.setMouseEnabled(True, False)
        self.p2.setRange(yRange = (0, 5), xRange = (0, 4), disableAutoRange=True)
        self.p2.getViewBox().setLimits(xMin=-1, xMax=10)  
        self.p2.setLabel('left', text='Value', color='#ffffff')
        self.p2.setLabel('bottom', text='Time', color='#ffffff')

        self.p2.showGrid(x=False, y=True)  
        self.p2.setLogMode(x=False, y=False)
        self.p2.addLegend() 

        self.o_len = 0 

        # Initial other
        self.connect_button.setEnabled(True)
        self.stop_button.setEnabled(False)

    def clear_button_click(self):
        self.log.clear()

    def connect_button_click(self):
        dialog = Dialog()
        dialog.show()
        dialog.exec_()
        
        if dialog.flag:
            self.log.append(dialog.log)
            try:
                dbConn = MySQLdb.connect(dialog.ip, dialog.ssid, dialog.password, dialog.db_name)
                cursor = dbConn.cursor()
                cursor.execute("SELECT * FROM test")        # check the table

                self.log.append("Connected")
                self.connect_button.setEnabled(False)
                self.stop_button.setEnabled(True)

                # start thread
                run_flag = True
                self.thread = UpdateThread(run_flag, dialog.ip, dialog.ssid, dialog.password, dialog.db_name)
                self.thread.log.connect(self.update_log_from_thread)
                self.thread.chart.connect(self.update_chart_from_thread)
                self.thread.start()

            except MySQLdb._exceptions.OperationalError as error:
                self.log.append(str(error))
                self.connect_button.setEnabled(True)
                self.stop_button.setEnabled(False)

    def stop_button_click(self):
        self.connect_button.setEnabled(True)
        self.stop_button.setEnabled(False)
        # restore chart
        ticks = [list(zip(range(0, 11), ['0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '10']))]
        self.p1.clear()
        self.p2.clear()
        self.p1.getViewBox().setLimits(xMin=-1, xMax=10)  
        self.p2.getViewBox().setLimits(xMin=-1, xMax=10)  
        self.p1.setRange(yRange = (0, 10), xRange = (0, 4), disableAutoRange=True)
        self.p2.setRange(yRange = (0, 5), xRange = (0, 4), disableAutoRange=True)
        self.p1.getAxis('bottom').setTicks(ticks)
        self.p2.getAxis('bottom').setTicks(ticks)

        self.o_len = 0

        # stop thread
        self.thread.stop()
        self.log.append("Disconnected")

    def update_log_from_thread(self, log):
        self.log.append(log)

    def update_chart_from_thread(self, data):
        id_arr = []
        wind_arr = []
        illuminance_arr = []
        vrms_arr = []
        irms_arr = []
        watt_arr = []
        timestamp_arr = []

        self.p1.clear()
        self.p2.clear()
        self.p1.getViewBox().setLimits(xMin=-1, xMax=len(data)+5)  
        self.p2.getViewBox().setLimits(xMin=-1, xMax=len(data)+5)  

        if self.refresh_checkbox.isChecked() and self.o_len != len(data):
            self.o_len = len(data)
            self.p1.setRange(yRange = (0, 10), xRange = (len(data)-3, len(data)+1))
            self.p2.setRange(yRange = (0, 5), xRange = (len(data)-3, len(data)+1))
        else:
            self.o_len = 0  

        for id, wind, illuminance, vrms, irms, watt, timestamp in data:
            id_arr.append(id)
            wind_arr.append(wind)
            illuminance_arr.append(illuminance)
            vrms_arr.append(vrms)
            irms_arr.append(irms)
            watt_arr.append(watt)
            t = str(timestamp).split(' ')
            timestamp_arr.append(t[0]+'\n'+t[1])
            ticks = [list(zip(range(1, len(data)+1), timestamp_arr))]

        self.p1.plot(id_arr, wind_arr, pen='r', name='Wind speed', symbol='o', symbolSize=5)
        self.p1.plot(id_arr, illuminance_arr, pen='g', name='Illuminance', symbol='o', symbolSize=5)
        self.p2.plot(id_arr, vrms_arr, pen='r', name='Vo[V]', symbol='o', symbolSize=5)
        self.p2.plot(id_arr, irms_arr, pen='g', name='Io[A]', symbol='o', symbolSize=5)
        self.p2.plot(id_arr, watt_arr, pen='b', name='Watt[W]', symbol='o', symbolSize=5)
        self.p1.getAxis('bottom').setTicks(ticks)
        self.p2.getAxis('bottom').setTicks(ticks)
        

class Dialog(QDialog, connect_dialog_ui.Ui_Dialog):
    def __init__(self):
        super().__init__()
        self.setupUi(self)
        self.setWindowFlags(QtCore.Qt.WindowStaysOnTopHint)       # The dialog window stay on top
        self.setWindowModality(QtCore.Qt.ApplicationModal)
        self.flag = False
        self.buttonBox.accepted.connect(self.accepted_click)
        
    def accepted_click(self):
        self.flag = True
        self.ip = self.ip_lineEdit.text()
        self.ssid = self.ssid_lineEdit.text()
        self.password = self.pass_lineEdit.text()
        self.db_name = self.db_lineEdit.text()
        self.log="Connecting..."


if __name__ == '__main__':
    app = QtWidgets.QApplication(sys.argv)
    window = Main()
    window.show()
    sys.exit(app.exec_())