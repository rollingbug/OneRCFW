/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    gps.cpp
 * @brief   GPS navigation functions based on NMEA-0183.
 *
 *          Abbreviations:
 *
 *              2DRMS   - Twice the distance root mean square.
 *              ACCY    - Accuracy.
 *              ALT     - Altitude.
 *              CEP     - Circular Error Probable.
 *              COG     - Course over ground.
 *              COORD   - Coordinate.
 *              DDM     - Degrees Decimal Minutes.
 *              DD      - Decimal Degrees.
 *              DMS     - Deg/Min/Sec.
 *              DIST    - Distance
 *              FAA     - Federal Aviation Administration.
 *              GPS     - Global Positioning System.
 *              H       - Horizontal.
 *              HDOP    - Horizontal Dilution of Precision.
 *              GNSS    - Global Navigation Satellite System.
 *              LAT     - Latitude.
 *              LONG    - Longitude.
 *              NAV     - Navigation.
 *              NMEA    - National Marine Electronics Association.
 *              WGS     - World Geodetic System.
 *              WPT     - Waypoint.
 *              SAT     - Satellite.
 *              R95     - 95% radius.
 *              RMS     - Root mean square.
 *              SEP     - Spherical Error Probable.
 *              V       - Vertical
 *
 *           Reference:
 *
 *              NMEA 0183
 *              https://en.wikipedia.org/wiki/NMEA_0183
 *
 *              World Geodetic System (WGS84)
 *              https://en.wikipedia.org/wiki/World_Geodetic_System
 *
 *              http://www.catb.org/gpsd/NMEA.html#_gll_geographic_position_latitude_longitude
 *
 *              Calculate distance, bearing and more between Latitude/Longitude points.
 *              http://www.movable-type.co.uk/scripts/latlong.html
 *
 *              Circular error probable (CEP, RMS, R95, 2DRMS).
 *              https://en.wikipedia.org/wiki/Circular_error_probable
 *
 *              Question: How to calculate GPS error?
 *              https://forums.adafruit.com/viewtopic.php?f=8&t=97357
 *
 *              GPS Accuracy: Lies, Damn Lies, and Statistics.
 *              http://gpsworld.com/gps-accuracy-lies-damn-lies-and-statistics/
 *
 *           Coordinates:
 *
 *                   N(+)
 *                    |
 *                    |
 *            W(-)---------E(+)
 *                    |
 *                    |
 *                   S(-)
 *
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "gps.h"
#include "timers_drv.h"
#include "math_lib.h"
#include "uart_stream.h"
#include "uart_sim.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* GPS NMEA special keys */
#define GPS_NMEA_CR_KEY                 0x0D    /* Carriage return */
#define GPS_NMEA_LF_KEY                 0x0A    /* Line feed, end delimiter */
#define GPS_NMEA_ENCAP_KEY              0x21    /* Start of encapsulation sentence delimiter */
#define GPS_NMEA_START_KEY              0x24    /* Start delimiter */
#define GPS_NMEA_CHKSUM_KEY             0x2A    /* Checksum delimiter */
#define GPS_NMEA_FIELD_KEY              0x2C    /* Field delimiter */
#define GPS_NMEA_TAG_KEY                0x5C    /* TAG block delimiter */
#define GPS_NMEA_CODE_KEY               0x5E    /* Code delimiter for HEX representation of ISO/IEC 8859-1 (ASCII) characters */
#define GPS_NMEA_RESV_KEY               0x7E    /* Reserved */

#define GPS_NMEA_ADDR_MAX_SIZE          5       /* 5 bytes */

#define GPS_NMEA_CHECKSUM_SIZE          2       /* 2 bytes, HEX ASCII */

/* Macro to check NMEA address content */
#define GPS_NMEA_ADDR_IS_VALID(byte)    \
    ((byte >= '0' && byte <= '9') || (byte >= 'A' && byte <= 'Z'))

/* Macro to accumulate NMEA checksum */
#define GPS_NMEA_ACCUM_CHKSUM(byte, accum_chksum)   \
    (accum_chksum ^= byte)

#define GPS_NMEA_FIELD(type, field)     ((uint8_t)(type | field))
#define GPS_NMEA_GPGGA(field)           ((uint8_t)(GPS_RX_NMEA_TYPE_GGA | field))
#define GPS_NMEA_GPRMC(field)           ((uint8_t)(GPS_RX_NMEA_TYPE_RMC | field))


/*
 *******************************************************************************
 * NMEA related data type definition
 *******************************************************************************
 */

/* index of NMEA GPGGA fields */
typedef enum gps_rx_gga_field{
    GPS_RX_GGA_FIELD_ID             = 0x00, /* Message ID field, GPGGA */
    GPS_RX_GGA_FIELD_UTC            = 0x01, /* UTC Time, Current time */
    GPS_RX_GGA_FIELD_LAT            = 0x02, /* Latitude, Degrees + minutes */
    GPS_RX_GGA_FIELD_NS_IND         = 0x03, /* N/S Indicator, hemisphere N=north or S=south */
    GPS_RX_GGA_FIELD_LONG           = 0x04, /* Longitude, Degrees + minutes */
    GPS_RX_GGA_FIELD_EW_IND         = 0x05, /* E/W indicator, E=east or W=west */
    GPS_RX_GGA_FIELD_FIX_STATUS     = 0x06, /* Position Fix Status Indicator */
    GPS_RX_GGA_FIELD_NO_SV          = 0x07, /* Satellites Used, Range 0 to 12 */
    GPS_RX_GGA_FIELD_HDOP           = 0x08, /* HDOP, Horizontal Dilution of Precision */
    GPS_RX_GGA_FIELD_ALT_VAL        = 0x09, /* MSL Altitude */
    GPS_RX_GGA_FIELD_ALT_UNIT       = 0x0A, /* Units, Meters (fixed field) */
    GPS_RX_GGA_FIELD_ALT_REF        = 0x0B, /* Geoid Separation */
    GPS_RX_GGA_FIELD_USEP_UNIT      = 0x0C, /* Units, Meters (fixed field) */
    GPS_RX_GGA_FIELD_DIFF_AGE       = 0x0D, /* Age of Differential Corrections, Blank (Null) fields when DGPS is not used */
    GPS_RX_GGA_FIELD_DIFF_STATION   = 0x0E, /* Diff. Reference Station ID */
    GPS_RX_GGA_FIELD_CHKSUM         = 0x0F, /* Checksum */
}__attribute__((packed)) GPS_RX_GGA_FIELD;

