import wx 
import MP_GUI
import threading
import Queue
import time
import os
import serial.tools.list_ports
import datetime
from MP_handler import MP_handler
from MP_plot import *
from MP_frames import *


class MainGUI(MP_GUI.frm_MPMain): 

    __mp_handler = MP_handler()
    __mp_att_roll_plot = None
    __mp_att_pitch_plot = None
    
    __mp_frame_queue = Queue.Queue(0)
    __mp_console_message_queue = Queue.Queue(0)
    __mp_is_console_auto_scroll = True
    __mp_is_save_log = False
    __mp_log_filename = ''
    __mp_log_file_ptr = None
    __mp_log_wbytes = 0
    
    __mp_rcin_ch0 = int(0)
    __mp_rcin_ch1 = int(0)
    __mp_rcin_ch2 = int(0)
    __mp_rcin_ch3 = int(0)
    __mp_rcin_ch4 = int(0)

    def __init__(self, parent): 
        MP_GUI.frm_MPMain.__init__(self, parent)  

        # Get com port list 
        ports = []
        for n, (port, desc, hwid) in enumerate(sorted(serial.tools.list_ports.comports()), 1):
            ports.append(port)
        
        self.combo_SerialPort.SetItems(ports)
        self.combo_SerialPort.SetSelection(0) 
        
        # Initial baud rate options
        baud = ['600', '1200', '2400', '4800', '9600', '14400', '19200', '28800',       \
                '38400', '56000', '57600', '115200', '128000', '256000']
        self.combo_BaudRate.SetItems(baud)
        self.combo_BaudRate.SetStringSelection('57600')
        
        self.text_ConsoleSavePath.AppendText(os.getcwd())
        self.text_ConsoleLines.AppendText('0')
        self.text_LogBytes.AppendText('0')
        
        self.__mp_att_roll_plot = MP_Plot(self.panel_RollAnglePlot, 'Roll angle')
        self.__mp_att_pitch_plot = MP_Plot(self.panel_PitchAnglePlot, 'Pitch angle')

        self.__mp_accel_sensor_x = MP_Plot(self.panel_AccelSensorXPlot, 'Accel Sensor X', True, 'b')
        self.__mp_accel_sensor_y = MP_Plot(self.panel_AccelSensorYPlot, 'Accel Sensor Y', True, 'b')
        self.__mp_accel_sensor_z = MP_Plot(self.panel_AccelSensorZPlot, 'Accel Sensor Z', True, 'b')
        self.__mp_gyro_sensor_x = MP_Plot(self.panel_GyroSensorXPlot, 'Gyro Sensor X', True, 'b')
        self.__mp_gyro_sensor_y = MP_Plot(self.panel_GyroSensorYPlot, 'Gyro Sensor Y', True, 'b')
        self.__mp_gyro_sensor_z = MP_Plot(self.panel_GyroSensorZPlot, 'Gyro Sensor Z', True, 'b')
        
        self.__mp_accel_vector_x = MP_Plot(self.panel_AccelVctrXPlot, 'Accel Vector X', True)
        self.__mp_accel_vector_y = MP_Plot(self.panel_AccelVctrYPlot, 'Accel Vector Y', True)
        self.__mp_accel_vector_z = MP_Plot(self.panel_AccelVctrZPlot, 'Accel Vector Z', True)
        self.__mp_gyro_vector_x = MP_Plot(self.panel_GyroVctrXPlot, 'Gyro Vector X', True)
        self.__mp_gyro_vector_y = MP_Plot(self.panel_GyroVctrYPlot, 'Gyro Vector Y', True)
        self.__mp_gyro_vector_z = MP_Plot(self.panel_GyroVctrZPlot, 'Gyro Vector Z', True)

        self.__mp_handler.set_rx_frame_queue(self.__mp_frame_queue)
        self.__thread_start()
        
    def __frame_displayer(self):

        while(True):
            item_cnt = self.__mp_frame_queue.qsize()
            cli_message = ''
            
            while(item_cnt):
                rx_data = self.__mp_frame_queue.get(True, None)
                
                self.__update_input_stick_slider(rx_data["data"])
                
                self.__frame_plotter(rx_data["data"])

                attrs = vars(rx_data["data"])
                del attrs["s_f"]
                del attrs["len"]
                del attrs["CRC16"]
                
                cli_message += rx_data["rx_time"] + ": "
                cli_message += ', '.join("%s: %s" % item for item in attrs.items())
                cli_message += '\n'

                item_cnt -= 1

            # Pass MP frame data to MP console
            if(cli_message != ''):
                self.__mp_console_message_queue.put(cli_message, True, None)
                
                if(self.__mp_log_file_ptr != None):
                    self.__mp_log_file_ptr.write(cli_message)
                    self.__mp_log_wbytes += len(cli_message)           
                    self.text_LogBytes.Clear()
                    self.text_LogBytes.AppendText(str(self.__mp_log_wbytes))
                
            time.sleep(10.0 / 1000.0)
            
    def __update_input_stick_slider(self, rx_frame):
        if(rx_frame != None):
            if(rx_frame.cmd == MP_RC_IN_ID):
                self.__mp_rcin_ch0 = int(rx_frame.RCIN_0) / 2
                self.__mp_rcin_ch1 = int(rx_frame.RCIN_1) / 2
                self.__mp_rcin_ch2 = int(rx_frame.RCIN_2) / 2
                self.__mp_rcin_ch3 = int(rx_frame.RCIN_3) / 2
                self.__mp_rcin_ch4 = int(rx_frame.RCIN_4) / 2
        
    def __frame_plotter(self, rx_frame):
        if(rx_frame != None):
            if(rx_frame.cmd == MP_AHRS_FULL_ID):
                if(self.__mp_att_roll_plot != None):
                    self.__mp_att_roll_plot.add_data(float(rx_frame.ned_roll))
                    
                if(self.__mp_att_pitch_plot != None):
                    self.__mp_att_pitch_plot.add_data(float(rx_frame.ned_pitch))
                    
            if(rx_frame.cmd == MP_AHRS_ACCEL_ID or rx_frame.cmd == MP_AHRS_FULL_ID):        
                if(self.__mp_accel_sensor_x != None):
                    self.__mp_accel_sensor_x.add_data(int(rx_frame.a_snr_x))
                if(self.__mp_accel_sensor_y != None):
                    self.__mp_accel_sensor_y.add_data(int(rx_frame.a_snr_y))
                if(self.__mp_accel_sensor_z != None):
                    self.__mp_accel_sensor_z.add_data(int(rx_frame.a_snr_z))
                if(self.__mp_accel_vector_x != None):
                    self.__mp_accel_vector_x.add_data(int(rx_frame.a_vctr_x))
                if(self.__mp_accel_vector_y != None):
                    self.__mp_accel_vector_y.add_data(int(rx_frame.a_vctr_y))
                if(self.__mp_accel_vector_z != None):
                    self.__mp_accel_vector_z.add_data(int(rx_frame.a_vctr_z))
                    
            if(rx_frame.cmd == MP_AHRS_FULL_ID):        
                if(self.__mp_gyro_sensor_x != None):
                    self.__mp_gyro_sensor_x.add_data(int(rx_frame.g_snr_x))
                if(self.__mp_gyro_sensor_y != None):
                    self.__mp_gyro_sensor_y.add_data(int(rx_frame.g_snr_y))
                if(self.__mp_gyro_sensor_z != None):
                    self.__mp_gyro_sensor_z.add_data(int(rx_frame.g_snr_z))
                if(self.__mp_gyro_vector_x != None):
                    self.__mp_gyro_vector_x.add_data(int(rx_frame.level_vctr_x))
                if(self.__mp_gyro_vector_y != None):
                    self.__mp_gyro_vector_y.add_data(int(rx_frame.level_vctr_y))
                if(self.__mp_gyro_vector_z != None):
                    self.__mp_gyro_vector_z.add_data(int(rx_frame.level_vctr_z))
        
    def OnRollMaxYCheckBox( self, event ):
        if(self.chkbox_RollMaxYChk.IsChecked() == True):
            self.text_RollMaxYVal.Enable(True)
            self.OnRollMaxYValTextEnter(event)
        else:
            self.text_RollMaxYVal.Enable(False)
            self.__mp_att_roll_plot.set_y_range(180)
        
    def OnPitchMaxYCheckBox( self, event ):
        if(self.chkbox_PitchMaxYChk.IsChecked() == True):
            self.text_PitchMaxYVal.Enable(True)
            self.OnPitchMaxYValTextEnter(event)
        else:
            self.text_PitchMaxYVal.Enable(False)
            self.__mp_att_pitch_plot.set_y_range(180)
        
    def OnRollMaxYValTextEnter( self, event ):
        if(self.__mp_att_roll_plot.set_y_range(self.text_RollMaxYVal.GetValue()) == False):
            self.text_RollMaxYVal.SetValue(str(self.__mp_att_roll_plot.get_y_range()))

    def OnPitchMaxYValTextEnter( self, event ):
        if(self.__mp_att_pitch_plot.set_y_range(self.text_PitchMaxYVal.GetValue()) == False):
            self.text_PitchMaxYVal.SetValue(str(self.__mp_att_pitch_plot.get_y_range()))

    def OnAccelXSnrYCheckBox( self, event ):
        if(self.chkbox_AccelXSnrYChk.IsChecked() == True):
            self.text_AccelXSnrMaxYVal.Enable(True)
            self.OnAccelXMaxYValTextEnter(event)
            self.__mp_accel_sensor_x.set_auto_relim(False)
        else:
            self.text_AccelXSnrMaxYVal.Enable(False)
            self.__mp_accel_sensor_x.set_auto_relim(True)

    def OnAccelYSnrYCheckBox( self, event ):
        if(self.chkbox_AccelYSnrYChk.IsChecked() == True):
            self.text_AccelYSnrMaxYVal.Enable(True)
            self.OnAccelYMaxYValTextEnter(event)
            self.__mp_accel_sensor_y.set_auto_relim(False)
        else:
            self.text_AccelYSnrMaxYVal.Enable(False)
            self.__mp_accel_sensor_y.set_auto_relim(True)

    def OnAccelZSnrYCheckBox( self, event ):
        if(self.chkbox_AccelZSnrYChk.IsChecked() == True):
            self.text_AccelZSnrMaxYVal.Enable(True)
            self.OnAccelZMaxYValTextEnter(event)
            self.__mp_accel_sensor_z.set_auto_relim(False)
        else:
            self.text_AccelZSnrMaxYVal.Enable(False)
            self.__mp_accel_sensor_z.set_auto_relim(True)
            
    def OnGyroXSnrYCheckBox( self, event ):
        if(self.chkbox_GyroXSnrYChk.IsChecked() == True):
            self.text_GyroXSnrMaxYVal.Enable(True)
            self.OnGyroXMaxYValTextEnter(event)
            self.__mp_gyro_sensor_x.set_auto_relim(False)
        else:
            self.text_GyroXSnrMaxYVal.Enable(False)
            self.__mp_gyro_sensor_x.set_auto_relim(True)

    def OnGyroYSnrYCheckBox( self, event ):
        if(self.chkbox_GyroYSnrYChk.IsChecked() == True):
            self.text_GyroYSnrMaxYVal.Enable(True)
            self.OnGyroYMaxYValTextEnter(event)
            self.__mp_gyro_sensor_y.set_auto_relim(False)
        else:
            self.text_GyroYSnrMaxYVal.Enable(False)
            self.__mp_gyro_sensor_y.set_auto_relim(True)

    def OnGyroZSnrYCheckBox( self, event ):
        if(self.chkbox_GyroZSnrYChk.IsChecked() == True):
            self.text_GyroZSnrMaxYVal.Enable(True)
            self.OnGyroZMaxYValTextEnter(event)
            self.__mp_gyro_sensor_z.set_auto_relim(False)
        else:
            self.text_GyroZSnrMaxYVal.Enable(False)
            self.__mp_gyro_sensor_z.set_auto_relim(True)    

    def OnAccelXMaxYValTextEnter( self, event ):
        if(self.__mp_accel_sensor_x.set_y_range(self.text_AccelXSnrMaxYVal.GetValue()) == False):
            self.text_AccelXSnrMaxYVal.SetValue(str(self.__mp_accel_sensor_x.get_y_range()))
	
    def OnAccelYMaxYValTextEnter( self, event ):
        if(self.__mp_accel_sensor_y.set_y_range(self.text_AccelYSnrMaxYVal.GetValue()) == False):
            self.text_AccelYSnrMaxYVal.SetValue(str(self.__mp_accel_sensor_y.get_y_range()))

    def OnAccelZMaxYValTextEnter( self, event ):
        if(self.__mp_accel_sensor_z.set_y_range(self.text_AccelZSnrMaxYVal.GetValue()) == False):
            self.text_AccelZSnrMaxYVal.SetValue(str(self.__mp_accel_sensor_z.get_y_range()))

    def OnGyroXMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_sensor_x.set_y_range(self.text_GyroXSnrMaxYVal.GetValue()) == False):
            self.text_GyroXSnrMaxYVal.SetValue(str(self.__mp_gyro_sensor_x.get_y_range()))

    def OnGyroYMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_sensor_y.set_y_range(self.text_GyroYSnrMaxYVal.GetValue()) == False):
            self.text_GyroYSnrMaxYVal.SetValue(str(self.__mp_gyro_sensor_y.get_y_range()))
	
    def OnGyroZMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_sensor_z.set_y_range(self.text_GyroZSnrMaxYVal.GetValue()) == False):
            self.text_GyroZSnrMaxYVal.SetValue(str(self.__mp_gyro_sensor_z.get_y_range()))  
            
    def OnAccelXVctrYCheckBox( self, event ):
        if(self.chkbox_AccelXVctrYChk.IsChecked() == True):
            self.text_AccelXVctrMaxYVal.Enable(True)
            self.OnAccelXVctrMaxYValTextEnter(event)
            self.__mp_accel_vector_x.set_auto_relim(False)
        else:
            self.text_AccelXVctrMaxYVal.Enable(False)
            self.__mp_accel_vector_x.set_auto_relim(True)

    def OnAccelYVctrYCheckBox( self, event ):
        if(self.chkbox_AccelYVctrYChk.IsChecked() == True):
            self.text_AccelYVctrMaxYVal.Enable(True)
            self.OnAccelYVctrMaxYValTextEnter(event)
            self.__mp_accel_vector_y.set_auto_relim(False)
        else:
            self.text_AccelYVctrMaxYVal.Enable(False)
            self.__mp_accel_vector_y.set_auto_relim(True)

    def OnAccelZVctrYCheckBox( self, event ):
        if(self.chkbox_AccelZVctrYChk.IsChecked() == True):
            self.text_AccelZVctrMaxYVal.Enable(True)
            self.OnAccelZVctrMaxYValTextEnter(event)
            self.__mp_accel_vector_z.set_auto_relim(False)
        else:
            self.text_AccelZVctrMaxYVal.Enable(False)
            self.__mp_accel_vector_z.set_auto_relim(True)
            
    def OnGyroXVctrYCheckBox( self, event ):
        if(self.chkbox_GyroXVctrYChk.IsChecked() == True):
            self.text_GyroXVctrMaxYVal.Enable(True)
            self.OnGyroXVctrMaxYValTextEnter(event)
            self.__mp_gyro_vector_x.set_auto_relim(False)
        else:
            self.text_GyroXVctrMaxYVal.Enable(False)
            self.__mp_gyro_vector_x.set_auto_relim(True)

    def OnGyroYVctrYCheckBox( self, event ):
        if(self.chkbox_GyroYVctrYChk.IsChecked() == True):
            self.text_GyroYVctrMaxYVal.Enable(True)
            self.OnGyroYVctrMaxYValTextEnter(event)
            self.__mp_gyro_vector_y.set_auto_relim(False)
        else:
            self.text_GyroYVctrMaxYVal.Enable(False)
            self.__mp_gyro_vector_y.set_auto_relim(True)

    def OnGyroZVctrYCheckBox( self, event ):
        if(self.chkbox_GyroZVctrYChk.IsChecked() == True):
            self.text_GyroZVctrMaxYVal.Enable(True)
            self.OnGyroZVctrMaxYValTextEnter(event)
            self.__mp_gyro_vector_z.set_auto_relim(False)
        else:
            self.text_GyroZVctrMaxYVal.Enable(False)
            self.__mp_gyro_vector_z.set_auto_relim(True)          
        
    def OnAccelXVctrMaxYValTextEnter( self, event ):
        if(self.__mp_accel_vector_x.set_y_range(self.text_AccelXVctrMaxYVal.GetValue()) == False):
            self.text_AccelXVctrMaxYVal.SetValue(str(self.__mp_accel_vector_x.get_y_range()))
	
    def OnAccelYVctrMaxYValTextEnter( self, event ):
        if(self.__mp_accel_vector_y.set_y_range(self.text_AccelYVctrMaxYVal.GetValue()) == False):
            self.text_AccelYVctrMaxYVal.SetValue(str(self.__mp_accel_vector_y.get_y_range()))

        print self.text_AccelYVctrMaxYVal.GetValue()
            
    def OnAccelZVctrMaxYValTextEnter( self, event ):
        if(self.__mp_accel_vector_z.set_y_range(self.text_AccelZVctrMaxYVal.GetValue()) == False):
            self.text_AccelZVctrMaxYVal.SetValue(str(self.__mp_accel_vector_z.get_y_range()))

    def OnGyroXVctrMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_vector_x.set_y_range(self.text_GyroXVctrMaxYVal.GetValue()) == False):
            self.text_GyroXVctrMaxYVal.SetValue(str(self.__mp_gyro_vector_x.get_y_range()))

    def OnGyroYVctrMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_vector_y.set_y_range(self.text_GyroYVctrMaxYVal.GetValue()) == False):
            self.text_GyroYVctrMaxYVal.SetValue(str(self.__mp_gyro_vector_y.get_y_range()))
	
    def OnGyroZVctrMaxYValTextEnter( self, event ):
        if(self.__mp_gyro_vector_z.set_y_range(self.text_GyroZVctrMaxYVal.GetValue()) == False):
            self.text_GyroZVctrMaxYVal.SetValue(str(self.__mp_gyro_vector_z.get_y_range()))  
        
    def OnAttPlotPauseButtonClick( self, event ):
        if(self.btn_AttPlotPause.GetLabel() == 'Pause'):
            self.__mp_att_roll_plot.set_pause(True)
            self.__mp_att_pitch_plot.set_pause(True)


            self.btn_AttPlotPause.SetForegroundColour(wx.RED)
            self.btn_AttPlotPause.SetLabel('Resume')
            
        else:
            self.__mp_att_roll_plot.set_pause(False)
            self.__mp_att_pitch_plot.set_pause(False)
            
            self.btn_AttPlotPause.SetForegroundColour(wx.BLACK)
            self.btn_AttPlotPause.SetLabel('Pause')
        
    def OnAttPlotResetButtonClick( self, event ):
        self.__mp_att_roll_plot.reset()
        self.__mp_att_pitch_plot.reset()
        
    def OnAttPlotSavePictureButtonClick( self, event ):
    
        filelist = []
        filedate = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filepath = os.getcwd() + '\\'
        
        filename = filedate + '_att_roll.png'
        filelist.append(filename)
        self.__mp_att_roll_plot.save_plot(filepath + filename)
        
        filename = filedate + '_att_pitch.png'
        filelist.append(filename)
        self.__mp_att_pitch_plot.save_plot(filepath + filename)
    
        self.text_AttPlotSaveFileList.SetValue(', '.join(filelist))

    def OnSnrPlotPauseButtonClick( self, event ):
        if(self.btn_SnrPlotPause.GetLabel() == 'Pause'):
            self.__mp_accel_sensor_x.set_pause(True)
            self.__mp_accel_sensor_y.set_pause(True)
            self.__mp_accel_sensor_z.set_pause(True)
            self.__mp_gyro_sensor_x.set_pause(True)
            self.__mp_gyro_sensor_y.set_pause(True)
            self.__mp_gyro_sensor_z.set_pause(True)
            
            self.btn_SnrPlotPause.SetForegroundColour(wx.RED)
            self.btn_SnrPlotPause.SetLabel('Resume')
            
        else:
            self.__mp_accel_sensor_x.set_pause(False)
            self.__mp_accel_sensor_y.set_pause(False)
            self.__mp_accel_sensor_z.set_pause(False)
            self.__mp_gyro_sensor_x.set_pause(False)
            self.__mp_gyro_sensor_y.set_pause(False)
            self.__mp_gyro_sensor_z.set_pause(False)
            
            self.btn_SnrPlotPause.SetForegroundColour(wx.BLACK)
            self.btn_SnrPlotPause.SetLabel('Pause')
        
    def OnSnrPlotResetButtonClick( self, event ):
        self.__mp_accel_sensor_x.reset()
        self.__mp_accel_sensor_y.reset()
        self.__mp_accel_sensor_z.reset()
        self.__mp_gyro_sensor_x.reset()
        self.__mp_gyro_sensor_y.reset()
        self.__mp_gyro_sensor_z.reset()
        
    def OnSnrPlotSavePictureButtonClick( self, event ):
    
        filelist = []
        filedate = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filepath = os.getcwd() + '\\'
        
        filename = filedate + '_snr_accel_x.png'
        filelist.append(filename)
        self.__mp_accel_sensor_x.save_plot(filepath + filename)
        
        filename = filedate + '_snr_accel_y.png'
        filelist.append(filename)
        self.__mp_accel_sensor_y.save_plot(filepath + filename)
        
        filename = filedate + '_snr_accel_z.png'
        filelist.append(filename)
        self.__mp_accel_sensor_z.save_plot(filepath + filename)

        filename = filedate + '_snr_gyro_x.png'
        filelist.append(filename)
        self.__mp_gyro_sensor_x.save_plot(filepath + filename)
        
        filename = filedate + '_snr_gyro_y.png'
        filelist.append(filename)
        self.__mp_gyro_sensor_y.save_plot(filepath + filename)
        
        filename = filedate + '_snr_gyro_z.png'
        filelist.append(filename)
        self.__mp_gyro_sensor_z.save_plot(filepath + filename)
        
        self.text_SnrPlotSaveFileList.SetValue(', '.join(filelist))


    def OnVctrPlotPauseButtonClick( self, event ):
        if(self.btn_VctrPlotPause.GetLabel() == 'Pause'):
            self.__mp_accel_vector_x.set_pause(True)
            self.__mp_accel_vector_y.set_pause(True)
            self.__mp_accel_vector_z.set_pause(True)
            self.__mp_gyro_vector_x.set_pause(True)
            self.__mp_gyro_vector_y.set_pause(True)
            self.__mp_gyro_vector_z.set_pause(True)
            
            self.btn_VctrPlotPause.SetForegroundColour(wx.RED)
            self.btn_VctrPlotPause.SetLabel('Resume')
            
        else:
            self.__mp_accel_vector_x.set_pause(False)
            self.__mp_accel_vector_y.set_pause(False)
            self.__mp_accel_vector_z.set_pause(False)
            self.__mp_gyro_vector_x.set_pause(False)
            self.__mp_gyro_vector_y.set_pause(False)
            self.__mp_gyro_vector_z.set_pause(False)
            
            self.btn_VctrPlotPause.SetForegroundColour(wx.BLACK)
            self.btn_VctrPlotPause.SetLabel('Pause')
        
    def OnVctrPlotResetButtonClick( self, event ):
        self.__mp_accel_vector_x.reset()
        self.__mp_accel_vector_y.reset()
        self.__mp_accel_vector_z.reset()
        self.__mp_gyro_vector_x.reset()
        self.__mp_gyro_vector_y.reset()
        self.__mp_gyro_vector_z.reset()
        
    def OnVctrPlotSavePictureButtonClick( self, event ):
    
        filelist = []
        filedate = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
        filepath = os.getcwd() + '\\'
        
        filename = filedate + '_vctr_accel_x.png'
        filelist.append(filename)
        self.__mp_accel_vector_x.save_plot(filepath + filename)
        
        filename = filedate + '_vctr_accel_y.png'
        filelist.append(filename)
        self.__mp_accel_vector_y.save_plot(filepath + filename)
        
        filename = filedate + '_vctr_accel_z.png'
        filelist.append(filename)
        self.__mp_accel_vector_z.save_plot(filepath + filename)

        filename = filedate + '_vctr_gyro_x.png'
        filelist.append(filename)
        self.__mp_gyro_vector_x.save_plot(filepath + filename)
        
        filename = filedate + '_vctr_gyro_y.png'
        filelist.append(filename)
        self.__mp_gyro_vector_y.save_plot(filepath + filename)
        
        filename = filedate + '_vctr_gyro_z.png'
        filelist.append(filename)
        self.__mp_gyro_vector_z.save_plot(filepath + filename)
        
        self.text_VctrPlotSaveFileList.SetValue(', '.join(filelist))
        
    def OnCleanScreenClick( self, event ):
        self.text_Console.Clear()
        self.text_ConsoleLines.Clear()
        self.text_ConsoleLines.AppendText('0')        

    def OnSaveLogClick( self, event ):
    
        if(self.btn_SaveLog.GetLabel() == 'Save LOG'):
            self.__mp_is_save_log = True
            filename = datetime.datetime.now().strftime("%Y%m%d_%H%M%S") + '.log'
            self.__mp_log_filename = os.getcwd() + '\\' + filename           
            self.__mp_log_file_ptr = open(self.__mp_log_filename, 'w')

            self.btn_SaveLog.SetForegroundColour(wx.RED)
            self.btn_SaveLog.SetLabel('Stop LOG')
            
        else:
            self.__mp_is_save_log = False
            self.__mp_log_wbytes = 0
            
            if(self.__mp_log_file_ptr != None):
                self.__mp_log_file_ptr.close()
                self.__mp_log_file_ptr = None
            
            self.btn_SaveLog.SetForegroundColour(wx.BLACK)
            self.btn_SaveLog.SetLabel('Save LOG')

    def OnSerialOpenClick( self, event ):

        if(self.btn_SerialOpen.GetLabel() == 'Open'):
        
            port = self.combo_SerialPort.GetStringSelection()
            baud = self.combo_BaudRate.GetStringSelection()

            self.__mp_handler.open_serial(port, int(baud))
            self.__mp_handler.thread_start()
            
            self.combo_SerialPort.Enable(False)
            self.combo_BaudRate.Enable(False)

            self.btn_SerialOpen.SetLabel('Close')
        else:
        
            self.__mp_handler.thread_stop()
            self.__mp_handler.close_serial()
            
            self.combo_SerialPort.Enable(True)
            self.combo_BaudRate.Enable(True)
        
            self.btn_SerialOpen.SetLabel('Open')
    
        pass
        
    def OnConsoleTimer(self, event):
    
        item_cnt = self.__mp_console_message_queue.qsize()
        
        while(item_cnt):
            try:
                CLI_message = self.__mp_console_message_queue.get(False, None)
                
                if(CLI_message != None):
                    #self.text_Console.Freeze();
                    #self.text_Console.AppendText(CLI_message)
                    
                    self.text_Console.SetInsertionPointEnd()
                    self.text_Console.WriteText(CLI_message)
                    self.text_Console.SetInsertionPointEnd()
                    
                    if(self.__mp_is_console_auto_scroll):
                        #self.text_Console.ScrollLines(self.text_Console.GetNumberOfLines())
                        
                        self.text_Console.ShowPosition(self.text_Console.GetLastPosition())
                        self.text_Console.ScrollLines(-1)
                        
                    #self.text_Console.Thaw()
                    
                    self.text_ConsoleLines.Clear()
                    self.text_ConsoleLines.AppendText(str(self.text_Console.GetNumberOfLines()))
                    
            except:
                # Pretend nothing happened.
                pass
                
            item_cnt -= 1

        pass
      
    def OnAttPlotTimer( self, event ): 

        self.slide_InputCh1.SetValue(self.__mp_rcin_ch0)
        self.text_InputCh1.SetLabel(str(self.__mp_rcin_ch0))
        self.slide_InputCh2.SetValue(self.__mp_rcin_ch1)
        self.text_InputCh2.SetLabel(str(self.__mp_rcin_ch1))
        self.slide_InputCh3.SetValue(self.__mp_rcin_ch2)
        self.text_InputCh3.SetLabel(str(self.__mp_rcin_ch2))
        self.slide_InputCh4.SetValue(self.__mp_rcin_ch3)
        self.text_InputCh4.SetLabel(str(self.__mp_rcin_ch3))
        self.slide_InputCh5.SetValue(self.__mp_rcin_ch4)
        self.text_InputCh5.SetLabel(str(self.__mp_rcin_ch4))
    
        if(self.__mp_att_roll_plot != None):
            self.__mp_att_roll_plot.draw_plot()
        if(self.__mp_att_pitch_plot != None):
            self.__mp_att_pitch_plot.draw_plot()
            
        if(self.__mp_accel_sensor_x != None):
            self.__mp_accel_sensor_x.draw_plot()
        if(self.__mp_accel_sensor_y != None):
            self.__mp_accel_sensor_y.draw_plot()
        if(self.__mp_accel_sensor_z != None):
            self.__mp_accel_sensor_z.draw_plot()                        
        if(self.__mp_gyro_sensor_x != None):
            self.__mp_gyro_sensor_x.draw_plot()
        if(self.__mp_gyro_sensor_y != None):
            self.__mp_gyro_sensor_y.draw_plot()
        if(self.__mp_gyro_sensor_z != None):
            self.__mp_gyro_sensor_z.draw_plot()  
    
        if(self.__mp_accel_vector_x != None):
            self.__mp_accel_vector_x.draw_plot()
        if(self.__mp_accel_vector_y != None):
            self.__mp_accel_vector_y.draw_plot()
        if(self.__mp_accel_vector_z != None):
            self.__mp_accel_vector_z.draw_plot()                        
        if(self.__mp_gyro_vector_x != None):
            self.__mp_gyro_vector_x.draw_plot()
        if(self.__mp_gyro_vector_y != None):
            self.__mp_gyro_vector_y.draw_plot()
        if(self.__mp_gyro_vector_z != None):
            self.__mp_gyro_vector_z.draw_plot()             

    def OnAutoScrollCheckBox(self, event):
        self.__mp_is_console_auto_scroll = self.chkbox_AutoScroll.GetValue()
        
    def __thread_start(self):
        t1 = threading.Thread(target = self.__frame_displayer)
        t1.start()
      
app = wx.App(False) 
frame = MainGUI(None) 
frame.Show(True) 

#start the applications 
app.MainLoop() 