# -*- coding: utf-8 -*- 

###########################################################################
## Python code generated with wxFormBuilder (version Jun 17 2015)
## http://www.wxformbuilder.org/
##
## PLEASE DO "NOT" EDIT THIS FILE!
###########################################################################

import wx
import wx.xrc

tmp_console_id = 1000
tmr_att_plot_id = 1001

###########################################################################
## Class frm_MPMain
###########################################################################

class frm_MPMain ( wx.Frame ):
	
	def __init__( self, parent ):
		wx.Frame.__init__ ( self, parent, id = wx.ID_ANY, title = u"OneRC GUI", pos = wx.DefaultPosition, size = wx.Size( 792,570 ), style = wx.CAPTION|wx.CLOSE_BOX|wx.DEFAULT_FRAME_STYLE|wx.MAXIMIZE_BOX|wx.MINIMIZE_BOX|wx.RESIZE_BORDER|wx.TAB_TRAVERSAL )
		
		self.SetSizeHintsSz( wx.DefaultSize, wx.DefaultSize )
		self.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 91, False, "Tahoma" ) )
		
		self.m_statusBar1 = self.CreateStatusBar( 1, wx.ST_SIZEGRIP, wx.ID_ANY )
		self.m_menubar1 = wx.MenuBar( 0 )
		self.m_menu1 = wx.Menu()
		self.m_menubar1.Append( self.m_menu1, u"MyMenu" ) 
		
		self.SetMenuBar( self.m_menubar1 )
		
		fgSizer1 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1.AddGrowableCol( 0 )
		fgSizer1.AddGrowableRow( 1 )
		fgSizer1.SetFlexibleDirection( wx.BOTH )
		fgSizer1.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_ALL )
		
		fgSizer2 = wx.FlexGridSizer( 0, 1, 0, 0 )
		fgSizer2.AddGrowableCol( 0 )
		fgSizer2.SetFlexibleDirection( wx.BOTH )
		fgSizer2.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_panel9 = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.Size( -1,35 ), wx.TAB_TRAVERSAL )
		self.m_panel9.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_INACTIVECAPTION ) )
		
		fgSizer9 = wx.FlexGridSizer( 0, 7, 0, 0 )
		fgSizer9.SetFlexibleDirection( wx.BOTH )
		fgSizer9.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText7 = wx.StaticText( self.m_panel9, wx.ID_ANY, u"Port : ", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText7.Wrap( -1 )
		self.m_staticText7.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		
		fgSizer9.Add( self.m_staticText7, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.BOTTOM|wx.LEFT|wx.RIGHT|wx.TOP, 5 )
		
		combo_SerialPortChoices = []
		self.combo_SerialPort = wx.ComboBox( self.m_panel9, wx.ID_ANY, u"Combo!", wx.DefaultPosition, wx.Size( 150,-1 ), combo_SerialPortChoices, 0 )
		fgSizer9.Add( self.combo_SerialPort, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.m_panel10 = wx.Panel( self.m_panel9, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_panel10.SetMinSize( wx.Size( 10,-1 ) )
		
		fgSizer9.Add( self.m_panel10, 1, wx.EXPAND |wx.ALL, 5 )
		
		self.m_staticText8 = wx.StaticText( self.m_panel9, wx.ID_ANY, u"Baud rate : ", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText8.Wrap( -1 )
		self.m_staticText8.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 92, False, wx.EmptyString ) )
		
		fgSizer9.Add( self.m_staticText8, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		combo_BaudRateChoices = []
		self.combo_BaudRate = wx.ComboBox( self.m_panel9, wx.ID_ANY, u"Combo!", wx.DefaultPosition, wx.Size( 150,-1 ), combo_BaudRateChoices, 0 )
		fgSizer9.Add( self.combo_BaudRate, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.m_panel11 = wx.Panel( self.m_panel9, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_panel11.SetMinSize( wx.Size( 10,-1 ) )
		
		fgSizer9.Add( self.m_panel11, 1, wx.EXPAND |wx.ALL, 5 )
		
		self.btn_SerialOpen = wx.Button( self.m_panel9, wx.ID_ANY, u"Open", wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer9.Add( self.btn_SerialOpen, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		self.m_panel9.SetSizer( fgSizer9 )
		self.m_panel9.Layout()
		fgSizer2.Add( self.m_panel9, 1, wx.EXPAND|wx.TOP, 5 )
		
		
		fgSizer1.Add( fgSizer2, 1, wx.ALL|wx.EXPAND|wx.LEFT, 5 )
		
		fgSizer4 = wx.FlexGridSizer( 0, 2, 0, 0 )
		fgSizer4.AddGrowableCol( 0 )
		fgSizer4.AddGrowableRow( 0 )
		fgSizer4.SetFlexibleDirection( wx.BOTH )
		fgSizer4.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_notebook2 = wx.Notebook( self, wx.ID_ANY, wx.DefaultPosition, wx.Size( 500,500 ), 0 )
		self.panel_MPConsloe = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		fgSizer6 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer6.AddGrowableRow( 0 )
		fgSizer6.SetFlexibleDirection( wx.BOTH )
		fgSizer6.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.text_Console = wx.TextCtrl( self.panel_MPConsloe, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 10000,-1 ), wx.TE_DONTWRAP|wx.TE_MULTILINE|wx.TE_READONLY|wx.TE_RICH2 )
		fgSizer6.Add( self.text_Console, 0, wx.ALL|wx.EXPAND, 5 )
		
		fgSizer10 = wx.FlexGridSizer( 2, 4, 0, 0 )
		fgSizer10.AddGrowableCol( 1 )
		fgSizer10.SetFlexibleDirection( wx.BOTH )
		fgSizer10.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText10 = wx.StaticText( self.panel_MPConsloe, wx.ID_ANY, u"Path : ", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText10.Wrap( -1 )
		fgSizer10.Add( self.m_staticText10, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.text_ConsoleSavePath = wx.TextCtrl( self.panel_MPConsloe, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 150,-1 ), 0 )
		fgSizer10.Add( self.text_ConsoleSavePath, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND, 5 )
		
		self.m_checkBox2 = wx.CheckBox( self.panel_MPConsloe, wx.ID_ANY, u"AutoName", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_checkBox2.SetValue(True) 
		fgSizer10.Add( self.m_checkBox2, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.btn_SaveLog = wx.Button( self.panel_MPConsloe, wx.ID_ANY, u"Save LOG", wx.DefaultPosition, wx.Size( 90,-1 ), 0 )
		fgSizer10.Add( self.btn_SaveLog, 0, wx.ALL, 5 )
		
		self.m_staticText11 = wx.StaticText( self.panel_MPConsloe, wx.ID_ANY, u"Lines:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText11.Wrap( -1 )
		fgSizer10.Add( self.m_staticText11, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		fgSizer13 = wx.FlexGridSizer( 0, 3, 0, 0 )
		fgSizer13.SetFlexibleDirection( wx.BOTH )
		fgSizer13.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.text_ConsoleLines = wx.TextCtrl( self.panel_MPConsloe, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, wx.TE_READONLY )
		fgSizer13.Add( self.text_ConsoleLines, 0, wx.ALL, 5 )
		
		self.m_staticText12 = wx.StaticText( self.panel_MPConsloe, wx.ID_ANY, u"LogBytes", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText12.Wrap( -1 )
		fgSizer13.Add( self.m_staticText12, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.text_LogBytes = wx.TextCtrl( self.panel_MPConsloe, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, wx.TE_READONLY )
		fgSizer13.Add( self.text_LogBytes, 0, wx.ALL, 5 )
		
		
		fgSizer10.Add( fgSizer13, 1, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.chkbox_AutoScroll = wx.CheckBox( self.panel_MPConsloe, wx.ID_ANY, u"AutoScroll", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.chkbox_AutoScroll.SetValue(True) 
		fgSizer10.Add( self.chkbox_AutoScroll, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.btn_ClearScreen = wx.Button( self.panel_MPConsloe, wx.ID_ANY, u"Clear screen", wx.DefaultPosition, wx.Size( 90,-1 ), 0 )
		fgSizer10.Add( self.btn_ClearScreen, 0, wx.ALL, 5 )
		
		
		fgSizer6.Add( fgSizer10, 1, wx.EXPAND, 5 )
		
		
		self.panel_MPConsloe.SetSizer( fgSizer6 )
		self.panel_MPConsloe.Layout()
		fgSizer6.Fit( self.panel_MPConsloe )
		self.m_notebook2.AddPage( self.panel_MPConsloe, u"Console", True )
		self.m_panel6 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		fgSizer17 = wx.FlexGridSizer( 3, 0, 0, 0 )
		fgSizer17.AddGrowableCol( 0 )
		fgSizer17.AddGrowableRow( 0 )
		fgSizer17.SetFlexibleDirection( wx.BOTH )
		fgSizer17.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer18 = wx.FlexGridSizer( 2, 3, 0, 0 )
		fgSizer18.AddGrowableCol( 0 )
		fgSizer18.AddGrowableCol( 1 )
		fgSizer18.AddGrowableCol( 2 )
		fgSizer18.AddGrowableRow( 0 )
		fgSizer18.AddGrowableRow( 1 )
		fgSizer18.SetFlexibleDirection( wx.BOTH )
		fgSizer18.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer19 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer19.AddGrowableCol( 0 )
		fgSizer19.AddGrowableRow( 0 )
		fgSizer19.SetFlexibleDirection( wx.BOTH )
		fgSizer19.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_RollAnglePlot = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_RollAnglePlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_RollAnglePlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer19.Add( self.panel_RollAnglePlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20.SetFlexibleDirection( wx.BOTH )
		fgSizer20.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13.Wrap( -1 )
		fgSizer20.Add( self.m_staticText13, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_RollMaxYChk = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20.Add( self.chkbox_RollMaxYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_RollMaxYVal = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"1", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_RollMaxYVal.SetMaxLength( 3 ) 
		self.text_RollMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_RollMaxYVal.Enable( False )
		
		fgSizer20.Add( self.text_RollMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer19.Add( fgSizer20, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer19, 1, wx.EXPAND, 5 )
		
		fgSizer181 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer181.AddGrowableCol( 0 )
		fgSizer181.AddGrowableRow( 0 )
		fgSizer181.SetFlexibleDirection( wx.BOTH )
		fgSizer181.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_PitchAnglePlot = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_PitchAnglePlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_PitchAnglePlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer181.Add( self.panel_PitchAnglePlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer201 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer201.SetFlexibleDirection( wx.BOTH )
		fgSizer201.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText131 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText131.Wrap( -1 )
		fgSizer201.Add( self.m_staticText131, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_PitchMaxYChk = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer201.Add( self.chkbox_PitchMaxYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_PitchMaxYVal = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"1", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_PitchMaxYVal.SetMaxLength( 3 ) 
		self.text_PitchMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_PitchMaxYVal.Enable( False )
		
		fgSizer201.Add( self.text_PitchMaxYVal, 0, wx.ALL, 5 )
		
		
		fgSizer181.Add( fgSizer201, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer181, 1, wx.EXPAND, 5 )
		
		fgSizer182 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer182.AddGrowableCol( 0 )
		fgSizer182.AddGrowableRow( 0 )
		fgSizer182.SetFlexibleDirection( wx.BOTH )
		fgSizer182.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_RollAnglePlot2 = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_RollAnglePlot2.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_RollAnglePlot2.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer182.Add( self.panel_RollAnglePlot2, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer202 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer202.SetFlexibleDirection( wx.BOTH )
		fgSizer202.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText132 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText132.Wrap( -1 )
		fgSizer202.Add( self.m_staticText132, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_RollMaxYChk2 = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer202.Add( self.chkbox_RollMaxYChk2, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_RollMaxYVal2 = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"180", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_RollMaxYVal2.SetMaxLength( 3 ) 
		self.text_RollMaxYVal2.Enable( False )
		
		fgSizer202.Add( self.text_RollMaxYVal2, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer182.Add( fgSizer202, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer182, 1, wx.EXPAND, 5 )
		
		fgSizer183 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer183.AddGrowableCol( 0 )
		fgSizer183.AddGrowableRow( 0 )
		fgSizer183.SetFlexibleDirection( wx.BOTH )
		fgSizer183.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_RollAnglePlot3 = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_RollAnglePlot3.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_RollAnglePlot3.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer183.Add( self.panel_RollAnglePlot3, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer203 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer203.SetFlexibleDirection( wx.BOTH )
		fgSizer203.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText133 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText133.Wrap( -1 )
		fgSizer203.Add( self.m_staticText133, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_RollMaxYChk3 = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer203.Add( self.chkbox_RollMaxYChk3, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_RollMaxYVal3 = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"180", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_RollMaxYVal3.SetMaxLength( 3 ) 
		self.text_RollMaxYVal3.Enable( False )
		
		fgSizer203.Add( self.text_RollMaxYVal3, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer183.Add( fgSizer203, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer183, 1, wx.EXPAND, 5 )
		
		fgSizer184 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer184.AddGrowableCol( 0 )
		fgSizer184.AddGrowableRow( 0 )
		fgSizer184.SetFlexibleDirection( wx.BOTH )
		fgSizer184.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_RollAnglePlot4 = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_RollAnglePlot4.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_RollAnglePlot4.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer184.Add( self.panel_RollAnglePlot4, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer204 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer204.SetFlexibleDirection( wx.BOTH )
		fgSizer204.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText134 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText134.Wrap( -1 )
		fgSizer204.Add( self.m_staticText134, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_RollMaxYChk4 = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer204.Add( self.chkbox_RollMaxYChk4, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_RollMaxYVal4 = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"180", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_RollMaxYVal4.SetMaxLength( 3 ) 
		self.text_RollMaxYVal4.Enable( False )
		
		fgSizer204.Add( self.text_RollMaxYVal4, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer184.Add( fgSizer204, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer184, 1, wx.EXPAND, 5 )
		
		fgSizer185 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer185.AddGrowableCol( 0 )
		fgSizer185.AddGrowableRow( 0 )
		fgSizer185.SetFlexibleDirection( wx.BOTH )
		fgSizer185.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_RollAnglePlot5 = wx.Panel( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_RollAnglePlot5.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_RollAnglePlot5.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer185.Add( self.panel_RollAnglePlot5, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer205 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer205.SetFlexibleDirection( wx.BOTH )
		fgSizer205.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText135 = wx.StaticText( self.m_panel6, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText135.Wrap( -1 )
		fgSizer205.Add( self.m_staticText135, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_RollMaxYChk5 = wx.CheckBox( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer205.Add( self.chkbox_RollMaxYChk5, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_RollMaxYVal5 = wx.TextCtrl( self.m_panel6, wx.ID_ANY, u"180", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		self.text_RollMaxYVal5.Enable( False )
		
		fgSizer205.Add( self.text_RollMaxYVal5, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer185.Add( fgSizer205, 1, wx.EXPAND, 5 )
		
		
		fgSizer18.Add( fgSizer185, 1, wx.EXPAND, 5 )
		
		
		fgSizer17.Add( fgSizer18, 1, wx.EXPAND, 5 )
		
		fgSizer54 = wx.FlexGridSizer( 0, 1, 0, 0 )
		fgSizer54.AddGrowableCol( 0 )
		fgSizer54.AddGrowableRow( 0 )
		fgSizer54.SetFlexibleDirection( wx.BOTH )
		fgSizer54.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticline4 = wx.StaticLine( self.m_panel6, wx.ID_ANY, wx.DefaultPosition, wx.Size( 10000,-1 ), wx.LI_HORIZONTAL )
		self.m_staticline4.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.m_staticline4.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		self.m_staticline4.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		
		fgSizer54.Add( self.m_staticline4, 0, wx.EXPAND, 5 )
		
		
		fgSizer17.Add( fgSizer54, 1, wx.EXPAND, 5 )
		
		fgSizer21 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer21.AddGrowableCol( 0 )
		fgSizer21.AddGrowableRow( 0 )
		fgSizer21.SetFlexibleDirection( wx.BOTH )
		fgSizer21.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.text_AttPlotSaveFileList = wx.TextCtrl( self.m_panel6, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 300,-1 ), 0 )
		self.text_AttPlotSaveFileList.SetForegroundColour( wx.Colour( 255, 0, 0 ) )
		
		fgSizer21.Add( self.text_AttPlotSaveFileList, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND, 5 )
		
		self.btn_AttPlotSavePicture = wx.Button( self.m_panel6, wx.ID_ANY, u"Save picture", wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer21.Add( self.btn_AttPlotSavePicture, 0, wx.ALL, 5 )
		
		self.btn_AttPlotPause = wx.Button( self.m_panel6, wx.ID_ANY, u"Pause", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer21.Add( self.btn_AttPlotPause, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )
		
		self.btn_AttPlotReset = wx.Button( self.m_panel6, wx.ID_ANY, u"Reset", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer21.Add( self.btn_AttPlotReset, 0, wx.ALL, 5 )
		
		
		fgSizer17.Add( fgSizer21, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.EXPAND, 5 )
		
		
		self.m_panel6.SetSizer( fgSizer17 )
		self.m_panel6.Layout()
		fgSizer17.Fit( self.m_panel6 )
		self.m_notebook2.AddPage( self.m_panel6, u"ATT plot", False )
		self.m_panel7 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		fgSizer171 = wx.FlexGridSizer( 3, 0, 0, 0 )
		fgSizer171.AddGrowableCol( 0 )
		fgSizer171.AddGrowableRow( 0 )
		fgSizer171.SetFlexibleDirection( wx.BOTH )
		fgSizer171.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer186 = wx.FlexGridSizer( 2, 3, 0, 0 )
		fgSizer186.AddGrowableCol( 0 )
		fgSizer186.AddGrowableCol( 1 )
		fgSizer186.AddGrowableCol( 2 )
		fgSizer186.AddGrowableRow( 0 )
		fgSizer186.AddGrowableRow( 1 )
		fgSizer186.SetFlexibleDirection( wx.BOTH )
		fgSizer186.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer191 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer191.AddGrowableCol( 0 )
		fgSizer191.AddGrowableRow( 0 )
		fgSizer191.SetFlexibleDirection( wx.BOTH )
		fgSizer191.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelSensorXPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelSensorXPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelSensorXPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer191.Add( self.panel_AccelSensorXPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer206 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer206.SetFlexibleDirection( wx.BOTH )
		fgSizer206.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText136 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText136.Wrap( -1 )
		fgSizer206.Add( self.m_staticText136, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelXSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer206.Add( self.chkbox_AccelXSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelXSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelXSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_AccelXSnrMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_AccelXSnrMaxYVal.Enable( False )
		
		fgSizer206.Add( self.text_AccelXSnrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer191.Add( fgSizer206, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer191, 1, wx.EXPAND, 5 )
		
		fgSizer1811 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1811.AddGrowableCol( 0 )
		fgSizer1811.AddGrowableRow( 0 )
		fgSizer1811.SetFlexibleDirection( wx.BOTH )
		fgSizer1811.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelSensorYPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelSensorYPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelSensorYPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1811.Add( self.panel_AccelSensorYPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2011 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2011.SetFlexibleDirection( wx.BOTH )
		fgSizer2011.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1311 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1311.Wrap( -1 )
		fgSizer2011.Add( self.m_staticText1311, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelYSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2011.Add( self.chkbox_AccelYSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelYSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelYSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_AccelYSnrMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_AccelYSnrMaxYVal.Enable( False )
		
		fgSizer2011.Add( self.text_AccelYSnrMaxYVal, 0, wx.ALL, 5 )
		
		
		fgSizer1811.Add( fgSizer2011, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer1811, 1, wx.EXPAND, 5 )
		
		fgSizer1821 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1821.AddGrowableCol( 0 )
		fgSizer1821.AddGrowableRow( 0 )
		fgSizer1821.SetFlexibleDirection( wx.BOTH )
		fgSizer1821.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelSensorZPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelSensorZPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelSensorZPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1821.Add( self.panel_AccelSensorZPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2021 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2021.SetFlexibleDirection( wx.BOTH )
		fgSizer2021.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1321 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1321.Wrap( -1 )
		fgSizer2021.Add( self.m_staticText1321, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelZSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2021.Add( self.chkbox_AccelZSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelZSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelZSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_AccelZSnrMaxYVal.Enable( False )
		
		fgSizer2021.Add( self.text_AccelZSnrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer1821.Add( fgSizer2021, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer1821, 1, wx.EXPAND, 5 )
		
		fgSizer1831 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1831.AddGrowableCol( 0 )
		fgSizer1831.AddGrowableRow( 0 )
		fgSizer1831.SetFlexibleDirection( wx.BOTH )
		fgSizer1831.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroSensorXPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroSensorXPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroSensorXPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1831.Add( self.panel_GyroSensorXPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2031 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2031.SetFlexibleDirection( wx.BOTH )
		fgSizer2031.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1331 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1331.Wrap( -1 )
		fgSizer2031.Add( self.m_staticText1331, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroXSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2031.Add( self.chkbox_GyroXSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroXSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroXSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_GyroXSnrMaxYVal.Enable( False )
		
		fgSizer2031.Add( self.text_GyroXSnrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer1831.Add( fgSizer2031, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer1831, 1, wx.EXPAND, 5 )
		
		fgSizer1841 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1841.AddGrowableCol( 0 )
		fgSizer1841.AddGrowableRow( 0 )
		fgSizer1841.SetFlexibleDirection( wx.BOTH )
		fgSizer1841.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroSensorYPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroSensorYPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroSensorYPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1841.Add( self.panel_GyroSensorYPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2041 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2041.SetFlexibleDirection( wx.BOTH )
		fgSizer2041.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1341 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1341.Wrap( -1 )
		fgSizer2041.Add( self.m_staticText1341, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroYSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2041.Add( self.chkbox_GyroYSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroYSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroYSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_GyroYSnrMaxYVal.Enable( False )
		
		fgSizer2041.Add( self.text_GyroYSnrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer1841.Add( fgSizer2041, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer1841, 1, wx.EXPAND, 5 )
		
		fgSizer1851 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1851.AddGrowableCol( 0 )
		fgSizer1851.AddGrowableRow( 0 )
		fgSizer1851.SetFlexibleDirection( wx.BOTH )
		fgSizer1851.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroSensorZPlot = wx.Panel( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroSensorZPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroSensorZPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1851.Add( self.panel_GyroSensorZPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2051 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2051.SetFlexibleDirection( wx.BOTH )
		fgSizer2051.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1351 = wx.StaticText( self.m_panel7, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1351.Wrap( -1 )
		fgSizer2051.Add( self.m_staticText1351, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroZSnrYChk = wx.CheckBox( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2051.Add( self.chkbox_GyroZSnrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroZSnrMaxYVal = wx.TextCtrl( self.m_panel7, wx.ID_ANY, u"32767", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroZSnrMaxYVal.SetMaxLength( 6 ) 
		self.text_GyroZSnrMaxYVal.Enable( False )
		
		fgSizer2051.Add( self.text_GyroZSnrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer1851.Add( fgSizer2051, 1, wx.EXPAND, 5 )
		
		
		fgSizer186.Add( fgSizer1851, 1, wx.EXPAND, 5 )
		
		
		fgSizer171.Add( fgSizer186, 1, wx.EXPAND, 5 )
		
		fgSizer541 = wx.FlexGridSizer( 0, 1, 0, 0 )
		fgSizer541.AddGrowableCol( 0 )
		fgSizer541.AddGrowableRow( 0 )
		fgSizer541.SetFlexibleDirection( wx.BOTH )
		fgSizer541.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticline41 = wx.StaticLine( self.m_panel7, wx.ID_ANY, wx.DefaultPosition, wx.Size( 10000,-1 ), wx.LI_HORIZONTAL )
		self.m_staticline41.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.m_staticline41.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		self.m_staticline41.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		
		fgSizer541.Add( self.m_staticline41, 0, wx.EXPAND, 5 )
		
		
		fgSizer171.Add( fgSizer541, 1, wx.EXPAND, 5 )
		
		fgSizer211 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer211.AddGrowableCol( 0 )
		fgSizer211.AddGrowableRow( 0 )
		fgSizer211.SetFlexibleDirection( wx.BOTH )
		fgSizer211.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.text_SnrPlotSaveFileList = wx.TextCtrl( self.m_panel7, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 300,-1 ), 0 )
		self.text_SnrPlotSaveFileList.SetForegroundColour( wx.Colour( 255, 0, 0 ) )
		
		fgSizer211.Add( self.text_SnrPlotSaveFileList, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND, 5 )
		
		self.btn_SnrPlotSavePicture = wx.Button( self.m_panel7, wx.ID_ANY, u"Save picture", wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer211.Add( self.btn_SnrPlotSavePicture, 0, wx.ALL, 5 )
		
		self.btn_SnrPlotPause = wx.Button( self.m_panel7, wx.ID_ANY, u"Pause", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer211.Add( self.btn_SnrPlotPause, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )
		
		self.btn_SnrPlotReset = wx.Button( self.m_panel7, wx.ID_ANY, u"Reset", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer211.Add( self.btn_SnrPlotReset, 0, wx.ALL, 5 )
		
		
		fgSizer171.Add( fgSizer211, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.EXPAND, 5 )
		
		
		self.m_panel7.SetSizer( fgSizer171 )
		self.m_panel7.Layout()
		fgSizer171.Fit( self.m_panel7 )
		self.m_notebook2.AddPage( self.m_panel7, u"IMU sensor plot", False )
		self.m_panel14 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		fgSizer1711 = wx.FlexGridSizer( 3, 0, 0, 0 )
		fgSizer1711.AddGrowableCol( 0 )
		fgSizer1711.AddGrowableRow( 0 )
		fgSizer1711.SetFlexibleDirection( wx.BOTH )
		fgSizer1711.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer1861 = wx.FlexGridSizer( 2, 3, 0, 0 )
		fgSizer1861.AddGrowableCol( 0 )
		fgSizer1861.AddGrowableCol( 1 )
		fgSizer1861.AddGrowableCol( 2 )
		fgSizer1861.AddGrowableRow( 0 )
		fgSizer1861.AddGrowableRow( 1 )
		fgSizer1861.SetFlexibleDirection( wx.BOTH )
		fgSizer1861.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		fgSizer1911 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer1911.AddGrowableCol( 0 )
		fgSizer1911.AddGrowableRow( 0 )
		fgSizer1911.SetFlexibleDirection( wx.BOTH )
		fgSizer1911.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelVctrXPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelVctrXPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelVctrXPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer1911.Add( self.panel_AccelVctrXPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer2061 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2061.SetFlexibleDirection( wx.BOTH )
		fgSizer2061.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText1361 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText1361.Wrap( -1 )
		fgSizer2061.Add( self.m_staticText1361, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelXVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2061.Add( self.chkbox_AccelXVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelXVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelXVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_AccelXVctrMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_AccelXVctrMaxYVal.Enable( False )
		
		fgSizer2061.Add( self.text_AccelXVctrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer1911.Add( fgSizer2061, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer1911, 1, wx.EXPAND, 5 )
		
		fgSizer18111 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer18111.AddGrowableCol( 0 )
		fgSizer18111.AddGrowableRow( 0 )
		fgSizer18111.SetFlexibleDirection( wx.BOTH )
		fgSizer18111.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelVctrYPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelVctrYPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelVctrYPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer18111.Add( self.panel_AccelVctrYPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20111 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20111.SetFlexibleDirection( wx.BOTH )
		fgSizer20111.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13111 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13111.Wrap( -1 )
		fgSizer20111.Add( self.m_staticText13111, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelYVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20111.Add( self.chkbox_AccelYVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelYVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelYVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_AccelYVctrMaxYVal.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.text_AccelYVctrMaxYVal.Enable( False )
		
		fgSizer20111.Add( self.text_AccelYVctrMaxYVal, 0, wx.ALL, 5 )
		
		
		fgSizer18111.Add( fgSizer20111, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer18111, 1, wx.EXPAND, 5 )
		
		fgSizer18211 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer18211.AddGrowableCol( 0 )
		fgSizer18211.AddGrowableRow( 0 )
		fgSizer18211.SetFlexibleDirection( wx.BOTH )
		fgSizer18211.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_AccelVctrZPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_AccelVctrZPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_AccelVctrZPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer18211.Add( self.panel_AccelVctrZPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20211 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20211.SetFlexibleDirection( wx.BOTH )
		fgSizer20211.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13211 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13211.Wrap( -1 )
		fgSizer20211.Add( self.m_staticText13211, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_AccelZVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20211.Add( self.chkbox_AccelZVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_AccelZVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_AccelZVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_AccelZVctrMaxYVal.Enable( False )
		
		fgSizer20211.Add( self.text_AccelZVctrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer18211.Add( fgSizer20211, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer18211, 1, wx.EXPAND, 5 )
		
		fgSizer18311 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer18311.AddGrowableCol( 0 )
		fgSizer18311.AddGrowableRow( 0 )
		fgSizer18311.SetFlexibleDirection( wx.BOTH )
		fgSizer18311.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroVctrXPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroVctrXPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroVctrXPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer18311.Add( self.panel_GyroVctrXPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20311 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20311.SetFlexibleDirection( wx.BOTH )
		fgSizer20311.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13311 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13311.Wrap( -1 )
		fgSizer20311.Add( self.m_staticText13311, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroXVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20311.Add( self.chkbox_GyroXVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroXVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroXVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_GyroXVctrMaxYVal.Enable( False )
		
		fgSizer20311.Add( self.text_GyroXVctrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer18311.Add( fgSizer20311, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer18311, 1, wx.EXPAND, 5 )
		
		fgSizer18411 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer18411.AddGrowableCol( 0 )
		fgSizer18411.AddGrowableRow( 0 )
		fgSizer18411.SetFlexibleDirection( wx.BOTH )
		fgSizer18411.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroVctrYPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroVctrYPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroVctrYPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer18411.Add( self.panel_GyroVctrYPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20411 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20411.SetFlexibleDirection( wx.BOTH )
		fgSizer20411.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13411 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13411.Wrap( -1 )
		fgSizer20411.Add( self.m_staticText13411, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroYVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20411.Add( self.chkbox_GyroYVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroYVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroYVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_GyroYVctrMaxYVal.Enable( False )
		
		fgSizer20411.Add( self.text_GyroYVctrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer18411.Add( fgSizer20411, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer18411, 1, wx.EXPAND, 5 )
		
		fgSizer18511 = wx.FlexGridSizer( 2, 0, 0, 0 )
		fgSizer18511.AddGrowableCol( 0 )
		fgSizer18511.AddGrowableRow( 0 )
		fgSizer18511.SetFlexibleDirection( wx.BOTH )
		fgSizer18511.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.panel_GyroVctrZPlot = wx.Panel( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 150,120 ), wx.TAB_TRAVERSAL )
		self.panel_GyroVctrZPlot.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOW ) )
		self.panel_GyroVctrZPlot.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_BACKGROUND ) )
		
		fgSizer18511.Add( self.panel_GyroVctrZPlot, 1, wx.EXPAND |wx.ALL, 5 )
		
		fgSizer20511 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer20511.SetFlexibleDirection( wx.BOTH )
		fgSizer20511.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText13511 = wx.StaticText( self.m_panel14, wx.ID_ANY, u"Y range:", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText13511.Wrap( -1 )
		fgSizer20511.Add( self.m_staticText13511, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		self.chkbox_GyroZVctrYChk = wx.CheckBox( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer20511.Add( self.chkbox_GyroZVctrYChk, 0, wx.ALIGN_CENTER_VERTICAL, 5 )
		
		self.text_GyroZVctrMaxYVal = wx.TextCtrl( self.m_panel14, wx.ID_ANY, u"2147483647", wx.DefaultPosition, wx.Size( 60,-1 ), wx.TE_PROCESS_ENTER )
		self.text_GyroZVctrMaxYVal.SetMaxLength( 11 ) 
		self.text_GyroZVctrMaxYVal.Enable( False )
		
		fgSizer20511.Add( self.text_GyroZVctrMaxYVal, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL, 5 )
		
		
		fgSizer18511.Add( fgSizer20511, 1, wx.EXPAND, 5 )
		
		
		fgSizer1861.Add( fgSizer18511, 1, wx.EXPAND, 5 )
		
		
		fgSizer1711.Add( fgSizer1861, 1, wx.EXPAND, 5 )
		
		fgSizer5411 = wx.FlexGridSizer( 0, 1, 0, 0 )
		fgSizer5411.AddGrowableCol( 0 )
		fgSizer5411.AddGrowableRow( 0 )
		fgSizer5411.SetFlexibleDirection( wx.BOTH )
		fgSizer5411.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticline411 = wx.StaticLine( self.m_panel14, wx.ID_ANY, wx.DefaultPosition, wx.Size( 10000,-1 ), wx.LI_HORIZONTAL )
		self.m_staticline411.SetFont( wx.Font( wx.NORMAL_FONT.GetPointSize(), 70, 90, 90, False, wx.EmptyString ) )
		self.m_staticline411.SetForegroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		self.m_staticline411.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_WINDOWTEXT ) )
		
		fgSizer5411.Add( self.m_staticline411, 0, wx.EXPAND, 5 )
		
		
		fgSizer1711.Add( fgSizer5411, 1, wx.EXPAND, 5 )
		
		fgSizer2111 = wx.FlexGridSizer( 0, 4, 0, 0 )
		fgSizer2111.AddGrowableCol( 0 )
		fgSizer2111.AddGrowableRow( 0 )
		fgSizer2111.SetFlexibleDirection( wx.BOTH )
		fgSizer2111.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.text_VctrPlotSaveFileList = wx.TextCtrl( self.m_panel14, wx.ID_ANY, wx.EmptyString, wx.DefaultPosition, wx.Size( 300,-1 ), 0 )
		self.text_VctrPlotSaveFileList.SetForegroundColour( wx.Colour( 255, 0, 0 ) )
		
		fgSizer2111.Add( self.text_VctrPlotSaveFileList, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALL|wx.EXPAND, 5 )
		
		self.btn_VctrPlotSavePicture = wx.Button( self.m_panel14, wx.ID_ANY, u"Save picture", wx.DefaultPosition, wx.DefaultSize, 0 )
		fgSizer2111.Add( self.btn_VctrPlotSavePicture, 0, wx.ALL, 5 )
		
		self.btn_VctrPlotPause = wx.Button( self.m_panel14, wx.ID_ANY, u"Pause", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer2111.Add( self.btn_VctrPlotPause, 0, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.ALL, 5 )
		
		self.btn_VctrPlotReset = wx.Button( self.m_panel14, wx.ID_ANY, u"Reset", wx.DefaultPosition, wx.Size( 60,-1 ), 0 )
		fgSizer2111.Add( self.btn_VctrPlotReset, 0, wx.ALL, 5 )
		
		
		fgSizer1711.Add( fgSizer2111, 1, wx.ALIGN_CENTER_VERTICAL|wx.ALIGN_RIGHT|wx.EXPAND, 5 )
		
		
		self.m_panel14.SetSizer( fgSizer1711 )
		self.m_panel14.Layout()
		fgSizer1711.Fit( self.m_panel14 )
		self.m_notebook2.AddPage( self.m_panel14, u"IMU vector plot", False )
		self.m_panel15 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_notebook2.AddPage( self.m_panel15, u"a page", False )
		self.m_panel16 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_notebook2.AddPage( self.m_panel16, u"a page", False )
		self.m_panel17 = wx.Panel( self.m_notebook2, wx.ID_ANY, wx.DefaultPosition, wx.DefaultSize, wx.TAB_TRAVERSAL )
		self.m_notebook2.AddPage( self.m_panel17, u"a page", False )
		
		fgSizer4.Add( self.m_notebook2, 1, wx.ALL|wx.EXPAND, 5 )
		
		self.m_panel2 = wx.Panel( self, wx.ID_ANY, wx.DefaultPosition, wx.Size( 200,-1 ), wx.TAB_TRAVERSAL )
		fgSizer56 = wx.FlexGridSizer( 4, 1, 0, 0 )
		fgSizer56.SetFlexibleDirection( wx.BOTH )
		fgSizer56.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_panel30 = wx.Panel( self.m_panel2, wx.ID_ANY, wx.DefaultPosition, wx.Size( 200,120 ), wx.TAB_TRAVERSAL )
		self.m_panel30.SetBackgroundColour( wx.SystemSettings.GetColour( wx.SYS_COLOUR_ACTIVECAPTION ) )
		
		fgSizer57 = wx.FlexGridSizer( 5, 3, 0, 0 )
		fgSizer57.SetFlexibleDirection( wx.BOTH )
		fgSizer57.SetNonFlexibleGrowMode( wx.FLEX_GROWMODE_SPECIFIED )
		
		self.m_staticText24 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"InCh1", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText24.Wrap( -1 )
		self.m_staticText24.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.m_staticText24, 0, wx.ALL, 1 )
		
		self.slide_InputCh1 = wx.Slider( self.m_panel30, wx.ID_ANY, 1500, 900, 2100, wx.DefaultPosition, wx.Size( 140,15 ), wx.SL_HORIZONTAL )
		self.slide_InputCh1.Enable( False )
		
		fgSizer57.Add( self.slide_InputCh1, 0, wx.ALL, 1 )
		
		self.text_InputCh1 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"1500", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.text_InputCh1.Wrap( -1 )
		self.text_InputCh1.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.text_InputCh1, 0, wx.ALL, 1 )
		
		self.m_staticText25 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"InCh2", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText25.Wrap( -1 )
		self.m_staticText25.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.m_staticText25, 0, wx.ALL, 1 )
		
		self.slide_InputCh2 = wx.Slider( self.m_panel30, wx.ID_ANY, 1500, 900, 2100, wx.DefaultPosition, wx.Size( 140,15 ), wx.SL_HORIZONTAL )
		self.slide_InputCh2.Enable( False )
		
		fgSizer57.Add( self.slide_InputCh2, 0, wx.ALL, 1 )
		
		self.text_InputCh2 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"1500", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.text_InputCh2.Wrap( -1 )
		self.text_InputCh2.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.text_InputCh2, 0, wx.ALL, 1 )
		
		self.m_staticText26 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"InCh3", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText26.Wrap( -1 )
		self.m_staticText26.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.m_staticText26, 0, wx.ALL, 1 )
		
		self.slide_InputCh3 = wx.Slider( self.m_panel30, wx.ID_ANY, 1500, 900, 2100, wx.DefaultPosition, wx.Size( 140,15 ), wx.SL_HORIZONTAL )
		self.slide_InputCh3.Enable( False )
		
		fgSizer57.Add( self.slide_InputCh3, 0, wx.ALL, 1 )
		
		self.text_InputCh3 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"1500", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.text_InputCh3.Wrap( -1 )
		self.text_InputCh3.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.text_InputCh3, 0, wx.ALL, 1 )
		
		self.m_staticText27 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"InCh4", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText27.Wrap( -1 )
		self.m_staticText27.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.m_staticText27, 0, wx.ALL, 1 )
		
		self.slide_InputCh4 = wx.Slider( self.m_panel30, wx.ID_ANY, 1500, 900, 2100, wx.DefaultPosition, wx.Size( 140,15 ), wx.SL_HORIZONTAL )
		self.slide_InputCh4.Enable( False )
		
		fgSizer57.Add( self.slide_InputCh4, 0, wx.ALL, 1 )
		
		self.text_InputCh4 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"1500", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.text_InputCh4.Wrap( -1 )
		self.text_InputCh4.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.text_InputCh4, 0, wx.ALL, 1 )
		
		self.m_staticText28 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"InCh5", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.m_staticText28.Wrap( -1 )
		self.m_staticText28.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.m_staticText28, 0, wx.ALL, 1 )
		
		self.slide_InputCh5 = wx.Slider( self.m_panel30, wx.ID_ANY, 1500, 900, 2100, wx.DefaultPosition, wx.Size( 140,15 ), wx.SL_HORIZONTAL )
		self.slide_InputCh5.Enable( False )
		
		fgSizer57.Add( self.slide_InputCh5, 0, wx.ALL, 1 )
		
		self.text_InputCh5 = wx.StaticText( self.m_panel30, wx.ID_ANY, u"1500", wx.DefaultPosition, wx.DefaultSize, 0 )
		self.text_InputCh5.Wrap( -1 )
		self.text_InputCh5.SetFont( wx.Font( 9, 70, 90, 90, False, wx.EmptyString ) )
		
		fgSizer57.Add( self.text_InputCh5, 0, wx.ALL, 1 )
		
		
		self.m_panel30.SetSizer( fgSizer57 )
		self.m_panel30.Layout()
		fgSizer56.Add( self.m_panel30, 1, wx.EXPAND |wx.ALL, 0 )
		
		self.m_panel31 = wx.Panel( self.m_panel2, wx.ID_ANY, wx.DefaultPosition, wx.Size( 200,100 ), wx.TAB_TRAVERSAL )
		fgSizer56.Add( self.m_panel31, 1, wx.EXPAND |wx.ALL, 5 )
		
		self.m_panel32 = wx.Panel( self.m_panel2, wx.ID_ANY, wx.DefaultPosition, wx.Size( 200,150 ), wx.TAB_TRAVERSAL )
		fgSizer56.Add( self.m_panel32, 1, wx.EXPAND |wx.ALL, 5 )
		
		
		self.m_panel2.SetSizer( fgSizer56 )
		self.m_panel2.Layout()
		fgSizer4.Add( self.m_panel2, 1, wx.EXPAND |wx.ALL, 5 )
		
		
		fgSizer1.Add( fgSizer4, 1, wx.EXPAND, 5 )
		
		
		self.SetSizer( fgSizer1 )
		self.Layout()
		self.tmr_Console = wx.Timer()
		self.tmr_Console.SetOwner( self, tmp_console_id )
		self.tmr_Console.Start( 10 )
		
		self.tmr_AttPlot = wx.Timer()
		self.tmr_AttPlot.SetOwner( self, tmr_att_plot_id )
		self.tmr_AttPlot.Start( 20 )
		
		
		self.Centre( wx.BOTH )
		
		# Connect Events
		self.Bind( wx.EVT_ACTIVATE, self.OnMainFrmActivate )
		self.btn_SerialOpen.Bind( wx.EVT_BUTTON, self.OnSerialOpenClick )
		self.text_Console.Bind( wx.EVT_SET_FOCUS, self.OnCLISetFocus )
		self.btn_SaveLog.Bind( wx.EVT_BUTTON, self.OnSaveLogClick )
		self.chkbox_AutoScroll.Bind( wx.EVT_CHECKBOX, self.OnAutoScrollCheckBox )
		self.btn_ClearScreen.Bind( wx.EVT_BUTTON, self.OnCleanScreenClick )
		self.chkbox_RollMaxYChk.Bind( wx.EVT_CHECKBOX, self.OnRollMaxYCheckBox )
		self.text_RollMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnRollMaxYValTextEnter )
		self.chkbox_PitchMaxYChk.Bind( wx.EVT_CHECKBOX, self.OnPitchMaxYCheckBox )
		self.text_PitchMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnPitchMaxYValTextEnter )
		self.chkbox_RollMaxYChk2.Bind( wx.EVT_CHECKBOX, self.OnRollMaxYCheckBox )
		self.chkbox_RollMaxYChk3.Bind( wx.EVT_CHECKBOX, self.OnRollMaxYCheckBox )
		self.chkbox_RollMaxYChk4.Bind( wx.EVT_CHECKBOX, self.OnRollMaxYCheckBox )
		self.chkbox_RollMaxYChk5.Bind( wx.EVT_CHECKBOX, self.OnRollMaxYCheckBox )
		self.btn_AttPlotSavePicture.Bind( wx.EVT_BUTTON, self.OnAttPlotSavePictureButtonClick )
		self.btn_AttPlotPause.Bind( wx.EVT_BUTTON, self.OnAttPlotPauseButtonClick )
		self.btn_AttPlotReset.Bind( wx.EVT_BUTTON, self.OnAttPlotResetButtonClick )
		self.chkbox_AccelXSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelXSnrYCheckBox )
		self.text_AccelXSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelXMaxYValTextEnter )
		self.chkbox_AccelYSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelYSnrYCheckBox )
		self.text_AccelYSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelYMaxYValTextEnter )
		self.chkbox_AccelZSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelZSnrYCheckBox )
		self.text_AccelZSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelZMaxYValTextEnter )
		self.chkbox_GyroXSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroXSnrYCheckBox )
		self.text_GyroXSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroXMaxYValTextEnter )
		self.chkbox_GyroYSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroYSnrYCheckBox )
		self.text_GyroYSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroYMaxYValTextEnter )
		self.chkbox_GyroZSnrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroZSnrYCheckBox )
		self.text_GyroZSnrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroZMaxYValTextEnter )
		self.btn_SnrPlotSavePicture.Bind( wx.EVT_BUTTON, self.OnSnrPlotSavePictureButtonClick )
		self.btn_SnrPlotPause.Bind( wx.EVT_BUTTON, self.OnSnrPlotPauseButtonClick )
		self.btn_SnrPlotReset.Bind( wx.EVT_BUTTON, self.OnSnrPlotResetButtonClick )
		self.chkbox_AccelXVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelXVctrYCheckBox )
		self.text_AccelXVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelVctrXMaxYValTextEnter )
		self.chkbox_AccelYVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelYVctrYCheckBox )
		self.text_AccelYVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelVctrYMaxYValTextEnter )
		self.chkbox_AccelZVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnAccelZVctrYCheckBox )
		self.text_AccelZVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnAccelVctrZMaxYValTextEnter )
		self.chkbox_GyroXVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroXVctrYCheckBox )
		self.text_GyroXVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroVctrXMaxYValTextEnter )
		self.chkbox_GyroYVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroYVctrYCheckBox )
		self.text_GyroYVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroVctrYMaxYValTextEnter )
		self.chkbox_GyroZVctrYChk.Bind( wx.EVT_CHECKBOX, self.OnGyroZVctrYCheckBox )
		self.text_GyroZVctrMaxYVal.Bind( wx.EVT_TEXT_ENTER, self.OnGyroVctrZMaxYValTextEnter )
		self.btn_VctrPlotSavePicture.Bind( wx.EVT_BUTTON, self.OnVctrPlotSavePictureButtonClick )
		self.btn_VctrPlotPause.Bind( wx.EVT_BUTTON, self.OnVctrPlotPauseButtonClick )
		self.btn_VctrPlotReset.Bind( wx.EVT_BUTTON, self.OnVctrPlotResetButtonClick )
		self.Bind( wx.EVT_TIMER, self.OnConsoleTimer, id=tmp_console_id )
		self.Bind( wx.EVT_TIMER, self.OnAttPlotTimer, id=tmr_att_plot_id )
	
	def __del__( self ):
		pass
	
	
	# Virtual event handlers, overide them in your derived class
	def OnMainFrmActivate( self, event ):
		pass
	
	def OnSerialOpenClick( self, event ):
		pass
	
	def OnCLISetFocus( self, event ):
		pass
	
	def OnSaveLogClick( self, event ):
		pass
	
	def OnAutoScrollCheckBox( self, event ):
		pass
	
	def OnCleanScreenClick( self, event ):
		pass
	
	def OnRollMaxYCheckBox( self, event ):
		pass
	
	def OnRollMaxYValTextEnter( self, event ):
		pass
	
	def OnPitchMaxYCheckBox( self, event ):
		pass
	
	def OnPitchMaxYValTextEnter( self, event ):
		pass
	
	
	
	
	
	def OnAttPlotSavePictureButtonClick( self, event ):
		pass
	
	def OnAttPlotPauseButtonClick( self, event ):
		pass
	
	def OnAttPlotResetButtonClick( self, event ):
		pass
	
	def OnAccelXSnrYCheckBox( self, event ):
		pass
	
	def OnAccelXMaxYValTextEnter( self, event ):
		pass
	
	def OnAccelYSnrYCheckBox( self, event ):
		pass
	
	def OnAccelYMaxYValTextEnter( self, event ):
		pass
	
	def OnAccelZSnrYCheckBox( self, event ):
		pass
	
	def OnAccelZMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroXSnrYCheckBox( self, event ):
		pass
	
	def OnGyroXMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroYSnrYCheckBox( self, event ):
		pass
	
	def OnGyroYMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroZSnrYCheckBox( self, event ):
		pass
	
	def OnGyroZMaxYValTextEnter( self, event ):
		pass
	
	def OnSnrPlotSavePictureButtonClick( self, event ):
		pass
	
	def OnSnrPlotPauseButtonClick( self, event ):
		pass
	
	def OnSnrPlotResetButtonClick( self, event ):
		pass
	
	def OnAccelXVctrYCheckBox( self, event ):
		pass
	
	def OnAccelVctrXMaxYValTextEnter( self, event ):
		pass
	
	def OnAccelYVctrYCheckBox( self, event ):
		pass
	
	def OnAccelVctrYMaxYValTextEnter( self, event ):
		pass
	
	def OnAccelZVctrYCheckBox( self, event ):
		pass
	
	def OnAccelVctrZMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroXVctrYCheckBox( self, event ):
		pass
	
	def OnGyroVctrXMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroYVctrYCheckBox( self, event ):
		pass
	
	def OnGyroVctrYMaxYValTextEnter( self, event ):
		pass
	
	def OnGyroZVctrYCheckBox( self, event ):
		pass
	
	def OnGyroVctrZMaxYValTextEnter( self, event ):
		pass
	
	def OnVctrPlotSavePictureButtonClick( self, event ):
		pass
	
	def OnVctrPlotPauseButtonClick( self, event ):
		pass
	
	def OnVctrPlotResetButtonClick( self, event ):
		pass
	
	def OnConsoleTimer( self, event ):
		pass
	
	def OnAttPlotTimer( self, event ):
		pass
	