/* index of NMEA GPRMC fields */
typedef enum gps_rx_rmc_field{
    GPS_RX_RMC_FIELD_ID             = 0x00, /* Message ID field, GPRMC */
    GPS_RX_RMC_FIELD_UTC            = 0x01, /* UTC Time, Current time */
    GPS_RX_RMC_FIELD_NAV_STATUE     = 0x02, /* Status, V = Navigation receiver warning, A = Data valid */
    GPS_RX_RMC_FIELD_LAT            = 0x03, /* Latitude, Degrees + minutes */
    GPS_RX_RMC_FIELD_NS_IND         = 0x04, /* N/S Indicator, hemisphere N=north or S=south */
    GPS_RX_RMC_FIELD_LONG           = 0x05, /* Longitude, Degrees + minutes */
    GPS_RX_RMC_FIELD_EW_IND         = 0x06, /* E/W indicator, E=east or W=west */
    GPS_RX_RMC_FIELD_SPEED          = 0x07, /* Speed over ground, knots */
    GPS_RX_RMC_FIELD_COG            = 0x08, /* Course over ground, degrees */
    GPS_RX_RMC_FIELD_DATE           = 0x09, /* Date in day, month, year format */
    GPS_RX_RMC_FIELD_MAG_VAR        = 0x0A, /* Magnetic variation value, not being output by receiver, degrees */
    GPS_RX_RMC_FIELD_MAG_EW_IND     = 0x0B, /* Magnetic variation E/W indicator, not being output by receiver */
    GPS_RX_RMC_FIELD_FIX_STATUS     = 0x0C, /* Mode Indicator */
    GPS_RX_RMC_FIELD_CHKSUM         = 0x0D, /* Checksum */
}__attribute__((packed)) GPS_RX_RMC_FIELD;


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* RX frame decoding state Definition */
typedef enum gps_rx_nmea_state{
    GPS_RX_NMEA_WAIT_START          = 0,
    GPS_RX_NMEA_WAIT_FIELD,
    GPS_RX_NMEA_WAIT_CHKSUM,
    GPS_RX_NMEA_WAIT_END,
}__attribute__((packed)) GPS_RX_NMEA_STATE;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

static GPS_RX_NMEA_STATE GPS_RxNMEAState;
static uint8_t *p_GPS_RxNMEAField;
static uint8_t GPS_RxNMEAFieldCnt;
static uint8_t GPS_RxNMEABufIdx;
static uint8_t GPS_RxNMEAMsgBuf[96];
static GPS_RX_NMEA_TYPE GPS_RxNMEAType;
static uint8_t GPS_RxNMEAChkSum;
static uint8_t GPS_RxChkSumBuf[GPS_NMEA_CHECKSUM_SIZE + 1];
static uint8_t GPS_RxChkSumBufIdx;
static GPS_NMEA_REPORT GPS_RxNMEAPrivate;

GPS_ERROR_LOG GPS_ErrorLog;


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

static uint8_t GPS_RecvNMEA(uint8_t *p_frm_buf, uint8_t frm_buf_size,
                            GPS_NMEA_REPORT *p_report, uint32_t *p_recv_time,
                            GPS_RX_NMEA_TYPE *p_nmea_type);
static int8_t GPS_DecodeNMEA_Filed(uint8_t *p_field_start, uint8_t nmea_type,
                                   uint8_t field_idx, GPS_NMEA_REPORT *p_report);
static int8_t GPS_UpdateWptRelativeBearing(GPS_DATA *p_gps_data, float *p_bearing);
static float GPS_DM_TO_DD(float dm_val);
static uint32_t GPS_FastHextoul(uint8_t *p_hex_str);
static int32_t GPS_FastStrtoi(uint8_t *p_str, uint8_t **p_end);
static float GPS_FastStrtof(uint8_t *p_str, uint8_t **p_end);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * GPS_Init - Function to initialize GPS hardware component and related functions.
 *
 * @param   [in/out]    *p_nav_data     The core data structure for storing latest
 *                                      GPS status and navigation information.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_Init(GPS_DATA *p_gps_data)
{
    int8_t ret_val;
    const char *p_ret_msg[] = {"OK", "Fail"};

    if(p_gps_data == NULL)
        return -1;

    p_gps_data->general.gga_det_cnt = 0;
    p_gps_data->general.gga_invalid_cnt = 0;
    p_gps_data->general.gga_timestamp = 0;
    p_gps_data->general.rmc_det_cnt = 0;
    p_gps_data->general.rmc_invalid_cnt = 0;
    p_gps_data->general.rmc_timestamp = 0;
    p_gps_data->general.uknown_det_cnt = 0;
    p_gps_data->general.uknown_timestamp = 0;

    p_gps_data->nmea.gpgga.UTC = 0.0;
    p_gps_data->nmea.gpgga.coord.LAT_DD = 0.0;
    p_gps_data->nmea.gpgga.coord.LONG_DD = 0.0;
    p_gps_data->nmea.gpgga.fix_status = 0;
    p_gps_data->nmea.gpgga.SAT_Used = 0;
    p_gps_data->nmea.gpgga.HDOP = 0.0;
    p_gps_data->nmea.gpgga.ALT_meters = 0.0;

    p_gps_data->nmea.gprmc.nav_status = 0;
    p_gps_data->nmea.gprmc.gnd_speed_MS = 0.0;
    p_gps_data->nmea.gprmc.COG_degrees = 0.0;
    p_gps_data->nmea.gprmc.date = 0;
    p_gps_data->nmea.gprmc.fix_status = 0;

    p_gps_data->wpt.is_set = false;
    p_gps_data->wpt.is_valid = false;
    p_gps_data->wpt.discard_cnt = 0;
    p_gps_data->wpt.update_cycle = 0;
    p_gps_data->wpt.coord.LAT_DD = 0.0;
    p_gps_data->wpt.coord.LONG_DD = 0.0;
    p_gps_data->wpt.bearing_angle = 0.0;
    p_gps_data->wpt.distance = 0.0;

    p_gps_data->nav.is_position_set = false;
    p_gps_data->nav.current_coord.LAT_DD = 0.0;
    p_gps_data->nav.current_coord.LONG_DD = 0.0;
    p_gps_data->nav.is_valid = false;
    p_gps_data->nav.relative_bearing_angle = 0.0;

    GPS_ErrorLog.nmea_field_err_cnt = 0;
    GPS_ErrorLog.nmea_chksum_err_cnt = 0;
    GPS_ErrorLog.nmea_end_err_cnt = 0;
    GPS_ErrorLog.rx_timeout_cnt = 0;

    /* Initialize for NMEA RX handler */
    memset((void *)GPS_RxNMEAMsgBuf, 0, sizeof(GPS_RxNMEAMsgBuf));
    memset((void *)GPS_RxChkSumBuf, 0, sizeof(GPS_RxChkSumBuf));
    GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;
    p_GPS_RxNMEAField = NULL;
    GPS_RxNMEAFieldCnt = 0;
    GPS_RxChkSumBufIdx = 0;
    GPS_RxNMEABufIdx = 0;
    GPS_RxNMEAType = GPS_RX_NMEA_TYPE_UNKNOWN;
    GPS_RxNMEAChkSum = 0;

    /* Initialize GPS hardware module */
    ret_val = GPS_MODULE_INIT();

    /* Display initialization result */
    Uart0_Println(PSTR("[GPS] %s"), ret_val ? p_ret_msg[1] : p_ret_msg[0]);
    if(ret_val == 0){
        Uart0_Println(PSTR("[GPS] ACCY METERS: %f"), GPS_MOUDLE_DEFAULT_HACCY_METERS);
    }

    return ret_val;
}

