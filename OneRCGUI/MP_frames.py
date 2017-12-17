#!/usr/bin/python
# -*- coding: UTF-8 -*-

from struct import *
from collections import namedtuple
import numpy as np


#******************************************************************************
# MP frame header
#******************************************************************************
"""
"""
MP_FRM_HDR_DEFINE    = np.array(
                        [
                            ['B', 's_f'],                                           # unsigned char
                            ['B', 'cmd'],                                           # unsigned char
                            ['B', 'seq'],                                           # unsigned char
                            ['B', 'len'],                                           # unsigned char
                        ])
MP_FRM_HDR_STRUCT    = np.array(
                        [   
                            'MP_HDR',                                               # Name
                            calcsize('=' + ''.join(MP_FRM_HDR_DEFINE[:, 0])),       # Size
                            ''.join(MP_FRM_HDR_DEFINE[:, 0]),                       # Field data type
                            ', '.join(MP_FRM_HDR_DEFINE[:, 1]),                     # Field name
                            
                        ])
"""
"""
MP_FRM_TAIL_DEFINE    = np.array(
                        [
                            ['H', 'CRC16'],                                         # unsigned char
                        ])
MP_FRM_TAIL_STRUCT    = np.array(
                        [   
                            'MP_TAIL',                                              # ID
                            calcsize('=' + ''.join(MP_FRM_TAIL_DEFINE[:, 0])),      # Size
                            ''.join(MP_FRM_TAIL_DEFINE[:, 0]),                      # Field data type
                            ', '.join(MP_FRM_TAIL_DEFINE[:, 1]),                    # Field name
                            
                        ])
                
#******************************************************************************
# FC system information payload
#******************************************************************************        
MP_HEARTBEAT_DEFINE     = np.array(
                        [
                            ['I', 'Heartbeat'],                                     # 4 bytes
                        ])
MP_HEARTBEAT_STRUCT     = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_HEARTBEAT_DEFINE[:, 0])),     # Size
                            ''.join(MP_HEARTBEAT_DEFINE[:, 0]),                     # Field data type
                            ', '.join(MP_HEARTBEAT_DEFINE[:, 1]),                   # Field name
                            
                        ])
      
MP_GENERAL_DEFINE       = np.array(
                        [
                            ['B', 'fly_mode'],                                      # 1 bytes
                            ['B', 'IMU_fail_cnt'],                                  # 1 bytes
                            ['B', 'AHRS_delay_cnt'],                                # 1 bytes
                            ['B', 'rcin_cyc_cnt'],                                  # 1 bytes
                            ['B', 'rcout_cyc_cnt'],                                 # 1 bytes
                            ['H', 'delta_ctrl_time'],                               # 2 bytes
                            ['f', 'MCU_Vcc'],                                       # 4 bytes
                        ])
MP_GENERAL_STRUCT       = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_GENERAL_DEFINE[:, 0])),       # Size
                            ''.join(MP_GENERAL_DEFINE[:, 0]),                       # Field data type
                            ', '.join(MP_GENERAL_DEFINE[:, 1]),                     # Field name
                            
                        ])
                        
MP_SETPOINT_DEFINE      = np.array(
                        [
                            ['f', 'roll'],                                          # 4 bytes
                            ['f', 'pitch'],                                         # 4 bytes
                            ['f', 'yaw'],                                           # 4 bytes
                        ])
MP_SETPOINT_STRUCT      = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_SETPOINT_DEFINE[:, 0])),      # Size
                            ''.join(MP_SETPOINT_DEFINE[:, 0]),                      # Field data type
                            ', '.join(MP_SETPOINT_DEFINE[:, 1]),                    # Field name
                            
                        ])

MP_CRUISE_DEFINE        = np.array(
                        [
                            ['B', 'cruise_state'],                                  # 1 bytes
                        ])
MP_CRUISE_STRUCT        = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_CRUISE_DEFINE[:, 0])),        # Size
                            ''.join(MP_CRUISE_DEFINE[:, 0]),                        # Field data type
                            ', '.join(MP_CRUISE_DEFINE[:, 1]),                      # Field name
                            
                        ])

#******************************************************************************
# IMU sensor information payload
#******************************************************************************        
MP_IMU_SENSOR_DEFINE    = np.array(
                        [
                            ['H', 'idx'],                                           # 2 bytes
                            ['h', 'accel_x'],                                       # 2 bytes
                            ['h', 'accel_y'],                                       # 2 bytes
                            ['h', 'accel_z'],                                       # 2 bytes
                            ['h', 'gyro_x'],                                        # 2 bytes
                            ['h', 'gyro_y'],                                        # 2 bytes
                            ['h', 'gyro_z'],                                        # 2 bytes
                            ['H', 'delta_t'],                                       # 2 bytes
                        ])
