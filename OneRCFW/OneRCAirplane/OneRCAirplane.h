/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    OneRCAirplane.h
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef ONERCAIRPLANE_H_
#define ONERCAIRPLANE_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define AIRPLANE_CONFIG_ID              0x315f4150  /* "1_AP" */

#define AIRPLANE_FW_DATE                0x20171216

#define AIRPLANE_DBG_PIN_HIGH()         do{                         \
                                            PORTC |= _BV(PORTC0);   \
                                        }while(0)

#define AIRPLANE_DBG_PIN_LOW()          do{                         \
                                            PORTC &= ~_BV(PORTC0);  \
                                        }while(0)

#define AIRPLANE_CFG_ROM_ADDR           0x0

#define AIRPLANE_CTRL_LOOP_PERIOD       5000    /* 5000 us = 5.0 ms */
#define AIRPLANE_CTRL_LOOP_DELAY_THR    5500    /* 5500 us = 5.5 ms */

#define AIRPLANE_CHK_CFG_MODE_TIMEOUT   1000    /* 1000 ms = 1 second */

#define AIRPLANE_RC_CALI_SMOOTH_PERIOD  10      /* 10 ms */
#define AIRPLANE_RC_CALI_SMOOTH_CNT     300     /* 300 times */

#define AIRPLANE_GPS_HOME_SAMPLE_CNT    64      /* Home position sample count */

/* Maximum pitch compensation angle limitation +- N */
#define AIRPLANE_BANK_TURN_MAX_PITCH    5.0     /* Degree */

/*
 * Gain setting for mapping roll [180 .. 0 .. -180] to pitch [N .. 0 .. -N] degree
 * Say pitch_3_degree / (1.0 - cos(roll_30_degree)) = 22.3923048447,
 * means we will set pitch angle to 3.0 degree when roll angle is equal to +- 30 degree.
 */
#define AIRPLANE_BANK_TURN_PITCH_GAIN   22.3923048447

/* Airplane status snapshot function */
#define AIRPLANE_STATUS_SNAPSHOT_EN     false

/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

typedef enum airplane_cruise_state{
    AIRPLANE_CRUISE_FORWARDTO_WPT               = 0,
    AIRPLANE_CRUISE_AWAYFROM_WPT,
}__attribute__((packed)) AIRPLANE_CRUISE_STATE;

typedef enum airplane_nav_state{
    AIRPLANE_NAV_WAIT_NMEA_FRM                  = 0,
    AIRPLANE_NAV_UPDATE_MOVING_BEARING,
    AIRPLANE_NAV_UPDATE_WAYPOINT_BEARING,
    AIRPLANE_NAV_UPDATE_RELATIVE_BEARING,
}__attribute__((packed)) AIRPLANE_NAV_STATE;

typedef enum airplane_type{
    AIRPLANE_NORMAL                             = 0,
    AIRPLANE_DELTA,
    AIRPLANE_VTAIL,
}__attribute__((packed)) AIRPLANE_TYPE;

typedef enum airplane_fly_mode{
    AIRPLANE_MANUAL_FLY                         = 0,
    AIRPLANE_SELF_STABILIZE,
    AIRPLANE_RETURN_TO_HOME,
}__attribute__((packed)) AIRPLANE_FLY_MODE;

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

void LED_Blink(uint32_t on_millis, uint32_t off_millis);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // ONERCAIRPLANE_H_