/**
 * GPS_UpdateNMEA - Function to collect NMEA frame byte, decode and update related information.
 *
 * @param   [in/out]            *p_gps_data     Data structure for storing latest GPS information.
 *
 * @return  [GPS_RX_NMEA_TYPE]  Type of received and updated NMEA frame.
 *
 * @retval  [GPS_RX_NMEA_TYPE_GGA]
 * @retval  [GPS_RX_NMEA_TYPE_GLL]
 * @retval  [GPS_RX_NMEA_TYPE_GSA]
 * @retval  [GPS_RX_NMEA_TYPE_GSV]
 * @retval  [GPS_RX_NMEA_TYPE_RMC]
 * @retval  [GPS_RX_NMEA_TYPE_VTG]
 * @retval  [GPS_RX_NMEA_TYPE_GRS]
 * @retval  [GPS_RX_NMEA_TYPE_GST]
 * @retval  [GPS_RX_NMEA_TYPE_ZDA]
 * @retval  [GPS_RX_NMEA_TYPE_GBS]
 * @retval  [GPS_RX_NMEA_TYPE_DTM]
 * @retval  [GPS_RX_NMEA_TYPE_THS]
 * @retval  [GPS_RX_NMEA_TYPE_GPQ]
 * @retval  [GPS_RX_NMEA_TYPE_TXT]
 * @retval  [GPS_RX_NMEA_TYPE_UNKNOWN]
 */
GPS_RX_NMEA_TYPE GPS_UpdateNMEA(GPS_DATA *p_gps_data)
{
    uint8_t nmea_rx_byte;
    uint32_t nmea_timestamp;
    GPS_RX_NMEA_TYPE nmea_type;

    nmea_type = GPS_RX_NMEA_TYPE_UNKNOWN;

    if(p_gps_data == NULL)
        return nmea_type;

    nmea_rx_byte = GPS_RecvNMEA(GPS_RxNMEAMsgBuf, sizeof(GPS_RxNMEAMsgBuf),
                                &GPS_RxNMEAPrivate, &nmea_timestamp, &nmea_type);

    if(nmea_rx_byte){

        /* Received GPGGA message */
        if(nmea_type == GPS_RX_NMEA_TYPE_GGA){
            p_gps_data->general.gga_det_cnt++;
            p_gps_data->general.gga_timestamp = nmea_timestamp;

            if(GPS_RxNMEAPrivate.gpgga.fix_status == 0){
                p_gps_data->general.gga_invalid_cnt++;
            }

            memcpy((void *)&p_gps_data->nmea.gpgga, (void *)&GPS_RxNMEAPrivate.gpgga,
                   sizeof(GPS_RxNMEAPrivate.gpgga));
        }
        /* Received GPRMC message */
        else if(nmea_type == GPS_RX_NMEA_TYPE_RMC){
            p_gps_data->general.rmc_det_cnt++;
            p_gps_data->general.rmc_timestamp = nmea_timestamp;

            if((GPS_RxNMEAPrivate.gprmc.fix_status | GPS_RxNMEAPrivate.gprmc.nav_status) == 0){
                p_gps_data->general.rmc_invalid_cnt++;
            }

            memcpy((void *)&p_gps_data->nmea.gprmc, (void *)&GPS_RxNMEAPrivate.gprmc,
                   sizeof(GPS_RxNMEAPrivate.gprmc));
        }
        /* Received unknown type message */
        else{
            p_gps_data->general.uknown_det_cnt++;
            p_gps_data->general.uknown_timestamp = nmea_timestamp;
        }
    }

    return nmea_type;
}