MP_IMU_SENSOR_STRUCT    = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_IMU_SENSOR_DEFINE[:, 0])),    # Size
                            ''.join(MP_IMU_SENSOR_DEFINE[:, 0]),                    # Field data type
                            ', '.join(MP_IMU_SENSOR_DEFINE[:, 1]),                  # Field name
                            
                        ])
                        
#******************************************************************************
# AHRS payload
#******************************************************************************
""" typedef struct ahrs_accel_data
"""
MP_AHRS_ACCEL_DEFINE    = np.array(
                        [
                            ['i', 'a_vctr_x'],                                      # 4 bytes
                            ['i', 'a_vctr_y'],                                      # 4 bytes
                            ['i', 'a_vctr_z'],                                      # 4 bytes
                            ['h', 'a_snr_x'],                                       # 2 bytes
                            ['h', 'a_snr_y'],                                       # 2 bytes
                            ['h', 'a_snr_z'],                                       # 2 bytes
                            ['i', 'G_SQ'],                                          # 4 bytes
                        ])
MP_AHRS_ACCEL_STRUCT    = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_AHRS_ACCEL_DEFINE[:, 0])),    # Size
                            ''.join(MP_AHRS_ACCEL_DEFINE[:, 0]),                    # Field data type
                            ', '.join(MP_AHRS_ACCEL_DEFINE[:, 1]),                  # Field name
                            
                        ])
  
""" typedef struct ahrs_data
"""      
MP_AHRS_START_DEFINE    = np.array(
                        [
                            ['H', 'dt']                                             # 2 bytes
                        ])
                                
MP_AHRS_END_DEFINE      = np.array(
                        [
                            ['B', 'accel_exceed_cnt'],                              # 1 bytes
                            ['h', 'g_snr_x'],                                       # 2 bytes
                            ['h', 'g_snr_y'],                                       # 2 bytes
                            ['h', 'g_snr_z'],                                       # 2 bytes
                            ['i', 'g_fxp_rad_x'],                                   # 4 bytes
                            ['i', 'g_fxp_rad_y'],                                   # 4 bytes
                            ['i', 'g_fxp_rad_z'],                                   # 4 bytes
                            ['i', 'level_vctr_x'],                                  # 4 bytes
                            ['i', 'level_vctr_y'],                                  # 4 bytes
                            ['i', 'level_vctr_z'],                                  # 4 bytes
                            ['i', 'heading_vctr_x'],                                # 4 bytes
                            ['i', 'heading_vctr_y'],                                # 4 bytes
                            ['i', 'heading_vctr_z'],                                # 4 bytes
                            ['f', 'ned_roll'],                                      # 4 bytes
                            ['f', 'ned_pitch'],                                     # 4 bytes
                            ['f', 'ned_head'],                                      # 4 bytes
                            ['f', 'body_roll'],                                     # 4 bytes
                            ['f', 'body_pitch'],                                    # 4 bytes
                            ['f', 'body_yaw'],                                      # 4 bytes
                        ])
MP_AHRS_DATA_DEFINE     = np.vstack((MP_AHRS_START_DEFINE, MP_AHRS_ACCEL_DEFINE, MP_AHRS_END_DEFINE))
MP_AHRS_DATA_STRUCT     = np.array(
                        [   
                            3,                                                      # ID
                            calcsize('=' + ''.join(MP_AHRS_DATA_DEFINE[:, 0])),     # Size
                            ''.join(MP_AHRS_DATA_DEFINE[:, 0]),                     # Field data type
                            ', '.join(MP_AHRS_DATA_DEFINE[:, 1]),                   # Field name
                        ])
                        
#******************************************************************************
# PID payload
#******************************************************************************

""" typedef struct pid_value
"""
MP_PID_VALUE_DEFINE    = np.array(
                        [
                            ['f', 'integral'],                                      # float
                            ['f', 'derivative'],                                    # float
                            ['f', 'output'],                                        # float
                            ['f', 'prev_error'],                                    # float
                            ['H', 'dt'],                                            # 2 bytes
                        ])
