/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    imu_ctrl.h
 * @brief   IMU (inertial measurement unit)
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef IMU_CTRL_H_
#define IMU_CTRL_H_

#include <stdint.h>


/*
 *******************************************************************************
 * IMU sensors configuration
 *******************************************************************************
 */

/* IMU sensor hardware type definition */
#define IMU_SENSOR_MPU6050

/* General */
#define IMU_AXES    3
#define IMU_X       0
#define IMU_Y       1
#define IMU_Z       2


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Get IMU information from MPU6050 real sensor. */
#if defined(IMU_SENSOR_MPU6050)
    #include "mpu6050_drv.h"
    #define IMU_SENSOR_NAME                                     "MPU6050"
    #define IMU_SENSOR_UNIT_1G                                  MPU6050_UNIT_1G
    #define IMU_SENSOR_UNIT_1DPS                                MPU6050_UNIT_1DPS
    #define IMU_SENSOR_CAL_MODE                                 IMU_SENSOR_CAL_RUNTIME
    #define IMU_SENSOR_CAL_ACCEL_DEF                            {113, -63, -59}
    #define IMU_SENSOR_CAL_GYRO_DEF                             {-27, 11, 9}
    #define IMU_SENSOR_INIT()                                   mpu6050_Init()
    #define IMU_SENSOR_GET_6_RAW_DATA(p_accel, p_gyro)          mpu6050_ReadXYZDirectly(p_accel, p_gyro)

/* Get IMU information from PC off-line data. */
#elif defined(IMU_SENSOR_PC_SIM)
    #define IMU_SENSOR_PC_EN                                    true
    #define IMU_SENSOR_NAME                                     "IMU_PC_SIM"
    #define IMU_SENSOR_UNIT_1G                                  (4096)
    #define IMU_SENSOR_UNIT_1DPS                                (16.4)
    #define IMU_SENSOR_CAL_MODE                                 IMU_SENSOR_CAL_NONE
    #define IMU_SENSOR_CAL_ACCEL_DEF                            {0, 0, 0}
    #define IMU_SENSOR_CAL_GYRO_DEF                             {0, 0, 0}
    #define IMU_SENSOR_INIT()                                   IMU_SIM_Init()
    #define IMU_SENSOR_GET_6_RAW_DATA(p_accel, p_gyro)          IMU_SIM_Get6RawData(p_accel, p_gyro)
    #define IMU_SENSOR_SAMPLE_TIME                              IMU_SIM_SampleMicros

/* Get IMU information from FlightGear FDM. */
#elif defined(IMU_SENSOR_FG_SIM)
    #define IMU_SENSOR_FG_EN                                    true
    #define IMU_SENSOR_NAME                                     "IMU_FG_SIM"
    #define IMU_SENSOR_ANGLE_FROM_FG                            true    /* Get NED angle from FG directly */
    #define IMU_SENSOR_UNIT_1G                                  (4096)
    #define IMU_SENSOR_UNIT_1DPS                                (16.4)
    #define IMU_SENSOR_CAL_MODE                                 IMU_SENSOR_CAL_NONE
    #define IMU_SENSOR_CAL_ACCEL_DEF                            {0, 0, 0}
    #define IMU_SENSOR_CAL_GYRO_DEF                             {0, 0, 0}
    #define IMU_SENSOR_INIT()                                   IMU_FG_Init()
    #define IMU_SENSOR_GET_6_RAW_DATA(p_accel, p_gyro)          IMU_FG_Get6RawData(p_accel, p_gyro)
    #define IMU_SENSOR_UPDATE_FROM_UART(p_accel, p_gyro)        IMU_FG_UpdateIMUFromUart(p_accel, p_gyro)

#else
    #error "Incorrect IMU sensor setting"

#endif


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* IMU calibration mode definition */
typedef enum imu_sensor_op{

    IMU_SENSOR_CAL_PROG_DEF         = 0,    /* Apply code default calibrated bias. */
    IMU_SENSOR_CAL_RUNTIME,                 /* Perform calibration procedure now. */
    IMU_SENSOR_CAL_NONE,                    /* Do NOT apply any calibrated bias. */
}IMU_SENSOR_CAL_OP;

/* IMU sensors raw data */
typedef struct imu_sensor_data{
    int16_t accel_raw[IMU_AXES];
    int16_t gyro_raw[IMU_AXES];
}IMU_SENSOR_DATA;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

#if defined(IMU_SENSOR_PC_EN)
extern uint16_t IMU_SIM_SampleMicros;
#endif


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

int8_t IMU_Init();
int8_t IMU_Get6RawData(IMU_SENSOR_DATA *p_data);
int8_t IMU_DoCalibration(IMU_SENSOR_CAL_OP cal_mode);
void IMU_SetCalibratedBias(int16_t *p_accel_bias, int16_t *p_gyro_bias);
void IMU_GetCalibratedBias(int16_t *p_accel_bias, int16_t *p_gyro_bias);

#if defined(IMU_SENSOR_FG_EN)
void IMU_FG_UpdateIMUFromUart(int16_t *p_accel_xyz, int16_t *p_gyro_xyz);
#endif


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

#endif // IMU_CTRL_H_
