/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    ublox6m_drv.h
 * @brief   Ublox NEO 6M GPS module driver.
 *
 * UBX message data format:
 * Short    Type                Bytes   Comment         Min/Max             Resolution
 * U1       Unsigned Char       1                       0..255              1
 * I1       Signed Char         1       2's complement  -128..127           1
 * X1       Bitfield            1                       n/a                 n/a
 * U2       Unsigned Short      2                       0..65535            1
 * I2       Signed Short        2       2's complement  -32768..32767       1
 * X2       Bitfield            2                       n/a                 n/a
 * U4       Unsigned Long       4                       0..4'294'967'295    1
 * I4       Signed Long         4       2's complement  -2'147'483'648 ..   1
 *                                                      2'147'483'647
 * X4       Bitfield            4                       n/a                 n/a
 * R4       IEEE 754            4                       -1*2^+127 ..        ~ Value * 2^-24
 *          Single Precision                            2^+127
 * R8       IEEE 754            8                       -1*2^+1023 ..       ~ Value * 2^-53
 *          Double Precision                            2^+1023
 * CH       ASCII / ISO 8859.1  1
 *          Encoding
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef UBLOX6M_DRV_H_
#define UBLOX6M_DRV_H_

#include <stdint.h>


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* GPS module name */
#define UBLOX6M_DEV_NAME        "UBLOX6M"

/* Circular error probability, 50% in 2.5 meters radius */
#define UBLOX6M_CEP_METERS      (2.5)

/* GPS NMEA message output rate */
#define UBLOX6M_NMEA_RATE       {                                       \
                                    {UBLOX6M_MSG_NMEA_DTM, 0},          \
                                    {UBLOX6M_MSG_NMEA_GBS, 0},          \
                                    {UBLOX6M_MSG_NMEA_GGA, 1},          \
                                    {UBLOX6M_MSG_NMEA_GLL, 0},          \
                                    /* {UBLOX6M_MSG_NMEA_GPQ, 0}, */    \
                                    {UBLOX6M_MSG_NMEA_GRS, 0},          \
                                    {UBLOX6M_MSG_NMEA_GSA, 0},          \
                                    {UBLOX6M_MSG_NMEA_GST, 0},          \
                                    {UBLOX6M_MSG_NMEA_GSV, 0},          \
                                    {UBLOX6M_MSG_NMEA_RMC, 1},          \
                                    /* {UBLOX6M_MSG_NMEA_THS, 0}, */    \
                                    /* {UBLOX6M_MSG_NMEA_TXT, 0}, */    \
                                    {UBLOX6M_MSG_NMEA_VTG, 0},          \
                                    {UBLOX6M_MSG_NMEA_ZDA, 0}           \
                                }

/* GPS measurement and navigation rate */
#define UBLOX6M_MEAS_GPS_RATE   200         /*
                                             * Measurement Rate, GPS measurements are
                                             * taken every measRate milliseconds.
                                             * Set to 200ms (5Hz) by default.
                                             */

#define UBLOX6M_MEAS_NAV_RATE   1           /*
                                             * Navigation Rate, in number of measurement cycles.
                                             * On u-blox 5 and u-blox 6, this parameter cannot be
                                             * changed, and is always equals 1.
                                             */

#define UBLOX6M_MEAS_TIME_REF   1           /* 0 = UTC time, 1 = GPS time */

/* UBX protocol header ID */
#define UBLOX6M_UBX_HDR_SYNC1   0xB5
#define UBLOX6M_UBX_HDR_SYNC2   0x62


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* Ublox 6M Port ID definition */
typedef enum ublox6m_port_id{
    UBLOX6M_PORT_I2C            = 0x00,
    UBLOX6M_PORT_UART0          = 0x01,
    UBLOX6M_PORT_UART1          = 0x02,
    UBLOX6M_PORT_USB            = 0x03,
    UBLOX6M_PORT_SPI            = 0x04,
    UBLOX6M_PORT_RESERVED       = 0x05,

    UBLOX6M_PORT_TOTAL,

}__attribute__((packed)) UBLOX6M_PORT_ID;

