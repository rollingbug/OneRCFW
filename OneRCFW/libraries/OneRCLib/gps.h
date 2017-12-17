/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    gps.h
 * @brief   GPS navigation functions based on NMEA-0183.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef GPS_H_
#define GPS_H_

#include <stdint.h>


/*
 *******************************************************************************
 * GPS module configuration
 *******************************************************************************
 */

/* GPS module type definition */
#define GPS_MODULE_UBLOX6M

/* Get GPS information from UBLOX NEO-6M module. */
#if defined(GPS_MODULE_UBLOX6M)
    #include "ublox6m_drv.h"
    #define GPS_MODULE_NAME                     UBLOX6M_DEV_NAME
    #define GPS_MODULE_INIT()                   ublox6m_Init()
    #define GPS_MODULE_CEP_METERS               UBLOX6M_CEP_METERS

/* Get GPS information from FlightGear FDM. */
#elif defined(GPS_MODULE_FG)
    #define GPS_MODULE_NAME                     "UNKNOWN"
    #define GPS_MODULE_INIT()                   (0)
    #define GPS_MODULE_CEP_METERS               (2.5)

#else
    #error "Incorrect GPS module setting"

#endif


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Circular error probable conversion */
#define GPS_MODULE_RMS_METERS                   (1.2 * GPS_MODULE_CEP_METERS)   /* 63 ~ 68 % */
#define GPS_MODULE_R95_METERS                   (2.1 * GPS_MODULE_CEP_METERS)   /* 95% */
#define GPS_MODULE_2DRRMS_METERS                (2.4 * GPS_MODULE_CEP_METERS)   /* 95 ~ 98% */

#define GPS_MOUDLE_DEFAULT_HACCY_METERS         (GPS_MODULE_2DRRMS_METERS)
#define GPS_MODILE_RUNTIME_HACCY_METERS(hdop)   (hdop * GPS_MODULE_2DRRMS_METERS)

/* Earth radius, about 6371 km  = 6371000 meters */
#define GPS_EARTH_RADIUS_METERS                 (6371000)

#define GPS_KNOTS_TO_M_PER_SEC(knots)           ((knots) * 0.5144444)

#define GPS_FRM_TIMEOUT_MS                      2000


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* RX frame decoding state Definition */
typedef enum gps_rx_nmea_type{
    GPS_RX_NMEA_TYPE_GGA        = 0x00, /* Global positioning system fix data                           */
    GPS_RX_NMEA_TYPE_GLL        = 0x10, /* Latitude and longitude, with time of position fix and status */
    GPS_RX_NMEA_TYPE_GSA        = 0x20, /* GNSS DOP and Active Satellites                               */
    GPS_RX_NMEA_TYPE_GSV        = 0x30, /* GNSS Satellites in View                                      */
    GPS_RX_NMEA_TYPE_RMC        = 0x40, /* Recommended Minimum data                                     */
    GPS_RX_NMEA_TYPE_VTG        = 0x50, /* Course over ground and Ground speed                          */
    GPS_RX_NMEA_TYPE_GRS        = 0x60, /* GNSS Range Residuals                                         */
    GPS_RX_NMEA_TYPE_GST        = 0x70, /* GNSS Pseudo Range Error Statistics                           */
    GPS_RX_NMEA_TYPE_ZDA        = 0x80, /* Time and Date                                                */
    GPS_RX_NMEA_TYPE_GBS        = 0x90, /* GNSS Satellite Fault Detection                               */
    GPS_RX_NMEA_TYPE_DTM        = 0xA0, /* Datum Reference                                              */
    GPS_RX_NMEA_TYPE_THS        = 0xB0, /* True Heading and Status                                      */
    GPS_RX_NMEA_TYPE_GPQ        = 0xC0, /* Poll message                                                 */
    GPS_RX_NMEA_TYPE_TXT        = 0xD0, /* Text Transmission                                            */
    GPS_RX_NMEA_TYPE_UNKNOWN    = 0xE0, /* Unknown                                                      */
}__attribute__((packed)) GPS_RX_NMEA_TYPE;

/* Coordinate data structure */
typedef struct gps_coord_point{
    float LAT_DD;                       /* Latitude in decimal degrees format. */
    float LONG_DD;                      /* Longitude in decimal degrees format. */
}GPS_COORD_POINT;

