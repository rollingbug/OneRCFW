/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    mcu_protocol.cpp
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef MCU_PROTOCOL_H_
#define MCU_PROTOCOL_H_

#include <stdint.h>


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define MP_FRM_SFLAG            0x7E    /* Frame start flag */

#define MP_TX_FRM_BUF_SIZE      128
#define MP_RX_FRM_BUF_SIZE      64


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* MP request command (For host side) */
typedef enum mp_req_cmd{
    /* General */
    MP_REQ_SYS_HEARTBEAT    = 0,
    MP_REQ_SYS_GENERAL,
    MP_REQ_SYS_SETPOINT,
    MP_REQ_SYS_CRUISE_STATE,
    MP_REQ_SYS_RESERVED     = 31,

    /* GPS */
    MP_REQ_GPS_DATA         = 32,
    MP_REQ_GPS_GENERAL,
    MP_REQ_GPS_NMEA_FULL,
    MP_REQ_GPS_NMEA_GGA,
    MP_REQ_GPS_NMEA_RMC,
    MP_REQ_GPS_WAYPOINT,
    MP_REQ_GPS_NAVIGATION,
    MP_REQ_GPS_ERR_LOG,

    /* IMU and AHRS */
    MP_REQ_IMU_SENSOR_DATA  = 64,
    MP_REQ_NED_ANGLE_DATA,

    MP_REQ_AHRS_FULL        = 72,
    MP_REQ_AHRS_ACCEL,

    /* PID */
    MP_REQ_PID_DATA_ROLL    = 80,
    MP_REQ_PID_VAL_ROLL,
    MP_REQ_PID_CFG_ROLL,

    MP_REQ_PID_DATA_PITCH,
    MP_REQ_PID_VAL_PITCH,
    MP_REQ_PID_CFG_PITCH,

    MP_REQ_PID_DATA_YAW,
    MP_REQ_PID_VAL_YAW,
    MP_REQ_PID_CFG_YAW,

    MP_REQ_PID_DATA_BANK,
    MP_REQ_PID_VAL_BANK,
    MP_REQ_PID_CFG_BANK,

    /* RC control */
    MP_REQ_IN_CHANNELS      = 96,
    MP_REQ_OUT_CHANNELS     = 112,

    /* For normal ASCII message */
    MP_REQ_ASCII_MSG        = 120,

    /* Never use */
    MP_REQ_END              = 127
}MP_REQ_CMD;

/* MP response command (For MCU side) */
typedef enum mp_rsp_cmd{
    /* General */
    MP_RSP_SYS_HEARTBEAT    = MP_REQ_SYS_HEARTBEAT + 128,
    MP_RSP_SYS_GENERAL      = MP_REQ_SYS_GENERAL + 128,
    MP_RSP_SYS_SETPOINT     = MP_REQ_SYS_SETPOINT + 128,
    MP_RSP_SYS_CRUISE_STATE = MP_REQ_SYS_CRUISE_STATE + 128,
    MP_RSP_SYS_RESERVED     = MP_REQ_SYS_RESERVED + 128,

    /* GPS */
    MP_RSP_GPS_DATA         = MP_REQ_GPS_DATA + 128,
    MP_RSP_GPS_GENERAL      = MP_REQ_GPS_GENERAL + 128,
    MP_RSP_GPS_NMEA_FULL    = MP_REQ_GPS_NMEA_FULL + 128,
    MP_RSP_GPS_NMEA_GGA     = MP_REQ_GPS_NMEA_GGA + 128,
    MP_RSP_GPS_NMEA_RMC     = MP_REQ_GPS_NMEA_RMC + 128,
    MP_RSP_GPS_WAYPOINT     = MP_REQ_GPS_WAYPOINT + 128,
    MP_RSP_GPS_NAVIGATION   = MP_REQ_GPS_NAVIGATION + 128,
    MP_RSP_GPS_ERR_LOG      = MP_REQ_GPS_ERR_LOG + 128,

    /* IMU */
    MP_RSP_IMU_SENSOR_DATA  = MP_REQ_IMU_SENSOR_DATA + 128,
    MP_RSP_NED_ANGLE_DATA,

    /* AHRS */
    MP_RSP_AHRS_FULL        = MP_REQ_AHRS_FULL + 128,
    MP_RSP_AHRS_ACCEL,

    /* PID */
    MP_RSP_PID_DATA_ROLL    = MP_REQ_PID_DATA_ROLL + 128,
    MP_RSP_PID_VAL_ROLL,
    MP_RSP_PID_CFG_ROLL,

    MP_RSP_PID_DATA_PITCH,
    MP_RSP_PID_VAL_PITCH,
    MP_RSP_PID_CFG_PITCH,

    MP_RSP_PID_DATA_YAW,
    MP_RSP_PID_VAL_YAW,
    MP_RSP_PID_CFG_YAW,

    MP_RSP_PID_DATA_BANK,
    MP_RSP_PID_VAL_BANK,
    MP_RSP_PID_CFG_BANK,

    /* RC control */
    MP_RSP_IN_CHANNELS      = MP_REQ_IN_CHANNELS + 128,
    MP_RSP_OUT_CHANNELS     = MP_REQ_OUT_CHANNELS + 128,

    /* For normal ASCII message */
    MP_RSP_ASCII_MSG        = MP_REQ_ASCII_MSG + 128,

    /* Never use */
    MP_RSP_END              = MP_REQ_END + 128
}MP_RSP_CMD;

/* Frame header of MCU protocol */
typedef struct mp_frame_hdr{
    uint8_t s_flag;         /* 0x7E */
    uint8_t cmd;            /* Command */
    uint8_t sequence;       /* Sequence number */
    uint8_t len;            /* Length of payload not including checksum */
    uint8_t payload[0];     /* Payload */
}__attribute__((packed)) MP_FRAME_HDR;

/* Frame tail of MCU protocol */
typedef struct mp_frame_tail{
    uint16_t CRC16;         /* 16 bits CRC (CRC-16-CCITT) appended after data */
}__attribute__((packed)) MP_FRAME_TAIL;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

int8_t MP_Init();
uint8_t MP_Send(uint8_t cmd, uint8_t *p_data, uint8_t data_size);
uint8_t MP_Recv(uint8_t *p_frm_buf, uint8_t frm_buf_size);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // MCU_PROTOCOL_H_