typedef enum ublox6m_msg_class{
    UBLOX6M_MSG_CLASS_NAV       = 0x01, /* Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used               */
    UBLOX6M_MSG_CLASS_RXM       = 0x02, /* Receiver Manager Messages: Satellite Status, RTC Status                              */
    UBLOX6M_MSG_CLASS_INF       = 0x04, /* Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice */
    UBLOX6M_MSG_CLASS_ACK       = 0x05, /* Ack/Nack Messages: as replies to CFG Input Messages                                  */
    UBLOX6M_MSG_CLASS_CFG       = 0x06, /* Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc.   */
    UBLOX6M_MSG_CLASS_MON       = 0x0A, /* Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status         */
    UBLOX6M_MSG_CLASS_AID       = 0x0B, /* AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input                */
    UBLOX6M_MSG_CLASS_TIM       = 0x0D, /* Timing Messages: Timepulse Output, Timemark Results                                  */
    UBLOX6M_MSG_CLASS_ESF       = 0x10, /* External Sensor Fusion Messages: External sensor measurements and status information */
    UBLOX6M_MSG_CLASS_NEMA_STD  = 0xF0, /* Standard NMEA messages */
    UBLOX6M_MSG_CLASS_NEMA_UBX  = 0xF1, /* UBX NMEA Proprietary Messages */
}__attribute__((packed)) UBLOX6M_MSG_CLASS;