MP_PID_VALUE_STRUCT    = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_PID_VALUE_DEFINE[:, 0])),     # Size
                            ''.join(MP_PID_VALUE_DEFINE[:, 0]),                     # Field data type
                            ', '.join(MP_PID_VALUE_DEFINE[:, 1]),                   # Field name
                            
                        ])


""" typedef struct pid_config
"""
MP_PID_CONFIG_DEFINE    = np.array(
                        [
                            ['f', 'KP'],                                            # float
                            ['f', 'KI'],                                            # float
                            ['f', 'KD'],                                            # float
                            ['f', 'scale'],                                         # float
                            ['f', 'intg_max'],                                      # float
                            ['f', 'out_max'],                                       # float
                        ])
MP_PID_CONFIG_STRUCT    = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_PID_CONFIG_DEFINE[:, 0])),    # Size
                            ''.join(MP_PID_CONFIG_DEFINE[:, 0]),                    # Field data type
                            ', '.join(MP_PID_CONFIG_DEFINE[:, 1]),                  # Field name
                            
                        ])

""" typedef struct pid_data
"""      
MP_PID_DATA_DEFINE     = np.vstack((MP_PID_VALUE_DEFINE, MP_PID_CONFIG_DEFINE))
MP_PID_DATA_STRUCT     = np.array(
                        [   
                            3,                                                      # ID
                            calcsize('=' + ''.join(MP_PID_DATA_DEFINE[:, 0])),      # Size
                            ''.join(MP_PID_DATA_DEFINE[:, 0]),                      # Field data type
                            ', '.join(MP_PID_DATA_DEFINE[:, 1]),                    # Field name
                        ])                        
                        
#******************************************************************************
# RC pulse IN and OUT payload
#******************************************************************************        
MP_RC_IN_DEFINE         = np.array(
                        [
                            ['H', 'RCIN_0'],                                        # 2 bytes
                            ['H', 'RCIN_1'],                                        # 2 bytes
                            ['H', 'RCIN_2'],                                        # 2 bytes
                            ['H', 'RCIN_3'],                                        # 2 bytes
                            ['H', 'RCIN_4'],                                        # 2 bytes
                        ])
MP_RC_IN_STRUCT         = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_RC_IN_DEFINE[:, 0])),         # Size
                            ''.join(MP_RC_IN_DEFINE[:, 0]),                         # Field data type
                            ', '.join(MP_RC_IN_DEFINE[:, 1]),                       # Field name
                            
                        ])
                        
MP_RC_OUT_DEFINE        = np.array(
                        [
                            ['H', 'RCOUT_0'],                                       # 2 bytes
                            ['H', 'RCOUT_1'],                                       # 2 bytes
                            ['H', 'RCOUT_2'],                                       # 2 bytes
                            ['H', 'RCOUT_3'],                                       # 2 bytes
                        ])
MP_RC_OUT_STRUCT        = np.array(
                        [   
                            0,                                                      # ID
                            calcsize('=' + ''.join(MP_RC_OUT_DEFINE[:, 0])),        # Size
                            ''.join(MP_RC_OUT_DEFINE[:, 0]),                        # Field data type
                            ', '.join(MP_RC_OUT_DEFINE[:, 1]),                      # Field name
                            
                        ])
                        
#******************************************************************************
# GPS information payload
#******************************************************************************        

MP_GPS_GENERAL_DEFINE    = np.array(
                        [
                            ['B', 'gga_detect'],                                            # 1 bytes
                            ['B', 'gga_invalid'],                                           # 1 bytes
                            ['I', 'gga_timestamp'],                                         # 4 bytes
                            ['B', 'rmc_detect'],                                            # 1 bytes
                            ['B', 'rmc_invalid'],                                           # 1 bytes
                            ['I', 'rmc_timestamp'],                                         # 4 bytes
                            ['B', 'unknown_detect'],                                        # 1 bytes
                            ['I', 'unknown_timestamp'],                                     # 4 bytes                       
                        ])      
MP_GPS_GENERAL_STRUCT   = np.array(     
                        [           
                            0,                                                              # ID
                            calcsize('=' + ''.join(MP_GPS_GENERAL_DEFINE[:, 0])),           # Size
                            ''.join(MP_GPS_GENERAL_DEFINE[:, 0]),                           # Field data type
                            ', '.join(MP_GPS_GENERAL_DEFINE[:, 1]),                         # Field name
                            
                        ])

