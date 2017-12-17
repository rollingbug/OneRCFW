/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    ahrs.h
 * @brief   AHRS (attitude and heading reference system)
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef AHRS_H_
#define AHRS_H_

#include <stdint.h>
#include "imu_ctrl.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* General */
#define AHRS_AXES           IMU_AXES
#define AHRS_X              IMU_X
#define AHRS_Y              IMU_Y
#define AHRS_Z              IMU_Z


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

typedef struct ahrs_accel_data{
    int32_t fxp_vctr[AHRS_AXES];
    int16_t sensor_data[AHRS_AXES];
    int32_t G_SQ_FXP;
}AHRS_ACCEL_DATA;

typedef struct ahrs_ned_attitude{
    float roll_angle;
    float pitch_angle;
    float heading_angle;
}AHRS_NED_ATTITUDE;

typedef struct ahrs_body_attitude{
    float roll_angle;
    float pitch_angle;
    float yaw_angle;
}AHRS_BODY_ATTITUDE;

typedef struct ahrs_data{

    /* General */
    uint16_t delta_time;

    /* Accelerometer */
    AHRS_ACCEL_DATA accel;
    uint8_t accel_exceed_cnt;

    /* Gyroscope */
    int16_t gyro_sensor_data[AHRS_AXES];
    int32_t gyro_fxp_rads[AHRS_AXES];

    /* Level vector */
    int32_t level_fxp_vctr[AHRS_AXES];

    /* Heading vector */
    int32_t heading_fxp_vctr[AHRS_AXES];

    /* NED Attitude */
    AHRS_NED_ATTITUDE ned_att;
    AHRS_BODY_ATTITUDE body_att;

}AHRS_DATA;


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

int8_t AHRS_Init(AHRS_DATA *p_ahrs, int16_t *p_accel_raw);
int8_t AHRS_AttAngleUpdate(int16_t *p_accel_raw, int16_t *p_gyro_raw,
                           uint16_t delta_micros, AHRS_DATA *p_ahrs);

#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG
void AHRS_SetSimAngle(float roll_angle, float pitch_angle, float yaw_angle);
#endif


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // AHRS_H_