typedef enum ublox6m_msg_id{

    /* UBX Class ACK = 0x05, Ack/Nack Messages */
    UBLOX6M_MSG_ACK_ACK         = 0x01, /* Message Acknowledged                                         */
    UBLOX6M_MSG_ACK_NAK         = 0x00, /* Message Not_Acknowledged                                     */

    /* UBX Class AID = 0x0B, AssistNow Aiding Messages */
    UBLOX6M_MSG_AID_ALM         = 0x30, /* Poll GPS Aiding Almanac Data                                 */
    UBLOX6M_MSG_AID_ALPSRV      = 0x32, /* ALP client requests AlmanacPlus data from server             */
    UBLOX6M_MSG_AID_ALP         = 0x50, /* ALP file data transfer to the receiver                       */
    UBLOX6M_MSG_AID_AOP         = 0x33, /* Poll AssistNow Autonomous data                               */
    UBLOX6M_MSG_AID_DATA        = 0x10, /* Polls all GPS Initial Aiding Data                            */
    UBLOX6M_MSG_AID_EPH         = 0x31, /* Poll GPS Aiding Ephemeris Data                               */
    UBLOX6M_MSG_AID_HUI         = 0x02, /* Poll GPS Health, UTC and ionosphere parameters               */
    UBLOX6M_MSG_AID_INI         = 0x01, /* Poll GPS Initial Aiding Data                                 */
    UBLOX6M_MSG_AID_REQ         = 0x00, /* Sends a poll (AID_DATA) for all GPS Aiding Data              */

    /* UBX Class CFG = 0x06, Configuration Input Messages */
    UBLOX6M_MSG_CFG_ANT         = 0x13, /* Antenna Control Settings                                     */
    UBLOX6M_MSG_CFG_CFG         = 0x09, /* Clear, Save and Load configurations                          */
    UBLOX6M_MSG_CFG_DAT         = 0x06, /* Datum Setting                                                */
    UBLOX6M_MSG_CFG_EKF         = 0x12, /* EKF Module Settings                                          */
    UBLOX6M_MSG_CFG_ESFGWT      = 0x29, /* Get/Set settings of gyro+wheel tick sol (GWT) _ LEA_6R       */
    UBLOX6M_MSG_CFG_FXN         = 0x0E, /* FXN configuration                                            */
    UBLOX6M_MSG_CFG_INF         = 0x02, /* INF message configuration for one protocol                   */
    UBLOX6M_MSG_CFG_ITFM        = 0x39, /* 8 Command Jamming/Interference Monitor configuration.        */
    UBLOX6M_MSG_CFG_MSG         = 0x01, /* message configuration                                        */
    UBLOX6M_MSG_CFG_NAV5        = 0x24, /* Navigation Engine Settings                                   */
    UBLOX6M_MSG_CFG_NAVX5       = 0x23, /* Navigation Engine Expert Settings                            */
    UBLOX6M_MSG_CFG_NMEA        = 0x17, /* NMEA protocol configuration                                  */
    UBLOX6M_MSG_CFG_NVS         = 0x22, /* Clear, Save and Load non_volatile storage data               */
    UBLOX6M_MSG_CFG_PM2         = 0x3B, /* extended Power Management configuration                      */
    UBLOX6M_MSG_CFG_PM          = 0x32, /* Power Management configuration                               */
    UBLOX6M_MSG_CFG_PRT         = 0x00, /* configuration of the used I/O Port                           */
    UBLOX6M_MSG_CFG_RATE        = 0x08, /* Navigation/Measurement Rate Settings                         */
    UBLOX6M_MSG_CFG_RINV        = 0x34, /* Set/Get contents of Remote Inventory                         */
    UBLOX6M_MSG_CFG_RST         = 0x04, /* Reset Receiver / Clear Backup Data Structures                */
    UBLOX6M_MSG_CFG_RXM         = 0x11, /* RXM configuration                                            */
    UBLOX6M_MSG_CFG_SBAS        = 0x16, /* contents of SBAS Configuration                               */
    UBLOX6M_MSG_CFG_TMODE2      = 0x3D, /* Time Mode Settings 2                                         */
    UBLOX6M_MSG_CFG_TMODE       = 0x1D, /* Time Mode Settings                                           */
    UBLOX6M_MSG_CFG_TP5         = 0x31, /* Timepulse Parameters                                         */
    UBLOX6M_MSG_CFG_TP          = 0x07, /* TimePulse Parameters                                         */
    UBLOX6M_MSG_CFG_USB         = 0x1B, /* USB configuration                                            */

    /* UBX Class ESF = 0x10, External Sensor Fusion Messages */
    UBLOX6M_MSG_ESF_MEAS        = 0x02, /* External Sensor Fusion Measurements (LEA_6R)                 */
    UBLOX6M_MSG_ESF_STATUS      = 0x10, /* Periodic/Polled Sensor Fusion Status Information (LEA_6R)    */

    /* UBX Class INF = 0x04, Information Messages */
    UBLOX6M_MSG_INF_DEBUG       = 0x04, /* indicating debug output                                      */
    UBLOX6M_MSG_INF_ERROR       = 0x00, /* indicating an error                                          */
    UBLOX6M_MSG_INF_NOTICE      = 0x02, /* ASCII String output, with informational contents             */
    UBLOX6M_MSG_INF_TEST        = 0x03, /* indicating test output                                       */
    UBLOX6M_MSG_INF_WARNING     = 0x01, /* indicating a warning                                         */

    /* UBX Class MON = 0x0A, Monitoring Messages */
    UBLOX6M_MSG_MON_HW2         = 0x0B, /* Extended Hardware Status                                     */
    UBLOX6M_MSG_MON_HW          = 0x09, /* Hardware Status                                              */
    UBLOX6M_MSG_MON_IO          = 0x02, /* I/O Subsystem Status                                         */
    UBLOX6M_MSG_MON_MSGPP       = 0x06, /* Message Parse and Process Status                             */
    UBLOX6M_MSG_MON_RXBUF       = 0x07, /* Receiver Buffer Status                                       */
    UBLOX6M_MSG_MON_RXR         = 0x21, /* Receiver Status Information                                  */
    UBLOX6M_MSG_MON_TXBUF       = 0x08, /* Transmitter Buffer Status                                    */
    UBLOX6M_MSG_MON_VER         = 0x04, /* Receiver/Software/ROM Version                                */

    /* UBX Class NAV = 0x01, Navigation Results */
    UBLOX6M_MSG_NAV_AOPSTATUS   = 0x60, /* AssistNow Autonomous Status                                  */
    UBLOX6M_MSG_NAV_CLOCK       = 0x22, /* Clock Solution                                               */
    UBLOX6M_MSG_NAV_DGPS        = 0x31, /* DGPS Data Used for NAV                                       */
    UBLOX6M_MSG_NAV_DOP         = 0x04, /* Dilution of precision                                        */
    UBLOX6M_MSG_NAV_EKFSTATUS   = 0x40, /* Dead Reckoning Software Status                               */
    UBLOX6M_MSG_NAV_POSECEF     = 0x01, /* Position Solution in ECEF                                    */
    UBLOX6M_MSG_NAV_POSLLH      = 0x02, /* 2Geodetic Position Solution                                  */
    UBLOX6M_MSG_NAV_SBAS        = 0x32, /* Periodic/Polled SBAS Status Data                             */
    UBLOX6M_MSG_NAV_SOL         = 0x06, /* Navigation Solution Information                              */
    UBLOX6M_MSG_NAV_STATUS      = 0x03, /* Receiver Navigation Status                                   */
    UBLOX6M_MSG_NAV_SVINFO      = 0x30, /* Space Vehicle Information                                    */
    UBLOX6M_MSG_NAV_TIMEGPS     = 0x20, /* GPS Time Solution                                            */
    UBLOX6M_MSG_NAV_TIMEUTC     = 0x21, /* UTC Time Solution                                            */
    UBLOX6M_MSG_NAV_VELECEF     = 0x11, /* Velocity Solution in ECEF                                    */
    UBLOX6M_MSG_NAV_VELNED      = 0x12, /* Velocity Solution in NED                                     */

    /* UBX Class RXM = 0x02, Receiver Manager Messages */
    UBLOX6M_MSG_RXM_ALM         = 0x30, /* GPS Constellation Almanach Data                              */
    UBLOX6M_MSG_RXM_EPH         = 0x31, /* GPS Constellation Ephemeris Data                             */
    UBLOX6M_MSG_RXM_PMREQ       = 0x41, /* Requests a Power Management task                             */
    UBLOX6M_MSG_RXM_RAW         = 0x10, /* Raw Measurement Data                                         */
    UBLOX6M_MSG_RXM_SFRB        = 0x11, /* 42 Periodic Subframe Buffer                                  */
    UBLOX6M_MSG_RXM_SVSI        = 0x20, /* SV Status Info                                               */

    /* UBX Class TIM = 0x0D, Timing Messages */
    UBLOX6M_MSG_TIM_SVIN        = 0x04, /* Survey_in data                                               */
    UBLOX6M_MSG_TIM_TM2         = 0x03, /* Time mark data                                               */
    UBLOX6M_MSG_TIM_TP          = 0x01, /* Timepulse Timedata                                           */
    UBLOX6M_MSG_TIM_VRFY        = 0x06, /* Sourced Time Verification                                    */

    /* NMEA standard Class = 0xF0 */
    UBLOX6M_MSG_NMEA_DTM        = 0x0A, /* Datum Reference                                              */
    UBLOX6M_MSG_NMEA_GBS        = 0x09, /* GNSS Satellite Fault Detection                               */
    UBLOX6M_MSG_NMEA_GGA        = 0x00, /* Global positioning system fix data                           */
    UBLOX6M_MSG_NMEA_GLL        = 0x01, /* Latitude and longitude, with time of position fix and status */
    UBLOX6M_MSG_NMEA_GPQ        = 0x40, /* Poll message                                                 */
    UBLOX6M_MSG_NMEA_GRS        = 0x06, /* GNSS Range Residuals                                         */
    UBLOX6M_MSG_NMEA_GSA        = 0x02, /* GNSS DOP and Active Satellites                               */
    UBLOX6M_MSG_NMEA_GST        = 0x07, /* GNSS Pseudo Range Error Statistics                           */
    UBLOX6M_MSG_NMEA_GSV        = 0x03, /* GNSS Satellites in View                                      */
    UBLOX6M_MSG_NMEA_RMC        = 0x04, /* Recommended Minimum data                                     */
    UBLOX6M_MSG_NMEA_THS        = 0x0E, /* True Heading and Status                                      */
    UBLOX6M_MSG_NMEA_TXT        = 0x41, /* Text Transmission                                            */
    UBLOX6M_MSG_NMEA_VTG        = 0x05, /* Course over ground and Ground speed                          */
    UBLOX6M_MSG_NMEA_ZDA        = 0x08, /* Time and Date                                                */

    /* NMEA UBX Proprietary Class = 0xF1 */
    UBLOX6M_MSG_NMEA_UBX_00     = 0x00, /* Lat/Long Position Data                                       */
    UBLOX6M_MSG_NMEA_UBX_03     = 0x03, /* Satellite Status                                             */
    UBLOX6M_MSG_NMEA_UBX_04     = 0x04, /* Time of Day and Clock Information                            */
    UBLOX6M_MSG_NMEA_UBX_05     = 0x05, /* Lat/Long Position Data                                       */
    UBLOX6M_MSG_NMEA_UBX_06     = 0x06, /* Lat/Long Position Data                                       */
    UBLOX6M_MSG_NMEA_UBX_40     = 0x40, /* Set NMEA message output rate                                 */
    UBLOX6M_MSG_NMEA_UBX_41     = 0x41, /* Set Protocols and Baudrate                                   */

}UBLOX6M_MSG_ID;