MP_GPS_GGA_NMEA_DEFINE  = np.array(
                        [
                            ['f', 'gga_UTC'],                                               # 1 bytes
                            ['f', 'latDD'],                                                 # 4 bytes
                            ['f', 'longDD'],                                                # 4 bytes  
                            ['B', 'fix_status'],                                            # 1 bytes
                            ['B', 'SAT_Used'],                                              # 1 bytes
                            ['f', 'HDOP'],                                                  # 4 bytes 
                            ['f', 'ALT_meters'],                                            # 4 bytes                        
                        ])
MP_GPS_GGA_NMEA_STRUCT  = np.array(
                        [   
                            0,                                                              # ID
                            calcsize('=' + ''.join(MP_GPS_GGA_NMEA_DEFINE[:, 0])),          # Size
                            ''.join(MP_GPS_GGA_NMEA_DEFINE[:, 0]),                          # Field data type
                            ', '.join(MP_GPS_GGA_NMEA_DEFINE[:, 1]),                        # Field name
                            
                        ])
                        
MP_GPS_RMC_NMEA_DEFINE  = np.array(
                        [
                            ['B', 'rmc_nav_status'],                                        # 1 bytes
                            ['f', 'gnd_speed_MS'],                                          # 4 bytes
                            ['f', 'COG_deg'],                                               # 4 bytes
                            ['I', 'date'],                                                  # 4 bytes
                            ['B', 'fix_status'],                                            # 1 bytes              
                        ])
MP_GPS_RMC_NMEA_STRUCT  = np.array(
                        [   
                            0,                                                              # ID
                            calcsize('=' + ''.join(MP_GPS_RMC_NMEA_DEFINE[:, 0])),          # Size
                            ''.join(MP_GPS_RMC_NMEA_DEFINE[:, 0]),                          # Field data type
                            ', '.join(MP_GPS_RMC_NMEA_DEFINE[:, 1]),                        # Field name
                            
                        ])

MP_GPS_WAYPOINT_DEFINE  = np.array(
                        [
                            ['B', 'wpt_is_set'],                                            # 1 bytes
                            ['B', 'is_valid'],                                              # 1 bytes
                            ['B', 'discard_cnt'],                                           # 1 bytes
                            ['B', 'update_cyc'],                                            # 1 bytes
                            ['f', 'latDD'],                                                 # 4 bytes
                            ['f', 'longDD'],                                                # 4 bytes   
                            ['f', 'bearing'],                                               # 4 bytes   
                            ['f', 'dist'],                                                  # 4 bytes 
                        
                        ])
MP_GPS_WAYPOINT_STRUCT  = np.array(
                        [   
                            0,                                                              # ID
                            calcsize('=' + ''.join(MP_GPS_WAYPOINT_DEFINE[:, 0])),          # Size
                            ''.join(MP_GPS_WAYPOINT_DEFINE[:, 0]),                          # Field data type
                            ', '.join(MP_GPS_WAYPOINT_DEFINE[:, 1]),                        # Field name
                            
                        ])
                        
MP_GPS_NAVIGATION_DEFINE    = np.array(
                            [
                                ['B', 'position_is_set'],                                   # 1 bytes
                                ['f', 'current_latDD'],                                     # 4 bytes
                                ['f', 'current_longDD'],                                    # 4 bytes                                   
                                ['B', 'nav_is_valid'],                                      # 1 bytes
                                ['f', 'bearing'],                                           # 4 bytes   
                        
                            ])
MP_GPS_NAVIGATION_STRUCT    = np.array(
                            [   
                                0,                                                          # ID
                                calcsize('=' + ''.join(MP_GPS_NAVIGATION_DEFINE[:, 0])),    # Size
                                ''.join(MP_GPS_NAVIGATION_DEFINE[:, 0]),                    # Field data type
                                ', '.join(MP_GPS_NAVIGATION_DEFINE[:, 1]),                  # Field name
                            ])
                            
MP_GPS_ERR_LOG_DEFINE    = np.array(
                            [
                                ['B', 'field_err'],                                         # 1 bytes
                                ['B', 'chksum_err'],                                        # 1 bytes
                                ['B', 'end_err'],                                           # 1 bytes                                   
                                ['B', 'rx_timeout'],                                        # 1 bytes 
                            ])
MP_GPS_ERR_LOG_STRUCT   = np.array(
                            [   
                                0,                                                          # ID
                                calcsize('=' + ''.join(MP_GPS_ERR_LOG_DEFINE[:, 0])),       # Size
                                ''.join(MP_GPS_ERR_LOG_DEFINE[:, 0]),                       # Field data type
                                ', '.join(MP_GPS_ERR_LOG_DEFINE[:, 1]),                     # Field name
                            ])


