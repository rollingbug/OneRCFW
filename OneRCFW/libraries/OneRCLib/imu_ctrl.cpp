/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    imu_ctrl.cpp
 * @brief   IMU (inertial measurement unit)
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include "imu_ctrl.h"
#include "uart_stream.h"
#include "timers_drv.h"
#include "leds_ctrl.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Calibration */
#define IMU_CALIBRATE_NUM       600         /* Sampling number */
#define IMU_CALIBRATE_INTERVAL  5           /* Millisecond */


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

#if defined(IMU_SENSOR_PC_EN)
uint16_t IMU_SIM_SampleMicros;
#endif

#if defined(IMU_SENSOR_FG_EN)
static IMU_SENSOR_DATA IMU_FG_Sensor;
#endif

static int16_t IMU_AccelBias[IMU_AXES];
static int16_t IMU_GyroBias[IMU_AXES];


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */

#if defined(IMU_SENSOR_PC_EN)
static int8_t IMU_SIM_Init();
static int8_t IMU_SIM_Get6RawData(int16_t *p_accel_xyz, int16_t *p_gyro_xyz);
#endif

#if defined(IMU_SENSOR_FG_EN)
static int8_t IMU_FG_Init();
static int8_t IMU_FG_Get6RawData(int16_t *p_accel_xyz, int16_t *p_gyro_xyz);
#endif


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * IMU_Init - Function to initialize IMU sensors.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t IMU_Init()
{
    int8_t ret_val;
    const char *p_ret_msg[] = {"OK", "Fail"};

    /* Set default bias to zero */
    memset((void *)IMU_AccelBias, 0, sizeof(IMU_AccelBias));
    memset((void *)IMU_GyroBias, 0, sizeof(IMU_GyroBias));

    Uart0_Printf(PSTR("[IMU] Device %s : "), IMU_SENSOR_NAME);

    /* Initialize IMU (Accelerometer and gyroscope) */
    ret_val = IMU_SENSOR_INIT();

    Uart0_Println(PSTR("%s"), ret_val ? p_ret_msg[1] : p_ret_msg[0]);

    return ret_val;
}

/**
 * IMU_DoCalibration - Function to launch IMU calibration procedure..
 *
 * @param   [cal_mode]  Calibration mode:
 *                      IMU_SENSOR_CAL_PROG_DEF -
 *                          Apply code default calibrated bias.
 *                      IMU_SENSOR_CAL_RUNTIME -
 *                          Perform calibration procedure now.
 *                      IMU_SENSOR_CAL_NONE -
 *                          Do NOT apply any calibrated bias.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t IMU_DoCalibration(IMU_SENSOR_CAL_OP cal_mode)
{
    int16_t accel_bais_def[IMU_AXES] = IMU_SENSOR_CAL_ACCEL_DEF;
    int16_t gyro_bais_def[IMU_AXES] = IMU_SENSOR_CAL_GYRO_DEF;
    int16_t accel_raw[IMU_AXES] = {0};
    int16_t gyro_raw[IMU_AXES] = {0};
    int32_t accel_accum[IMU_AXES] = {0};
    int32_t gyro_accum[IMU_AXES] = {0};
    uint16_t cnt = 0;
    uint8_t idx = 0;

    switch(cal_mode){

        /* Using code default bias */
        case IMU_SENSOR_CAL_PROG_DEF:

            memcpy((void *)IMU_AccelBias, (void *)accel_bais_def, sizeof(IMU_AccelBias));
            memcpy((void *)IMU_GyroBias, (void *)gyro_bais_def, sizeof(IMU_GyroBias));

            break;

        /* Measure the bias now */
        case IMU_SENSOR_CAL_RUNTIME:

            /* collect N number raw samples */
            while(cnt < IMU_CALIBRATE_NUM){

                IMU_SENSOR_GET_6_RAW_DATA(accel_raw, gyro_raw);

                idx = 0;
                while(idx < IMU_AXES){

                    accel_accum[idx] += accel_raw[idx];
                    gyro_accum[idx] += gyro_raw[idx];

                    idx++;
                }

                cnt++;

                LEDS_Blink(LEDS_SLAVE_IDX, 20, 60);
                Timer1_DelayMillis(IMU_CALIBRATE_INTERVAL);
            }

            LEDS_PwrOFF(LEDS_SLAVE_IDX);

            /*
            * Calculate the average of accumulated accelerometer samples.
            * Please notice that the default Z axis is perpendicular to the horizontal plane,
            * that means the sensing value on Z axis containing both 1G gravity force and noise.
            */
            IMU_AccelBias[IMU_X] = (int16_t)(accel_accum[IMU_X] / IMU_CALIBRATE_NUM);
            IMU_AccelBias[IMU_Y] = (int16_t)(accel_accum[IMU_Y] / IMU_CALIBRATE_NUM);
            IMU_AccelBias[IMU_Z] = (int16_t)((accel_accum[IMU_Z] / IMU_CALIBRATE_NUM) - IMU_SENSOR_UNIT_1G);

            /* Calculate the average of accumulated gyroscope samples. */
            IMU_GyroBias[IMU_X] = (int16_t)(gyro_accum[IMU_X] / IMU_CALIBRATE_NUM);
            IMU_GyroBias[IMU_Y] = (int16_t)(gyro_accum[IMU_Y] / IMU_CALIBRATE_NUM);
            IMU_GyroBias[IMU_Z] = (int16_t)(gyro_accum[IMU_Z] / IMU_CALIBRATE_NUM);

            break;

        /* Do not apply any calibration */
        case IMU_SENSOR_CAL_NONE:
        default:

            memset((void *)IMU_AccelBias, 0, sizeof(IMU_AccelBias));
            memset((void *)IMU_GyroBias, 0, sizeof(IMU_GyroBias));

            break;
    }

    return 0;
}