/* Data structure to store NMEA report information */
typedef struct gps_nmea_report{
    struct{
        float UTC;                      /* UTC, hhmmdd.sss. */
        GPS_COORD_POINT coord;          /* Current coordinate in decimal degrees format. */
        uint8_t fix_status;             /* fix status. */
        uint8_t SAT_Used;               /* total using satellite. */
        float HDOP;                     /* Horizontal Dilution of Precision. */
        float ALT_meters;               /* Altitude in meters. */
    }gpgga;

    struct{
        uint8_t nav_status;             /* Navigation status. */
        float gnd_speed_MS;             /* Ground speed, meter/second. */
        float COG_degrees;              /* Course over ground, 0~ 360 degree. */
        uint32_t date;                  /* Current data, YYMMDD. */
        uint8_t fix_status;             /* fix status, NMEA version 2.3 only. */
    }gprmc;

}GPS_NMEA_REPORT;

/* Waypoint information data structure */
typedef struct gps_waypoint_data{

    bool is_set;                        /* Indicator to show whether waypoint is set or not */
    bool is_valid;                      /* Indicator to show whether current waypoint information is valid or not */
    uint8_t discard_cnt;                /* Total discarded NMEA frame (based on CEP check) */
    uint8_t update_cycle;               /* Update cycle counter */
    GPS_COORD_POINT coord;              /* Waypoint coordinate */
    float bearing_angle;                /* True bearing angle between current position and waypoint */
    float distance;                     /* Distance between current position and waypoint */

}GPS_WAYPOINT_DATA;

/* Navigation information data structure */
typedef struct gps_navigation_data{

    bool is_position_set;               /* Flag to indicate whether the current position is initialized or not */
    GPS_COORD_POINT current_coord;      /* Current position of vehicle. */

    bool is_valid;                      /* Indicator to show whether current navigation information is valid or not */
    float relative_bearing_angle;       /* Relative bearing angle between current moving direction and waypoint */

}GPS_NAVIGATION_DATA;

/* GPS data */
typedef struct gps_data{

    struct{

        uint8_t gga_det_cnt;            /* NMEA GPGGA frame detected count */
        uint8_t gga_invalid_cnt;        /* NMEA GPGGA frame invalid count */
        uint32_t gga_timestamp;         /* NMEA GPGGA frame RX timestamp */

        uint8_t rmc_det_cnt;            /* NMEA GPRMC frame detected count */
        uint8_t rmc_invalid_cnt;        /* NMEA GPRMC frame invalid count */
        uint32_t rmc_timestamp;         /* NMEA GPRMC frame RX timestamp */

        uint8_t uknown_det_cnt;         /* NMEA unknown frame detected count */
        uint32_t uknown_timestamp;      /* NMEA unknown frame RX timestamp */

    }general;

    GPS_NMEA_REPORT nmea;

    GPS_WAYPOINT_DATA wpt;

    GPS_NAVIGATION_DATA nav;

}GPS_DATA;

/* GPS RX error log */
typedef struct gps_error_log{
    uint8_t nmea_field_err_cnt;
    uint8_t nmea_chksum_err_cnt;
    uint8_t nmea_end_err_cnt;
    uint8_t rx_timeout_cnt;
}GPS_ERROR_LOG;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

extern GPS_ERROR_LOG GPS_ErrorLog;


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

int8_t GPS_Init(GPS_DATA *p_gps_data);
GPS_RX_NMEA_TYPE GPS_UpdateNMEA(GPS_DATA *p_gps_data);
int8_t GPS_UpdateNav(GPS_DATA *p_gps_data);

int8_t GPS_SetWpt(GPS_DATA *p_gps_data, GPS_COORD_POINT *p_waypoint);
int8_t GPS_ClrWpt(GPS_DATA *p_gps_data);
int8_t GPS_GetWptDistance(GPS_DATA *p_gps_data, float *p_distance);
int8_t GPS_GetWptTrueBearing(GPS_DATA *p_gps_data, float *p_bearing);
float GPS_GetWptRelativeBearing(GPS_DATA *p_gps_data);

float GPS_CalInitTrueBearingAngle(GPS_COORD_POINT *p_src, GPS_COORD_POINT *p_dest);
float GPS_CalApproxDistance(GPS_COORD_POINT *p_src, GPS_COORD_POINT *p_dest);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // GPS_H_