/**
 * GPS_UpdateNav - Function to update current navigation status.
 *                 (include waypoint distance and bearing)
 *
 * @param   [in/out]    *p_gps_data     Data structure for storing latest GPS information.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_UpdateNav(GPS_DATA *p_gps_data)
{
    GPS_WAYPOINT_DATA *p_waypoint;
    GPS_NMEA_REPORT *p_nmea;
    GPS_NAVIGATION_DATA *p_nav;
    float wpt_bearing;
    float wpt_distance;
    float move_distance;
    float haccy_meters;
    float relative_bearing;
    int8_t nav_result;

    if(p_gps_data == NULL)
        return -1;

    p_nmea = &p_gps_data->nmea;
    p_waypoint = &p_gps_data->wpt;
    p_nav = &p_gps_data->nav;

    if(p_nmea->gpgga.fix_status == 0
       || (GPS_RxNMEAPrivate.gprmc.fix_status | GPS_RxNMEAPrivate.gprmc.nav_status) == 0){
        return -1;
    }

    /* Calculate HDOP area */
    haccy_meters = (float)GPS_MODILE_RUNTIME_HACCY_METERS(p_nmea->gpgga.HDOP);

    /* Update current position */
    if(p_nav->is_position_set == true){
        move_distance = GPS_CalApproxDistance(&p_nav->current_coord, &p_nmea->gpgga.coord);

        /*
         * Only update current position when the distance between
         * old position and new position is larger than HDOP area,
         * and continue the navigation procedure, otherwise quit from
         * this function directly.
         */
        if(move_distance >= haccy_meters){
            p_nav->current_coord.LAT_DD = p_nmea->gpgga.coord.LAT_DD;
            p_nav->current_coord.LONG_DD = p_nmea->gpgga.coord.LONG_DD;
        }
        else{
            return -1;
        }
    }
    /* Initialize current position */
    else{
        p_nav->current_coord.LAT_DD = p_nmea->gpgga.coord.LAT_DD;
        p_nav->current_coord.LONG_DD = p_nmea->gpgga.coord.LONG_DD;
        p_nav->is_position_set = true;

        return -1;
    }

    /*
     * After current position is updated, then update the relative
     * bearing angle between current position and waypoint if the waypoint is set.
     */
    if(p_waypoint->is_set == true){

        wpt_distance = GPS_CalApproxDistance(&p_nmea->gpgga.coord, &p_waypoint->coord);

        /*
         * Only update navigation course when the distance between waypoint
         * and current position is larger than HDOP area.
         */
        if(wpt_distance >= haccy_meters){

            /* Calculate current moving course angle */
            wpt_bearing = GPS_CalInitTrueBearingAngle(&p_nmea->gpgga.coord, &p_waypoint->coord);

            /* Store current position and related information */
            p_waypoint->update_cycle++;
            p_waypoint->bearing_angle = wpt_bearing;
            p_waypoint->distance = wpt_distance;
            p_waypoint->is_valid = true;

            /* Update navigation information */
            nav_result = GPS_UpdateWptRelativeBearing(p_gps_data, &relative_bearing);
            if(nav_result == 0){
                p_nav->is_valid = true;
                p_nav->relative_bearing_angle = relative_bearing;
            }
            else{
                return -1;
            }

        }
        else{
            p_waypoint->discard_cnt++;

            return -1;
        }
    }

    return 0;
}

/**
 * GPS_SetWpt - Function to set the Coordinate of waypoint.
 *
 * @param   [in/out]    *p_gps_data     Data structure for storing latest GPS information.
 * @param   [in]        *p_waypoint     Coordinate of waypoint.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_SetWpt(GPS_DATA *p_gps_data, GPS_COORD_POINT *p_waypoint)
{
    if(p_gps_data == NULL || p_waypoint == NULL)
        return -1;

    p_gps_data->wpt.coord.LAT_DD = p_waypoint->LAT_DD;
    p_gps_data->wpt.coord.LONG_DD = p_waypoint->LONG_DD;
    p_gps_data->wpt.is_set = true;

    return 0;
}

/**
 * GPS_ClrWpt - Function to clear/cancel waypoint.
 *
 * @param   [in/out]    *p_gps_data     Data structure for storing latest GPS information.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_ClrWpt(GPS_DATA *p_gps_data)
{
    if(p_gps_data == NULL)
        return -1;

    p_gps_data->wpt.coord.LAT_DD = 0.0;
    p_gps_data->wpt.coord.LONG_DD = 0.0;
    p_gps_data->wpt.is_set = false;

    return 0;
}

/**
 * GPS_GetWptDistance - Function to get distance between current position and waypoint.
 *
 * @param   [in]        *p_gps_data     Data structure contains latest GPS information.
 *
 * @param   [out]       *p_distance     Distance (meters) between current position and waypoint.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_GetWptDistance(GPS_DATA *p_gps_data, float *p_distance)
{
    if(p_distance == NULL || p_gps_data == NULL)
        return -1;

    if(p_gps_data->wpt.is_set == false || p_gps_data->wpt.is_valid == false)
        return -1;

    *p_distance = p_gps_data->wpt.distance;

    return 0;
}

/**
 * GPS_GetWptTrueBearing - Function to get true bearing of waypoint.
 *
 * @param   [in]        *p_gps_data     Data structure contains latest GPS information.
 *
 * @param   [out]       **p_bearing     True bearing angle of waypoint.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t GPS_GetWptTrueBearing(GPS_DATA *p_gps_data, float *p_bearing)
{
    if(p_bearing == NULL || p_gps_data == NULL)
        return -1;

    if(p_gps_data->wpt.is_set == false || p_gps_data->wpt.is_valid == false)
        return -1;

    *p_bearing = p_gps_data->wpt.bearing_angle;

    return 0;
}

/**
 * GPS_GetWptRelativeBearing - Function to current relative bearing angle between
 *                             measured object and specific waypoint.
 *
 * @param   [in/out]        *p_gps_data     Data structure for storing latest GPS information.
 *
 * @return  [float]         relative bearing angle between moving object and specific waypoint.
 * @retval  [0 ~ 360]       relative bearing angle.
 *
 */
float GPS_GetWptRelativeBearing(GPS_DATA *p_gps_data)
{
    if(p_gps_data == NULL)
        return 0.0;

    if(p_gps_data->nav.is_valid == false)
        return 0.0;

    return p_gps_data->nav.relative_bearing_angle;
}