/**
 * IMU_SetCalibratedBias - Function to set bias for accelerometer and gyroscope.
 *
 * @param   [in]        *p_accel_bias       A int16_t array[IMU_AXES] which contains
 *                                          predefined bias setting of accelerometer.
 * @param   [in]        *p_gyro_bias        A int16_t array[IMU_AXES] which contains
 *                                          predefined bias setting of gyroscope.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
void IMU_SetCalibratedBias(int16_t *p_accel_bias, int16_t *p_gyro_bias)
{
    memcpy((void *)IMU_AccelBias, (void *)p_accel_bias, sizeof(IMU_AccelBias));
    memcpy((void *)IMU_GyroBias, (void *)p_gyro_bias, sizeof(IMU_GyroBias));
}

/**
 * IMU_GetCalibratedBias - Function to get current bias setting of accelerometer and gyroscope.
 *
 * @param   [out]        *p_accel_bias      A int16_t array[IMU_AXES] for storing current
 *                                          bias setting of accelerometer.
 * @param   [out]        *p_gyro_bias       A int16_t array[IMU_AXES] for storing current
 *                                          bias setting of gyroscope.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
void IMU_GetCalibratedBias(int16_t *p_accel_bias, int16_t *p_gyro_bias)
{
    memcpy((void *)p_accel_bias, (void *)IMU_AccelBias, sizeof(IMU_AccelBias));
    memcpy((void *)p_gyro_bias, (void *)IMU_GyroBias, sizeof(IMU_GyroBias));
}

/**
 * IMU_Get6RawData - Function to get raw sensing of accelerometer and gyroscope.
 *
 * @param   [out]       **p_data        Raw sensing of accelerometer and gyroscope.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t IMU_Get6RawData(IMU_SENSOR_DATA *p_data)
{
    int8_t ret_val;

    ret_val = IMU_SENSOR_GET_6_RAW_DATA(p_data->accel_raw, p_data->gyro_raw);

    /* Correct bias */
    p_data->accel_raw[IMU_X] -= IMU_AccelBias[IMU_X];
    p_data->accel_raw[IMU_Y] -= IMU_AccelBias[IMU_Y];
    p_data->accel_raw[IMU_Z] -= IMU_AccelBias[IMU_Z];
    p_data->gyro_raw[IMU_X] -= IMU_GyroBias[IMU_X];
    p_data->gyro_raw[IMU_Y] -= IMU_GyroBias[IMU_Y];
    p_data->gyro_raw[IMU_Z] -= IMU_GyroBias[IMU_Z];

    return ret_val;
}


/*
 *******************************************************************************
 * Simulating sensor functions (PC off-line data)
 *******************************************************************************
 */