/*
 *******************************************************************************
 * UBX protocol frame header and tail definition
 *******************************************************************************
 */
typedef struct ublox6m_msg_hdr{
    uint8_t sync_char1;
    uint8_t sync_char2;
    uint8_t msg_class;
    uint8_t msg_id;
    uint16_t length;
    uint8_t payload[0];     /* Payload */
}__attribute__((packed)) UBLOX6M_MSG_HDR;

typedef struct ublox6m_msg_tail{
    uint16_t chksum;
}__attribute__((packed)) UBLOX6M_MSG_TAIL;


/*
 *******************************************************************************
 * UBX protocol payload definition, ACK and NAK (0x05)
 *******************************************************************************
 */

/* ACK-ACK (0x05 0x01), Message Acknowledged */
typedef struct ublox6m_pl_ack{
    uint8_t ack_msg_class;
    uint8_t ack_msg_id;
}__attribute__((packed)) UBLOX6M_PL_CFG_ACK;

/* ACK-NAK (0x05 0x00), Message Not-Acknowledged */
typedef struct ublox6m_pl_nack{
    uint8_t nak_msg_class;
    uint8_t nak_msg_id;
}__attribute__((packed)) UBLOX6M_PL_CFG_NACK;


/*
 *******************************************************************************
 * UBX protocol payload definition, CFG (0x06)
 *******************************************************************************
 */