/**
 * GPS_CalInitTrueBearingAngle - Function to calculate the initial true
 *                               bearing angle from point A to B.
 *
 * Calculate distance, bearing and more between Latitude/Longitude points.
 * http://www.movable-type.co.uk/scripts/latlong.html.
 *
 * Absolute bearing refers to the angle between the magnetic North
 * (magnetic bearing) or true North (true bearing) and an object.
 * https://en.wikipedia.org/wiki/Bearing_(navigation)
 *
 * Formula: BearingRadian = atan2( sin DeltaLong x cos Lat2,
 *                                cos Lat1 x sin Lat2 - sin Lat1 x cos Lat2 x cos DeltaLong )
 *          where Lat1,Long1 is the start point, Lat2,Long2 the end point
 *          (DeltaLong is the difference in longitude)
 *
 * @param   [in]        *p_src      Coordinate of source point.
 * @param   [in]        *p_dest     Coordinate of destination point.
 *
 * @return  [float]     Initial bearing angle.
 * @retval  [0 ~ 360]   Initial bearing angle.
 *
 */
float GPS_CalInitTrueBearingAngle(GPS_COORD_POINT *p_src, GPS_COORD_POINT *p_dest)
{
    float desc_lat_rad;
    float desc_lat_cos;
    float desc_lat_sin;
    float src_lat_rad;
    float src_lat_cos;
    float src_lat_sin;
    float long_delta_rad;
    float x;
    float y;
    float bearing;

    if(p_src == NULL || p_dest == NULL)
        return 0.0;

    long_delta_rad = MATH_DEG2RAD((p_dest->LONG_DD - p_src->LONG_DD));

    desc_lat_rad = MATH_DEG2RAD(p_dest->LAT_DD);
    desc_lat_cos = cos(desc_lat_rad);
    desc_lat_sin = sin(desc_lat_rad);

    src_lat_rad = MATH_DEG2RAD(p_src->LAT_DD);
    src_lat_cos = cos(src_lat_rad);
    src_lat_sin = sin(src_lat_rad);

    y = sin(long_delta_rad) * desc_lat_cos;
    x = src_lat_cos * desc_lat_sin - src_lat_sin * desc_lat_cos * cos(long_delta_rad);

    bearing = MATH_RAD2DEG(atan2(y, x));
    if(bearing < 0.0){
        bearing += 360.0;
    }

    return bearing;
}

/**
 * GPS_CalApproxDistance - Function to calculate the distance between point A and B.
 *
 * Calculate distance, bearing and more between Latitude/Longitude points.
 * http://www.movable-type.co.uk/scripts/latlong.html.
 *
 * @param   [in]        *p_src      Coordinate of source point.
 * @param   [in]        *p_dest     Coordinate of destination point.
 *
 * @return  [float]     Distance in meters.
 * @retval  [0 ~ N]     Distance in meters.
 *
 */
