#!/usr/bin/python
# -*- coding: UTF-8 -*-

import socket
from struct import *
from collections import namedtuple
import numpy as np
from MP_handler import MP_handler
import Queue
import time
from MP_frames import *
import threading

UDP_IP = "127.0.0.1"
UDP_TX_PORT = 5500
UDP_RX_PORT = 5501

FLIGHTGEAR_RX_DEFINE    = np.array(
                        [
                            ['d', 'roll_angle'],                                        # double
                            ['d', 'pitch_angle'],                                       # double
                            ['d', 'yaw_angle'],                                         # double
                            ['d', 'latitude'],                                          # double
                            ['d', 'longitude'],                                         # double
                            ['d', 'altitude'],                                          # double
                            ['d', 'heading'],                                           # double
                            ['d', 'speed_north'],                                       # double
                            ['d', 'speed_east'],                                        # double
                            ['d', 'x_accel'],                                           # double    
                            ['d', 'y_accel'],                                           # double
                            ['d', 'z_accel'],                                           # double
                            ['d', 'roll_rate'],                                         # double
                            ['d', 'pitch_rate'],                                        # double     
                            ['d', 'yaw_rate'],                                          # double
                            ['I', 'magic'],                                             # unsigned int                           
                        ])
FLIGHTGEAR_RX_STRUCT    = np.array(
                        [   
                            'MP_HDR',                                                   # Name
                            calcsize('!' + ''.join(FLIGHTGEAR_RX_DEFINE[:, 0])),        # Size
                            ''.join(FLIGHTGEAR_RX_DEFINE[:, 0]),                        # Field data type
                            ', '.join(FLIGHTGEAR_RX_DEFINE[:, 1]),                      # Field name
                            
                        ])

FLIGHTGEAR_TX_DEFINE    = np.array(
                        [
                            ['d', 'throttle', 0],                                          # double 
                            ['d', 'aileron', 0],                                           # double
                            ['d', 'elevator', 0],                                          # double
                            ['d', 'rudder', 0],                                            # double
                        ])
FLIGHTGEAR_TX_STRUCT    = np.array(
                        [   
                            'MP_HDR',                                                   # Name
                            calcsize('!' + ''.join(FLIGHTGEAR_TX_DEFINE[:, 0])),        # Size
                            ''.join(FLIGHTGEAR_TX_DEFINE[:, 0]),                        # Field data type
                            ', '.join(FLIGHTGEAR_TX_DEFINE[:, 1]),                      # Field name
                            ', '.join(FLIGHTGEAR_TX_DEFINE[:, 2]),                      # Initial value
                            
                        ])

                        
tx_frame_format = '!' + FLIGHTGEAR_TX_STRUCT[2]
tx_frame_fields = FLIGHTGEAR_TX_STRUCT[3]
tx_frame_struct = namedtuple('FLIGHTGEAR_TX', FLIGHTGEAR_TX_STRUCT[3])        


tx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP
rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP

rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
rx_sock.bind((UDP_IP, UDP_RX_PORT))

__mp_rx_frame_queue = Queue.Queue(0)
__mp_handler = MP_handler()
__mp_handler.set_rx_frame_queue(__mp_rx_frame_queue)
__mp_handler.open_serial('COM3', int(57600))
__mp_handler.thread_start()

def restrict (val, minval, maxval):
    if val < minval: return minval
    if val > maxval: return maxval
    return val