/* CFG-MSG (0x06 0x01), Poll a message configuration */
typedef struct ublox6m_pl_cfg_msg_pull{
    uint8_t cfg_msg_class;
    uint8_t cfg_msg_id;
}__attribute__((packed)) UBLOX6M_PL_CFG_MSG_PULL;

/* CFG-MSG (0x06 0x01), Set Message Rate */
typedef struct ublox6m_pl_cfg_msg_access{
    uint8_t cfg_msg_class;
    uint8_t cfg_msg_id;
    uint8_t cfg_msg_rate;
}__attribute__((packed)) UBLOX6M_PL_CFG_MSG_ACCESS;

/* CFG-MSG (0x06 0x01), Set Message Rates */
typedef struct ublox6m_pl_cfg_msgs_access{
    uint8_t cfg_msg_class;
    uint8_t cfg_msg_id;
    uint8_t cfg_msg_rate[UBLOX6M_PORT_TOTAL];
}__attribute__((packed)) UBLOX6M_PL_CFG_MSGS_ACCESS;

/* CFG-RATE (0x06 0x08), oll Navigation/Measurement Rate Settings */
typedef struct ublox6m_pl_cfg_rate_pull{
}__attribute__((packed)) UBLOX6M_PL_CFG_RATE_PULL;

/* CFG-RATE (0x06 0x08), Navigation/Measurement Rate Settings */
typedef struct ublox6m_pl_cfg_rate_access{
    uint16_t measRate;  /* Measurement Rate, milliseconds */
    uint16_t navRate;   /* Navigation Rate, On u-blox 5 and u-blox 6, this parameter is always equals 1. */
    uint16_t timeRef;   /* Alignment to reference time: 0 = UTC time, 1 = GPS time */
}__attribute__((packed)) UBLOX6M_PL_CFG_RATE_ACCESS;


/*
 *******************************************************************************
 * UBX protocol payload definition, MON (0x0A)
 *******************************************************************************
 */

/* MON-VER (0x0A 0x04), pull Receiver/Software/ROM Version */
typedef struct ublox6m_pl_mon_ver_pull{
}__attribute__((packed)) UBLOX6M_PL_MON_VER_PULL;

/* MON-VER (0x0A 0x04), Receiver/Software/ROM Version */
typedef struct ublox6m_pl_mon_ver_access{
    uint8_t sw_version[30];
    uint8_t hw_version[10];
}__attribute__((packed)) UBLOX6M_PL_MON_VER_ACCESS;


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

int8_t ublox6m_Init();


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // UBLOX6M_DRV_H_