float GPS_CalApproxDistance(GPS_COORD_POINT *p_src, GPS_COORD_POINT *p_dest)
{
    float desc_lat_rad;
    float desc_long_rad;
    float src_lat_rad;
    float src_long_rad;
    float x;
    float y;

    if(p_src == NULL || p_dest == NULL)
        return 0.0;

    desc_lat_rad = MATH_DEG2RAD(p_dest->LAT_DD);
    desc_long_rad = MATH_DEG2RAD(p_dest->LONG_DD);

    src_lat_rad = MATH_DEG2RAD(p_src->LAT_DD);
    src_long_rad = MATH_DEG2RAD(p_src->LONG_DD);

    x = (desc_long_rad - src_long_rad) * cos((desc_lat_rad + src_lat_rad) * 0.5);
    y = (desc_lat_rad - src_lat_rad);

    return Math_FastSqrt((x * x + y * y)) * GPS_EARTH_RADIUS_METERS;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * GPS_SendNMEA - Function example
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [int]   Function executing result.
 * @retval  [0]     Success.
 * @retval  [-1]    Fail.
 *
 */
static uint8_t GPS_SendNMEA()
{
    return 0;
}

/**
 * GPS_RecvNMEA - Function to receive and decode NMEA frame transmitted by GPS module.
 *
 * We will replace ',', '*', '\n' and '\r' with '\0' in the received NMEA message,
 * so that it will be more easy to extract NMEA fields by using string process function.
 *
 * E.g.
 *      Original: $GPGLL,2445.60235,N,2100.91440,E,145225.90,A,A*6A\r\n
 *
 *      Replaced: $GPGLL,2445.60235,N,2100.91440,E,145225.90,A,A*6A\0\0
 *
 * @param   [in/out]    *p_frm_buf      Buffer to store incoming NMEA frame data.
 *                                      This variable should NOT be reset before
 *                                      we received completed NMEA frame.
 *
 * @param   [in]        frm_buf_size    Size of input buffer.
 *
 * @param   [out]       *p_report       Data structure to store NMEA report information.
 *
 * @param   [out]       *p_recv_time    Timestamp when detected NMEA start flag.
 *
 * @param   [out]       *p_nmea_type    Type of received NMEA frame.
 *
 * @return  [uint8_t]   Total received frame size.
 * @retval  [0]         No RX frame.
 * @retval  [1~N]       Byte size of received NMEA frame ($ + Address + Value + Checksum).
 *
 */
static uint8_t GPS_RecvNMEA(uint8_t *p_frm_buf, uint8_t frm_buf_size,
                            GPS_NMEA_REPORT *p_report, uint32_t *p_recv_time,
                            GPS_RX_NMEA_TYPE *p_nmea_type)
{
    uint8_t current_rx_cnt;
    uint8_t total_frm_size;
    uint8_t data_byte;
    uint8_t checksum;
    int8_t decode_result;
    static uint32_t prev_update_time = Timer1_GetMillis();

    current_rx_cnt = 0;
    total_frm_size = 0;

    if(p_frm_buf == NULL || frm_buf_size == 0)
        return 0;

    if(p_report == NULL || p_recv_time == NULL || p_nmea_type == NULL)
        return 0;

    /*
     * Process received byte, but break this loop once we received numbers of
     * frame data in case the keep comping data cause endless loop.
     */
    while(UartS_ReadByte(&data_byte) && current_rx_cnt < frm_buf_size){

        /*
         * Drop all collected frame data if the frame length is larger
         * than size of input buffer.
         */
        if(GPS_RxNMEABufIdx == frm_buf_size)
            GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;

        /* Restart the RX state machine if RX frame timeout has expired. */
        if(Timer1_GetMillis() - prev_update_time > GPS_FRM_TIMEOUT_MS){
            GPS_ErrorLog.rx_timeout_cnt++;
            GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;
        }

        switch(GPS_RxNMEAState){

            /* Detecting NMEA start flag. */
            case GPS_RX_NMEA_WAIT_START:

                GPS_RxNMEABufIdx = 0;

                /* Looking for '$' or '!'. */
                if(data_byte == GPS_NMEA_START_KEY || data_byte == GPS_NMEA_ENCAP_KEY){

                    prev_update_time = Timer1_GetMillis();

                    memset((void *)GPS_RxChkSumBuf, 0, sizeof(GPS_RxChkSumBuf));
                    GPS_RxNMEAFieldCnt = 0;
                    GPS_RxChkSumBufIdx = 0;
                    GPS_RxNMEAChkSum = 0;
                    GPS_RxNMEAType = GPS_RX_NMEA_TYPE_UNKNOWN;

                    p_frm_buf[GPS_RxNMEABufIdx] = data_byte;
                    GPS_RxNMEABufIdx++;

                    p_GPS_RxNMEAField = &p_frm_buf[GPS_RxNMEABufIdx];

                    GPS_RxNMEAState = GPS_RX_NMEA_WAIT_FIELD;
                }

                break;

            /* Collecting fields */
            case GPS_RX_NMEA_WAIT_FIELD:

                p_frm_buf[GPS_RxNMEABufIdx] = data_byte;
                GPS_RxNMEABufIdx++;

                /* ',' */
                if(data_byte == GPS_NMEA_FIELD_KEY){

                    /* accumulate NMEA checksum */
                    GPS_RxNMEAChkSum = GPS_NMEA_ACCUM_CHKSUM(data_byte, GPS_RxNMEAChkSum);

                    p_frm_buf[GPS_RxNMEABufIdx - 1] = '\0';

                    /* Decode filed */
                    if(GPS_RxNMEAFieldCnt == 0){

                        if(strcmp((char *)p_GPS_RxNMEAField, "GPGGA") == 0){

                            GPS_RxNMEAType = GPS_RX_NMEA_TYPE_GGA;
                        }
                        else if(strcmp((char *)p_GPS_RxNMEAField, "GPRMC") == 0){

                            GPS_RxNMEAType = GPS_RX_NMEA_TYPE_RMC;
                        }
                        else{
                            GPS_RxNMEAType = GPS_RX_NMEA_TYPE_UNKNOWN;
                        }
                    }
                    else{
                        decode_result = GPS_DecodeNMEA_Filed(p_GPS_RxNMEAField, GPS_RxNMEAType,
                                                             GPS_RxNMEAFieldCnt, p_report);

                        if(decode_result == -1){

                            GPS_ErrorLog.nmea_field_err_cnt++;

                            GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;
                        }
                    }

                    p_GPS_RxNMEAField = &p_frm_buf[GPS_RxNMEABufIdx];

                    GPS_RxNMEAFieldCnt++;

                }
                /* '*' */
                else if(data_byte == GPS_NMEA_CHKSUM_KEY){

                    GPS_RxNMEAState = GPS_RX_NMEA_WAIT_CHKSUM;

                    /* Decode field */
                    p_frm_buf[GPS_RxNMEABufIdx - 1] = '\0';
                    decode_result = GPS_DecodeNMEA_Filed(p_GPS_RxNMEAField, GPS_RxNMEAType,
                                                         GPS_RxNMEAFieldCnt, p_report);

                    if(decode_result == -1){

                        GPS_ErrorLog.nmea_field_err_cnt++;

                        GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;
                    }
                    else{
                        GPS_RxNMEAState = GPS_RX_NMEA_WAIT_CHKSUM;
                    }
                }
                /* Field data */
                else{
                    /* accumulate NMEA checksum */
                    GPS_RxNMEAChkSum = GPS_NMEA_ACCUM_CHKSUM(data_byte, GPS_RxNMEAChkSum);
                }

                break;

            /* Check NMEA checksum. */
            case GPS_RX_NMEA_WAIT_CHKSUM:

                p_frm_buf[GPS_RxNMEABufIdx] = data_byte;
                GPS_RxNMEABufIdx++;

                GPS_RxChkSumBuf[GPS_RxChkSumBufIdx] = data_byte;
                GPS_RxChkSumBufIdx++;

                /* Collecting checksum (2 bytes HEX ASCII). */
                if(GPS_RxChkSumBufIdx == GPS_NMEA_CHECKSUM_SIZE){

                    checksum = (uint8_t)GPS_FastHextoul(GPS_RxChkSumBuf);

                    /* Checksum is matched */
                    if(checksum == GPS_RxNMEAChkSum){
                        GPS_RxNMEAState = GPS_RX_NMEA_WAIT_END;
                    }
                    /* Incorrect checksum, reset */
                    else{
                        GPS_ErrorLog.nmea_chksum_err_cnt++;
                        GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;
                    }
                }

                break;

            /* Check CR or LF. */
            case GPS_RX_NMEA_WAIT_END:

                /* Make sure the checksum is correct and and last character is LF. */
                if(data_byte == GPS_NMEA_CR_KEY || data_byte == GPS_NMEA_LF_KEY){

                    p_frm_buf[GPS_RxNMEABufIdx] = '\0';
                    GPS_RxNMEABufIdx++;

                    *p_recv_time = Timer1_GetMillis();
                    *p_nmea_type = GPS_RxNMEAType;

                    total_frm_size = GPS_RxNMEABufIdx;
                }
                else{
                    GPS_ErrorLog.nmea_end_err_cnt++;
                }

                GPS_RxNMEAState = GPS_RX_NMEA_WAIT_START;

                break;

            default:
                break;
        }

        if(total_frm_size != 0)
            break;

        current_rx_cnt++;
    }

    return total_frm_size;
}

/**
 * GPS_RecvNMEA - Function to decode NMEA data field.
 *
 * We will replace ',', '*', '\n' and '\r' with '\0' in the received NMEA message,
 * so that it will be more easy to extract NMEA fields by using string process function.
 *
 * E.g.
 *      Original: $GPGLL,2445.60235,N,2100.91440,E,145225.90,A,A*6A\r\n
 *
 *      Replaced: $GPGLL,2445.60235,N,2100.91440,E,145225.90,A,A*6A\0\0
 *
 * @param   [in]        *p_field_start  A pointer point to field start offset.
 *
 *          [in]        nmea_type       Type of inputing NMEA frame.
 *
 *          [in]        field_idx       Index of inputing NMEA frame field.
 *
 * @param   [out]       *p_report       Data structure to store NMEA report information.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t GPS_DecodeNMEA_Filed(uint8_t *p_field_start, uint8_t nmea_type,
                                   uint8_t field_idx, GPS_NMEA_REPORT *p_report)
{
    uint8_t *p_field_end;

    if(p_field_start == NULL || p_report == NULL)
        return -1;

    switch(GPS_NMEA_FIELD(nmea_type, field_idx)){

        /*
         * GPGGA, Global Positioning System Fix Data
         */
        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_UTC):          /* hhmmss.ss */

            p_report->gpgga.UTC = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_LAT):          /* ddmm.mmmm */

            p_report->gpgga.coord.LAT_DD = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            p_report->gpgga.coord.LAT_DD = GPS_DM_TO_DD(p_report->gpgga.coord.LAT_DD);
            if(p_report->gpgga.coord.LAT_DD > 90.0)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_NS_IND):       /* N/S */

            if(strcmp("N", (char *)p_field_start) == 0){
                p_report->gpgga.coord.LAT_DD = p_report->gpgga.coord.LAT_DD;
            }
            else if(strcmp("S", (char *)p_field_start) == 0){
                p_report->gpgga.coord.LAT_DD = -p_report->gpgga.coord.LAT_DD;
            }
            else{
                return -1;
            }

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_LONG):         /* ddmm.mmmm */

            p_report->gpgga.coord.LONG_DD = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            p_report->gpgga.coord.LONG_DD = GPS_DM_TO_DD(p_report->gpgga.coord.LONG_DD);
            if(p_report->gpgga.coord.LONG_DD > 180.0)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_EW_IND):       /* E/W */

            if(strcmp("E", (char *)p_field_start) == 0){
                p_report->gpgga.coord.LONG_DD = p_report->gpgga.coord.LONG_DD;
            }
            else if(strcmp("W", (char *)p_field_start) == 0){
                p_report->gpgga.coord.LONG_DD = -p_report->gpgga.coord.LONG_DD;
            }
            else{
                return -1;
            }

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_FIX_STATUS):   /* 0, 1, 2, 6 */

            p_report->gpgga.fix_status = (uint8_t)GPS_FastStrtoi(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_NO_SV):        /* 0 ~ 12 */

            p_report->gpgga.SAT_Used = (uint8_t)GPS_FastStrtoi(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_HDOP):         /* float */

            p_report->gpgga.HDOP = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_ALT_VAL):      /* float */

            p_report->gpgga.ALT_meters = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPGGA(GPS_RX_GGA_FIELD_ALT_UNIT):     /* 'M', meter */

            if(strcmp("M", (char *)p_field_start) != 0){
                return -1;
            }

            break;

        /*
         * GPRMC, Recommended minimum specific GPS/Transit data
         */
        case GPS_NMEA_GPRMC(GPS_RX_RMC_FIELD_NAV_STATUE):   /* 'V', 'A' */

            /* Warning */
            if(strcmp("V", (char *)p_field_start) == 0){
                p_report->gprmc.nav_status = 0;
            }
            /* Valid */
            else if(strcmp("A", (char *)p_field_start) == 0){
                p_report->gprmc.nav_status = 1;
            }
            /* Unknown */
            else{
                return -1;
            }

            break;

        case GPS_NMEA_GPRMC(GPS_RX_RMC_FIELD_SPEED):        /* float, knots */

            p_report->gprmc.gnd_speed_MS = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            /* knot to m/s */
            p_report->gprmc.gnd_speed_MS = GPS_KNOTS_TO_M_PER_SEC(p_report->gprmc.gnd_speed_MS);

            break;

        case GPS_NMEA_GPRMC(GPS_RX_RMC_FIELD_COG):          /* float, degrees */

            p_report->gprmc.COG_degrees = GPS_FastStrtof(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPRMC(GPS_RX_RMC_FIELD_DATE):         /* ddmmyy */

            p_report->gprmc.date = (uint32_t)GPS_FastStrtoi(p_field_start, &p_field_end);
            if(p_field_end == NULL)
                return -1;

            break;

        case GPS_NMEA_GPRMC(GPS_RX_RMC_FIELD_FIX_STATUS):   /* 'N', 'A', 'D', 'E' */

            /* No Fix */
            if(strcmp("N", (char *)p_field_start) == 0){
                p_report->gprmc.fix_status = 0;
            }
            /* Autonomous GNSS Fix */
            else if(strcmp("A", (char *)p_field_start) == 0){
                p_report->gprmc.fix_status = 1;
            }
            /* Differential GNSS Fix, */
            else if(strcmp("D", (char *)p_field_start) == 0){
                p_report->gprmc.fix_status = 4;
            }
            /* Estimated/Dead Reckoning Fix */
            else if(strcmp("E", (char *)p_field_start) == 0){
                p_report->gprmc.fix_status = 5;
            }
            /* Unknown */
            else{
                return -1;
            }

            break;

        default:
            break;
    }

    return 0;
}