def UDP_RX_Thread():

    while(True):
        data = rx_sock.recv(1024)

        mp_frame_format = '!' + FLIGHTGEAR_RX_STRUCT[2]
        mp_frame_fields = FLIGHTGEAR_RX_STRUCT[3]
        mp_frame_struct = namedtuple('FLIGHTGEAR_RX', mp_frame_fields)
        
        flightgear_info = mp_frame_struct._make(unpack(mp_frame_format, data))
        
        mp_x_accel16 = int(flightgear_info.x_accel / 32.2 * 4096.0)         #- == pitch down
        mp_y_accel16 = int(flightgear_info.y_accel / 32.2 * -4096.0)        #- right  down
        mp_z_accel16 = int(flightgear_info.z_accel / 32.2 * -4096.0)        #- in FG
        
        mp_x_gyro16 = int(flightgear_info.roll_rate * 16.4)                 #+ right
        mp_y_gyro16 = int(flightgear_info.pitch_rate * -16.4)               #+ up
        mp_z_gyro16 = int(flightgear_info.yaw_rate * -16.4)                 #+ right

        mp_x_accel16 = restrict(mp_x_accel16, -32768, 32767)
        mp_y_accel16 = restrict(mp_y_accel16, -32768, 32767)
        mp_z_accel16 = restrict(mp_z_accel16, -32768, 32767)
        mp_x_gyro16 = restrict(mp_x_gyro16, -32768, 32767)
        mp_y_gyro16 = restrict(mp_y_gyro16, -32768, 32767)
        mp_z_gyro16 = restrict(mp_z_gyro16, -32768, 32767)
        """
        tx_mpu_payload = pack("<" + MP_IMU_SENSOR_STRUCT[2],
                              0, 
                              mp_x_accel16, mp_y_accel16, mp_z_accel16,
                              mp_x_gyro16, mp_y_gyro16, mp_z_gyro16,
                              0)
        
        __mp_handler.transmit_frame(MP_TX_IMU_SENSOR_DATA_ID, tx_mpu_payload)
        """
        tx_mpu_payload = pack("<fff", 
                              flightgear_info.roll_angle,
                              flightgear_info.pitch_angle,
                              flightgear_info.yaw_angle)
        
        __mp_handler.transmit_frame(MP_TX_IMU_SENSOR_DATA_ID + 1, tx_mpu_payload)

        #print mp_x_accel16, mp_y_accel16, mp_z_accel16, mp_x_gyro16, mp_y_gyro16, mp_z_gyro16
        #print flightgear_info.roll_angle, flightgear_info.pitch_angle, flightgear_info.yaw_angle
        
    pass

udp_rx_thread = threading.Thread(target = UDP_RX_Thread)
udp_rx_thread.start()
    
while(True):

    while(1):

        rx_frame = __mp_rx_frame_queue.get(True, None)
        if(rx_frame["data"].cmd == MP_RC_OUT_ID):
        
            tx_frame = pack(tx_frame_format, 
                            ((rx_frame["data"].RCOUT_0 - 2000.0) / 2000.0), 
                            ((rx_frame["data"].RCOUT_1 - 3000.0) / -1000.0),    # Roll
                            ((rx_frame["data"].RCOUT_2 - 3000.0) / 1000.0),     # Pitch, 
                            ((rx_frame["data"].RCOUT_3 - 3000.0) / -1000.0))    # YAW
            tx_sock.sendto(tx_frame, (UDP_IP, UDP_TX_PORT))
           
        """
        if(rx_frame["data"].cmd == MP_AHRS_FULL_ID):
           print rx_frame["data"].accel_exceed_cnt
           print "[", rx_frame["data"].pitch, rx_frame["data"].roll, rx_frame["data"].level_vctr_x, rx_frame["data"].level_vctr_y, rx_frame["data"].level_vctr_z,"]"
        """

        """
        if(rx_frame["data"].cmd == MP_AHRS_FULL_ID):
            print "AHRS: roll ", rx_frame["data"].ned_roll, "pitch ", rx_frame["data"].ned_pitch, "yaw ", rx_frame["data"].ned_head
                                
        if(rx_frame["data"].cmd == MP_GPS_GENERAL_ID):
            print rx_frame["data"]
                                
        if(rx_frame["data"].cmd == MP_GPS_NMEA_GGA_ID):
            print rx_frame["data"]
            
        if(rx_frame["data"].cmd == MP_GPS_NMEA_RMC_ID):
            print rx_frame["data"]
                                
        if(rx_frame["data"].cmd == MP_GPS_WAYPOINT_ID):
            print rx_frame["data"]
                                
        if(rx_frame["data"].cmd == MP_GPS_NAVIGATION_ID):
            print rx_frame["data"]
        """
        
        """
        if(rx_frame["data"].cmd == MP_PID_CFG_YAW_ID):
           print "KP = ", rx_frame["data"].KP, "KI = ",rx_frame["data"].KI, "KD = ",rx_frame["data"].KD
        """
        
        if(rx_frame["data"].cmd == MP_PID_CFG_PITCH_ID):
           print "Scale = ", rx_frame["data"].scale
           
        if(rx_frame["data"].cmd == MP_PID_VAL_PITCH_ID):
           print "Prev_error = ", rx_frame["data"].prev_error
           
        if(rx_frame["data"].cmd == MP_RC_OUT_ID):
           print "RCOUT_2 = ", rx_frame["data"].RCOUT_2
           
        if(rx_frame["data"].cmd == MP_SETPOINT_ID):
           print "setpoint = ", rx_frame["data"].pitch
        
    #time.sleep(10.0 / 1000.0)
    
__mp_handler.thread_stop()