#******************************************************************************
# Payload ID mapping table
#******************************************************************************

# TX
MP_TX_IMU_SENSOR_DATA_ID    = 64

# RX
MP_HEARTBEAT_ID             = 128
MP_GENERAL_ID               = 129
MP_SETPOINT_ID              = 130
MP_CRUISE_ID                = 131

# RX GPS
MP_GPS_GENERAL_ID           = 161
MP_GPS_NMEA_FULL_ID         = 162
MP_GPS_NMEA_GGA_ID          = 163
MP_GPS_NMEA_RMC_ID          = 164
MP_GPS_WAYPOINT_ID          = 165
MP_GPS_NAVIGATION_ID        = 166
MP_GPS_ERR_LOG_ID           = 167

MP_RXIMU_SENSOR_DATA_ID     = 192

MP_AHRS_FULL_ID             = 200
MP_AHRS_ACCEL_ID            = 201

MP_PID_DATA_ROLL_ID         = 208
MP_PID_VAL_ROLL_ID          = 209
MP_PID_CFG_ROLL_ID          = 210

MP_PID_DATA_PITCH_ID        = 211
MP_PID_VAL_PITCH_ID         = 212
MP_PID_CFG_PITCH_ID         = 213

MP_PID_DATA_YAW_ID          = 214
MP_PID_VAL_YAW_ID           = 215
MP_PID_CFG_YAW_ID           = 216

MP_PID_DATA_BANK_ID         = 217
MP_PID_VAL_BANK_ID          = 218
MP_PID_CFG_BANK_ID          = 219

MP_RC_IN_ID                 = 224     
MP_RC_OUT_ID                = 240

# Lookup table
MP_FRM_TABLES               = {
                                MP_TX_IMU_SENSOR_DATA_ID:   MP_IMU_SENSOR_STRUCT,

                                MP_HEARTBEAT_ID:            MP_HEARTBEAT_STRUCT,  
                                MP_GENERAL_ID:              MP_GENERAL_STRUCT,
                                MP_SETPOINT_ID:             MP_SETPOINT_STRUCT,
                                MP_CRUISE_ID:               MP_CRUISE_STRUCT,
                                
                                MP_GPS_GENERAL_ID:          MP_GPS_GENERAL_STRUCT,
                                MP_GPS_NMEA_GGA_ID:         MP_GPS_GGA_NMEA_STRUCT,
                                MP_GPS_NMEA_RMC_ID:         MP_GPS_RMC_NMEA_STRUCT,
                                MP_GPS_WAYPOINT_ID:         MP_GPS_WAYPOINT_STRUCT,
                                MP_GPS_NAVIGATION_ID:       MP_GPS_NAVIGATION_STRUCT,
                                MP_GPS_ERR_LOG_ID:          MP_GPS_ERR_LOG_STRUCT,

                                MP_RXIMU_SENSOR_DATA_ID:    MP_IMU_SENSOR_STRUCT,

                                MP_AHRS_FULL_ID:            MP_AHRS_DATA_STRUCT,

                                MP_AHRS_ACCEL_ID:           MP_AHRS_ACCEL_STRUCT,

                                
                                MP_PID_DATA_ROLL_ID:        MP_PID_DATA_STRUCT,
                                MP_PID_VAL_ROLL_ID:         MP_PID_VALUE_STRUCT,
                                MP_PID_CFG_ROLL_ID:         MP_PID_CONFIG_STRUCT,
                                
                                MP_PID_DATA_PITCH_ID:       MP_PID_DATA_STRUCT,
                                MP_PID_VAL_PITCH_ID:        MP_PID_VALUE_STRUCT,
                                MP_PID_CFG_PITCH_ID:        MP_PID_CONFIG_STRUCT,
                                
                                MP_PID_DATA_YAW_ID:         MP_PID_DATA_STRUCT,
                                MP_PID_VAL_YAW_ID:          MP_PID_VALUE_STRUCT,
                                MP_PID_CFG_YAW_ID:          MP_PID_CONFIG_STRUCT,
                                
                                MP_PID_DATA_BANK_ID:         MP_PID_DATA_STRUCT,
                                MP_PID_VAL_BANK_ID:          MP_PID_VALUE_STRUCT,
                                MP_PID_CFG_BANK_ID:          MP_PID_CONFIG_STRUCT,
                                
                                MP_RC_IN_ID:                MP_RC_IN_STRUCT,
                                MP_RC_OUT_ID:               MP_RC_OUT_STRUCT,
                            }