/**
 * GPS_UpdateWaypointRelativeBearing - Function to calculate and update current
 *                                     relative bearing angle between measured
 *                                     object and specific waypoint.
 *
 * @param   [in/out]    *p_gps_data     Data structure for storing latest GPS information.
 * @param   [out]       *p_bearing      Relative bearing angle, -360 ~ 360 degree.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t GPS_UpdateWptRelativeBearing(GPS_DATA *p_gps_data, float *p_bearing)
{
    GPS_WAYPOINT_DATA *p_waypoint;
    GPS_NMEA_REPORT *p_nmea;

    if(p_gps_data == NULL || p_bearing == NULL)
        return -1;

    p_nmea = &p_gps_data->nmea;
    p_waypoint = &p_gps_data->wpt;

    if(p_nmea->gpgga.fix_status == 0
       || (GPS_RxNMEAPrivate.gprmc.fix_status | GPS_RxNMEAPrivate.gprmc.nav_status) == 0){

        return -1;
    }

    if(p_waypoint->is_set == false || p_waypoint->is_valid == false)
        return -1;

    *p_bearing = p_waypoint->bearing_angle - p_nmea->gprmc.COG_degrees;

    return 0;
}

/**
 * GPS_DM_TO_DD - Function to degrees/minutes(DM) coordinate
 *                to decimal degrees(DD) format.
 *
 * DDMM.MMMM -> DD.DDDDDD
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [float]         Converted data
 * @retval  [DD.DDDDDD]     Degrees.
 *
 */