/**
 * IMU_SIM_Init - Function to initialize virtual IMU component for doing
 *                off-line simulation.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
#if defined(IMU_SENSOR_PC_EN)
static int8_t IMU_SIM_Init()
{
    return 0;
}
#endif

/**
 * IMU_SIM_Get6RawData - Function to read off-line accelerometer and gyroscope
 *                       through serial.
 *
 * This function is created for performing off-line IMU simulation, it will waiting
 * for any available pre-captured off line accelerometer and gyroscope XYZ raw data
 * which is transmitted by PC through serial port, then return the captured value.
 * to function caller.
 *
 * Current off-line serial ASCII data format is shown below:
 *
 *      <data_idx>, <AX>, <AY>, <AZ>, <GX, GY, GZ> <sample_micros>
 *
 *      1, -24, 4, 4123, 3, -1, 4, 5004
 *      2, -15, -3, 4093, 1, 2, 5, 5000
 *      3, 10, -7, 4106, 2, 0, 0, 5008
 *      4, ..
 *      5, ..
 *
 * @param   [out]       *p_accel_xyz    Raw sensing of accelerometer,
 *                                      the input buffer should be
 *                                      int16_t array[3] for storing
 *                                      X, Y and Z axis value.
 *
 * @param   [out]       *p_gyro_xyz     Raw sensing of gyroscope,
 *                                      the input buffer should be
 *                                      int16_t array[3].
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
#if defined(IMU_SENSOR_PC_EN)
static int8_t IMU_SIM_Get6RawData(int16_t *p_accel_xyz, int16_t *p_gyro_xyz)
{
    bool is_data_ready;
    int8_t ret_val;
    char str_buffer[256];
    char *p_str_tmp;
    int32_t current_idx;
    int16_t accel_raw[3];
    int16_t gyro_raw[3];

    if(Uart0_ReadAvailable() > 0)
    {
        memset((void *)str_buffer, 0, sizeof(str_buffer));

        is_data_ready = false;
        p_str_tmp = str_buffer;
        while(p_str_tmp < str_buffer + sizeof(str_buffer)){
            if(Uart0_ReadByte((uint8_t *)p_str_tmp) == 1){

                if(*p_str_tmp == '\r')
                    *p_str_tmp = 0;
                else if(*p_str_tmp == '\n'){
                    *p_str_tmp = 0;
                    is_data_ready = true;
                    break;
                }

                p_str_tmp++;
            }
        }

        if(is_data_ready == false)
            return -1;

        p_str_tmp = strtok(str_buffer, ",");
        current_idx = strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        accel_raw[0] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        accel_raw[1] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        accel_raw[2] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        gyro_raw[0] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        gyro_raw[1] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        gyro_raw[2] = (int16_t)strtol(p_str_tmp, NULL, 10);

        p_str_tmp = strtok(NULL, ",");
        IMU_SIM_SampleMicros = (uint16_t)strtoul(p_str_tmp, NULL, 10);

        memcpy(p_accel_xyz, accel_raw, sizeof(accel_raw));
        memcpy(p_gyro_xyz, gyro_raw, sizeof(gyro_raw));

        ret_val = 0;
    }
    else{
        ret_val = -1;
    }

    return ret_val;
}
#endif


/*
 *******************************************************************************
 * Simulating sensor functions (FlightGear FDM simulation)
 *******************************************************************************
 */

/**
 * IMU_FG_Init - Function to initialize virtual IMU component for doing
 *               off-line simulation.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
#if defined(IMU_SENSOR_FG_EN)
static int8_t IMU_FG_Init()
{
    IMU_FG_Sensor.accel_raw[IMU_X] = 0;
    IMU_FG_Sensor.accel_raw[IMU_Y] = 0;
    IMU_FG_Sensor.accel_raw[IMU_Z] = IMU_SENSOR_UNIT_1G;
    IMU_FG_Sensor.gyro_raw[IMU_X] = 0;
    IMU_FG_Sensor.gyro_raw[IMU_Y] = 0;
    IMU_FG_Sensor.gyro_raw[IMU_Z] = 0;

    return 0;
}
#endif

/**
 * IMU_FG_Get6RawData - Function to read off-line accelerometer and gyroscope
 *                      through serial.
 *
 * @param   [out]       *p_accel_xyz    Raw sensing of accelerometer,
 *                                      the input buffer should be
 *                                      int16_t array[3] for storing
 *                                      X, Y and Z axis value.
 *
 * @param   [out]       *p_gyro_xyz     Raw sensing of gyroscope,
 *                                      the input buffer should be
 *                                      int16_t array[3].
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
#if defined(IMU_SENSOR_FG_EN)
static int8_t IMU_FG_Get6RawData(int16_t *p_accel_xyz, int16_t *p_gyro_xyz)
{
    memcpy(p_accel_xyz, IMU_FG_Sensor.accel_raw, sizeof(IMU_FG_Sensor.accel_raw));
    memcpy(p_gyro_xyz, IMU_FG_Sensor.gyro_raw, sizeof(IMU_FG_Sensor.gyro_raw));

    return 0;
}
#endif

/**
 * IMU_FG_UpdateIMUFromUart - Function to get accelerometer and gyroscope data which
 *                            are generated by FlightGear through serial (MP protocol).
 *
 * @param   [out]       *p_accel_xyz    Raw sensing of accelerometer,
 *                                      the input buffer should be
 *                                      int16_t array[3] for storing
 *                                      X, Y and Z axis value.
 *
 * @param   [out]       *p_gyro_xyz     Raw sensing of gyroscope,
 *                                      the input buffer should be
 *                                      int16_t array[3].
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
#if defined(IMU_SENSOR_FG_EN)
void IMU_FG_UpdateIMUFromUart(int16_t *p_accel_xyz, int16_t *p_gyro_xyz)
{
    memcpy(IMU_FG_Sensor.accel_raw, p_accel_xyz, sizeof(IMU_FG_Sensor.accel_raw));
    memcpy(IMU_FG_Sensor.gyro_raw, p_gyro_xyz, sizeof(IMU_FG_Sensor.gyro_raw));
}
#endif
