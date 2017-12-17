#!/usr/bin/python
# -*- coding: UTF-8 -*-

import threading
import time
import serial
import sys
from time import sleep
from struct import *
import Queue
from MP_frames import *
from MP_decoder import MP_decoder
from MP_CRC import *
import datetime

class MP_handler(MP_decoder):

    __port_name = None
    __baud_rate = None
    __serial = None
    __serial_tx_queue = Queue.Queue(0)
    __serial_rx_queue = Queue.Queue(0)
    
    __frame_rx_queue = None

    __frame_string = ''
    
    __frame_sflag = 0
    __frame_rx_bytes = 0
    __frame_checksum_offset = 0
    __frame_drop = 0
    
    __frame_rx_cmd = 0
    __frame_rx_sequence = 0
    
    __frame_tx_sequence = 0
   
    __transmitter_thread = None   
    __receiver_thread = None
    __detector_thread = None
    __is_transmitter_en = False
    __is_receiver_en = False
    __is_detector_en = False
    
    __rx_crc_object = None
    __tx_crc_object = None
    
    __rx_frm_cnt = int(0)
    __rx_crc_err_cnt = int(0)
    __rx_seq_err_cnt = int(0)

    def __init__(self):

        self.__rx_crc_object = MP_crc()
        self.__tx_crc_object = MP_crc()
    
        MP_decoder.__init__(self)       

    def open_serial(self, port_name, baud_rate):

        self.__port_name = port_name
        self.__baud_rate = baud_rate
        self.__serial = serial.Serial(self.__port_name, self.__baud_rate,                       \
                                      timeout = 0.01,  bytesize = 8, parity = 'N', stopbits = 1)
                                      
    def close_serial(self):
    
        self.__serial.flush()
        self.__serial.close()
        self.__serial = None
    
        pass
      
    def transmit_frame(self, mp_command, tx_payload):
    
        tx_header = pack(MP_FRM_HDR_STRUCT[2],          # header structure definition
                         0x7E,                          # header start flag
                         mp_command,                    # command
                         self.__frame_tx_sequence,      # sequence number
                         len(tx_payload))               # length of payload
        
        tx_bytes = tx_header + tx_payload
        
        self.__tx_crc_object.crc = 0xFFFF
        self.__tx_crc_object.accumulate_str(tx_bytes[1:])

        tx_tail = pack(MP_FRM_TAIL_STRUCT[2], self.__tx_crc_object.crc)
        tx_frame = tx_header + tx_payload + tx_tail

        self.__serial_tx_queue.put(tx_frame, True, None)
        
        self.__frame_tx_sequence += 1
        if(self.__frame_tx_sequence > 255):
            self.__frame_tx_sequence = 0
            
    def __serial_transmitter(self):
    
        while(self.__is_transmitter_en):

            try:
                tx_data = self.__serial_tx_queue.get(True, 1.0)

                if(tx_data):
                    output_byte = self.__serial.write(tx_data)
                    if(output_byte != len(tx_data)):
                        print "Uart TX data length is mismatched"
                    
            except:
                pass
        pass
    
      
    def set_rx_frame_queue(self, rx_frame_queue):
        self.__frame_rx_queue = rx_frame_queue
            
    def __serial_receiver(self):
    
        while(self.__is_receiver_en):

            serial_input = self.__serial.read(100000)
            if(serial_input):
                self.__serial_rx_queue.put(serial_input, True, None)
    
        pass
    
    def __frame_detector(self):
        
        while(self.__is_detector_en):
        
            try:
                input = self.__serial_rx_queue.get(True, 1.0)
                
                for byte in input:
                
                    num = ord(byte)
                
                    # Start to collect frame bytes
                    if(self.__frame_sflag == 0x7E):
                    
                        self.__frame_string += byte
                        self.__frame_rx_bytes += 1
                
                        # Frame command field 
                        if(self.__frame_rx_bytes == 2):
                            self.__frame_rx_cmd = num
                        # Frame sequence filed
                        elif(self.__frame_rx_bytes == 3):
                            sequence_tmp = num
                        # Frame length field
                        elif(self.__frame_rx_bytes == 4):
                            self.__frame_checksum_offset = self.__frame_rx_bytes + num + 1
                            
                        # Compare CRC 
                        if(self.__frame_rx_bytes == self.__frame_checksum_offset + 1):
                        
                            self.__rx_crc_object.accumulate_str(self.__frame_string[1:-2])
                            
                            # Put complete frame in the queue
                            frame_struct = self.decode(self.__frame_string)

                            if(frame_struct != None):
                                
                                # CRC pass
                                if(frame_struct.CRC16 == self.__rx_crc_object.crc):
                                
                                    self.__rx_frm_cnt += 1
                                        
                                    # Check frame sequence
                                    self.__frame_rx_sequence += 1
                                    if(self.__frame_rx_sequence > 255):
                                        self.__frame_rx_sequence = 0
                                    
                                    # Increase frame drop counter if sequence is mismatched                               
                                    if(self.__frame_rx_sequence != sequence_tmp):
                                        self.__rx_seq_err_cnt += 1
                                        self.__frame_drop += 1
                                        self.__frame_rx_sequence = sequence_tmp
                                
                                    if(self.__frame_rx_queue != None):
                                    
                                        rx_time = datetime.datetime.now().strftime("%H:%M:%S.%f")
                                        rx_frame = {"data" : frame_struct, "rx_time" : rx_time}
                            
                                        self.__frame_rx_queue.put(rx_frame, True, None)
                                    else:
                                        print frame_struct
                                
                                # Incorrect CRC 
                                else:
                                    self.__rx_crc_err_cnt += 1
                                    print "Frm cnt, CRC err, seq err = ", self.__rx_frm_cnt, ", ", self.__rx_crc_err_cnt, ", ", self.__rx_seq_err_cnt
                                    
                            else:
                                self.__rx_crc_err_cnt += 1
                                print "Frm cnt, CRC err, seq err = ", self.__rx_frm_cnt, ", ", self.__rx_crc_err_cnt, ", ", self.__rx_seq_err_cnt
 
                                    
                            # Reset RX frame status
                            self.__frame_sflag = 0x00
                
                    # Detect frame start flag
                    elif(num == 0x7E):
                        self.__frame_sflag = 0x7E
                        self.__frame_rx_bytes = 1
                        self.__frame_checksum_offset = 0
                        
                        self.__frame_string = byte
                        
                        self.__rx_crc_object.crc = 0xFFFF
            except:    
                pass
        
    def thread_start(self):
    
        self.__frame_sflag = 0
        self.__is_transmitter_en = True
        self.__is_receiver_en = True
        self.__is_detector_en = True
        self.__serial_rx_queue = Queue.Queue(0)
    
        self.__transmitter_thread = threading.Thread(target = self.__serial_transmitter)
        self.__receiver_thread = threading.Thread(target = self.__serial_receiver)
        self.__detector_thread = threading.Thread(target = self.__frame_detector)
        self.__transmitter_thread.start()
        self.__receiver_thread.start()
        self.__detector_thread.start()
        
        pass
        
    def thread_stop(self):
    
        self.__is_transmitter_en = False
        self.__is_receiver_en = False
        self.__is_detector_en = False
    
        self.__transmitter_thread.join()
        self.__receiver_thread.join()
        self.__detector_thread.join()
    
        pass
        
"""
mp_handler = MP_handler()
mp_handler.open_serial("COM6", 115200)
mp_handler.thread_start()
"""

