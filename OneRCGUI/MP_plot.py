#!/usr/bin/python
# -*- coding: UTF-8 -*-

import wx

# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#
import matplotlib
matplotlib.use('WXAgg')
import matplotlib.pyplot as plt
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
    
import numpy as np
import pylab

class MP_Plot():

    __plot_title = ''
    __plot_total = int(0)
    __plot_Y_range = int(180)
    __plot_auto_relim = False
    __canvas_dpi = 800
    __plot_data = None
    __data_limit_size = 1000
    __color = 'r'

    def __init__(self, panel, title = 'New plot', auto_relim = False, color = 'r'):

        self.public_data = []
        self.paused = False
        self.__plot_title = title
        self.__plot_auto_relim = auto_relim
        self.__color = color

        self.create_main_panel(panel)

    def create_main_panel(self, panel):
        self.panel = panel

        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)

        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.canvas, 1, flag=wx.LEFT | wx.TOP | wx.GROW)        
        
        self.panel.SetSizer(self.vbox)
        self.panel.Layout()
        self.canvas.draw()
        
    def init_plot(self):

        self.fig = plt.figure(figsize=plt.figaspect(0.5))

        self.axes = self.fig.add_subplot(111)
        self.axes.set_axis_bgcolor('white')
        self.axes.set_title(self.__plot_title, size=8)
        self.axes.grid(True, color='gray')

        self.axes.set_ybound(-self.__plot_Y_range, self.__plot_Y_range)
        
        pylab.setp(self.axes.get_xticklabels(), fontsize=8)
        pylab.setp(self.axes.get_yticklabels(), fontsize=8)
        
        self.__plot_data, = self.axes.plot([], [], color = self.__color, linewidth = 1)
        
    def draw_plot(self):
        """ Redraws the plot
        """

        if(self.paused == True):
            return
            
        plot_data = []
        plot_data.extend(self.public_data)

        if(len(plot_data) == 0):
            return

        x_data = np.arange(len(plot_data)) + self.__plot_total
        y_data = np.array(plot_data)
        
        self.__plot_data.set_xdata(x_data)
        self.__plot_data.set_ydata(y_data)
       
        if(self.__plot_auto_relim == True):
            self.axes.set_ybound(round(max(y_data) + 0.5), round(min(y_data) - 0.5))
        else:
            self.axes.set_ybound(-self.__plot_Y_range, self.__plot_Y_range)
        self.axes.set_xbound(round(max(x_data) + 0.5), round(min(x_data) - 0.5))

        self.panel.Layout()
        self.canvas.draw()
    
    def add_data(self, data):
        if(len(self.public_data) >= self.__data_limit_size): 
            self.public_data.pop(0)
            
        self.public_data.append(data)   

        self.__plot_total += 1
        
    def set_y_range(self, y_new):
        try:
            y_tmp = int(y_new)
            self.__plot_Y_range = y_tmp
            return True
        except ValueError:
            return False
        except Exception:
            return False

    def get_y_range(self):
        return self.__plot_Y_range
    
    def set_auto_relim(self, enable = True):
        self.__plot_auto_relim = enable
    
    def set_pause(self, is_set = True):
        self.paused = is_set
        
    def reset(self):
        self.public_data = []
        self.__plot_total = 0
    
    def save_plot(self, path):
        self.canvas.print_figure(path, dpi=self.__canvas_dpi)
        self.panel.Layout()
        self.canvas.draw()
    
    def on_exit(self, event):
        self.Destroy()