static float GPS_DM_TO_DD(float dm_val)
{
    int degree;

    degree = (int)(dm_val * 0.01);
    dm_val = (dm_val - degree * 100) * (1.0 / 60.0);

    return degree + dm_val;
}

/**
 * GPS_FastHextoul - Function to convert HEX ASCII string to unsigned integer.
 *
 * This function can only be used for decoding NMEA message!
 *
 * @param   [in]        *p_hex_str          Input string.
 *
 * @return  [uint32_t]  Converted number.
 * @retval  [0~N]
 *
 */
static uint32_t GPS_FastHextoul(uint8_t *p_hex_str)
{
    uint32_t value;
    uint8_t ch;

    value = 0;

    if(p_hex_str == NULL)
        return value;

    for( ;(ch = *p_hex_str); p_hex_str++){
        if (ch >= '0' && ch <= '9')
            ch = ch - '0';
        else if (ch >= 'a' && ch <='f')
            ch = ch - 'a' + 10;
        else if (ch >= 'A' && ch <='F')
            ch = ch - 'A' + 10;

        value = (value << 4) | (ch & 0xF);
    }

    return value;
}

/**
 * GPS_FastStrtoi - Function to convert string to float number.
 *
 * This function can only be used for decoding NMEA message!
 *
 * @param   [in]    *p_str          Input string.
 * @param   [out]   **p_end         Value is set by this function to the next character
 *                                  in str after the numerical value.
 *                                  This parameter can also be a null pointer.
 *
 * @return  [float]         Converted float number.
 * @retval  [0~N]           Return 0.0 by default if no valid conversion.
 *
 */
static int32_t GPS_FastStrtoi(uint8_t *p_str, uint8_t **p_end)
{
    uint8_t ch;
    int32_t tmp1;
    bool is_negtive;

    tmp1 = 0;
    is_negtive = false;

    if(p_end != NULL)
        *p_end = NULL;

    if(p_str == NULL)
        return 0;

    if(*p_str == '\0')
        return 0;

    if(*p_str == '-'){
        is_negtive = true;
        p_str++;
    }

    /* Collect integer part characters and convert to int format */
    for( ;(ch = *p_str); p_str++){

        if(ch < '0' || ch > '9')
            break;

        tmp1 = ((tmp1 << 3) + (tmp1 << 1)) + (ch - '0');
    }

    /* Fail */
    if(*p_str != '\0'){
        return 0;
    }
    /* Success */
    else{
        if(p_end != NULL)
            *p_end = p_str;

        if(is_negtive)
            tmp1 = -tmp1;

        return tmp1;
    }
}

/**
 * GPS_FastStrtof - Function to convert string to float number.
 *
 * This function can only be used for decoding NMEA message!
 *
 * @param   [in]    *p_str          Input string.
 * @param   [out]   **p_end         Value is set by this function to the next character
 *                                  in str after the numerical value.
 *                                  This parameter can also be a null pointer.
 *
 * @return  [float]         Converted float number.
 * @retval  [0~N]           Return 0.0 by default if no valid conversion.
 *
 */
static float GPS_FastStrtof(uint8_t *p_str, uint8_t **p_end)
{
    static const float lookup[10] =
    {
        0.0,        0.1,        0.01,       0.001,      0.0001,
        0.00001,    0.000001,   0.0000001,  0.00000001, 0.000000001,
    };
    uint8_t digits;
    uint8_t ch;
    int32_t tmp1;
    int32_t tmp2;
    float value;
    bool is_negtive;

    tmp1 = 0;
    tmp2 = 0;
    digits = 0;
    value = 0.0;
    is_negtive = false;

    if(p_end != NULL)
        *p_end = NULL;

    if(p_str == NULL)
        return 0.0;

    if(*p_str == '\0')
        return 0.0;

    if(*p_str == '-'){
        is_negtive = true;
        p_str++;
    }

    /* Collect integer part characters and convert to int format */
    for( ;(ch = *p_str); p_str++){

        if(ch < '0' || ch > '9')
            break;

        tmp1 = ((tmp1 << 3) + (tmp1 << 1)) + (ch - '0');
    }

    /* Collect non-integer part characters and convert to int format */
    if(*p_str == '.'){
        p_str++;

        for( ;(ch = *p_str); p_str++){

            if(ch < '0' || ch > '9')
                break;

            tmp2 = ((tmp2 << 3) + (tmp2 << 1)) + (ch - '0');

            digits++;
        }
    }

    /* Fail */
    if(*p_str != '\0' || digits >= 10){
        return 0.0;
    }

    if(is_negtive){
        value = (-tmp1 - tmp2 * lookup[digits]);
    }
    else{
        value = (tmp1 + tmp2 * lookup[digits]);
    }

    if(isnan(value) || isinf(value)){
        return 0.0;
    }
    else{
        if(p_end != NULL)
            *p_end = p_str;

        return value;
    }
}
