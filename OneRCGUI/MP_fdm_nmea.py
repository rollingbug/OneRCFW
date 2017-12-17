#!/usr/bin/python
# -*- coding: UTF-8 -*-

import serial
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
UDP_RX_PORT = 5505

rx_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
rx_sock.bind((UDP_IP, UDP_RX_PORT))

nmea_serial = serial.Serial("COM6", 9600, timeout = 0.01,  bytesize = 8, parity = 'N', stopbits = 1)

__mp_rx_frame_queue = Queue.Queue(0)


def UDP_RX_Thread():

    while(True):
        data = rx_sock.recv(1024)
        print data
        nmea_msgs = data.split();
        for nmea in nmea_msgs[:]:
            if('$GPGGA' in nmea):
                nmea = nmea + "\r\n"
                nmea_serial.write(nmea)
                
            if('$GPRMC' in nmea):
                nmea = nmea + "\r\n"
                nmea_serial.write(nmea)
    pass

udp_rx_thread = threading.Thread(target = UDP_RX_Thread)
udp_rx_thread.start()
    
while(True):

    time.sleep(1)
