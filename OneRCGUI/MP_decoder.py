#!/usr/bin/python
# -*- coding: UTF-8 -*-

from MP_frames import *
from struct import *
from collections import namedtuple
import numpy as np
import os

class MP_decoder():
    
    __mp_frame_payload_format = ''
    __mp_frame_payload_fields = 'payload'
    __mp_frame_payload_struct = namedtuple('frame_payload_struct', __mp_frame_payload_fields)

    
    """ Decode MP frame raw bytes and convert to completed frame structure
    
    """
    def __init__(self):
        pass
      
      
    """ Decode MP frame raw bytes and convert to completed frame structure
    
    """
    def decode(self, raw_bytes):
    
        byte_start = 0
        byte_end = 0

        # Check input data size, should equal or bigger than size(header) + size(tail)
        if(len(raw_bytes) < int(MP_FRM_HDR_STRUCT[1]) + int(MP_FRM_TAIL_STRUCT[1])):
            return None

        # Decode frame header part
        byte_start = 0
        byte_end = byte_start + int(MP_FRM_HDR_STRUCT[1])
        hdr = self.decode_hdr(raw_bytes[byte_start:byte_end])

        # Check "len" field, and check whether it is correct or not.
        if(hdr.len !=  len(raw_bytes) - int(MP_FRM_HDR_STRUCT[1]) - int(MP_FRM_TAIL_STRUCT[1])):
            return None
            
        try:
            payload_struct = MP_FRM_TABLES[hdr.cmd]
        except:
            return None
            
        if(int(payload_struct[1]) != hdr.len):
            return None
            
        mp_frame_format = '=' + MP_FRM_HDR_STRUCT[2] + payload_struct[2] + MP_FRM_TAIL_STRUCT[2]
        mp_frame_fields = MP_FRM_HDR_STRUCT[3] + ', ' + payload_struct[3] + ', ' + MP_FRM_TAIL_STRUCT[3]
        mp_frame_struct = namedtuple('MP_FRM', mp_frame_fields)
            
        return mp_frame_struct._make(unpack(mp_frame_format, raw_bytes))

        
    """ Decode MP frame raw bytes header part
    
    """
    def decode_hdr(self, raw_bytes):
    
        hdr_struct = namedtuple(MP_FRM_HDR_STRUCT[0], MP_FRM_HDR_STRUCT[3])
        return hdr_struct._make(unpack(MP_FRM_HDR_STRUCT[2], raw_bytes))

        pass
      
      
    """ Decode MP frame raw bytes payload part
    
    """      
    def decode_payload(self, raw_bytes, byte_size):
        pass
       
       
    """ Decode MP frame raw bytes tail part
    
    """       
    def decode_tail(self, raw_bytes):
    
        hdr_struct = namedtuple(MP_FRM_HDR_STRUCT[0], MP_FRM_TAIL_STRUCT[3])
        return hdr_struct._make(unpack(MP_FRM_HDR_STRUCT[2], raw_bytes))
    
        pass

#mp_deocder = MP_decoder()

#a =  mp_deocder.decode(b'\x01\x02\x03\x03\xAA\xBB\xCC\x05')
#print a
#print len(a)