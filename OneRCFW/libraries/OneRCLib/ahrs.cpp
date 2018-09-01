/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    ahrs.cpp
 * @brief   AHRS (attitude and heading reference system)
 *
 *          Abbreviations:
 *              ACCEL   - Accelerometer.
 *              AHRS    - Attitude and Heading Reference System.
 *              ATT     - Attitude.
 *              APPROX  - Approximation.
 *              CF      - Complementary Filter.
 *              DEG     - Degree.
 *              DPS     - Degree Per Second.
 *              FXP     - Fixed Point.
 *              G       - Gravity.
 *              GYRO    - Gyroscope.
 *              INS     - Inertial Navigation System.
 *              MUL     - Multiple.
 *              NED     - North East Down.
 *              RAD     - Radian.
 *              RADS    - Radian Per Second.
 *              THR     - Threshold.
 *              VCTR    - Vector.
 *              SQ      - Square.
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <math.h>
#include <string.h>

#include "ahrs.h"
#include "imu_ctrl.h"
#include "math_lib.h"
#include "uart_stream.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Sensors */
#define AHRS_UNIT_1G        IMU_SENSOR_UNIT_1G
#define AHRS_UNIT_1DPS      IMU_SENSOR_UNIT_1DPS

/* Accelerometer low pass filter setting, set to 0 for disabling smoothing */
#define AHRS_ALPF_BETA      4

/* Sampling rate of sensor data */
#define AHRS_SAMPLE_TIME    IMU_SENSOR_SAMPLE_TIME  /* Period between each sample data, unit: us */
#define AHRS_SECOND         1000000                 /* Microseconds in one second */

/* Fixed point setting, Using s16.15 format for general purpose */
#define AHRS_FXP_SHIFT      15
#define AHRS_FXP_SCALE      (((int32_t)1 << AHRS_FXP_SHIFT))
#define AHRS_FXP_ONE        AHRS_FXP_SCALE

/*
 * Using s20.11 format for calculating square of gravity vector.
 *
 * We assume the maximum measured gravity on each accelerometer
 * axis is 16G, it's risky and may caused accumulating overflow
 * if we use general fixed point format to calculate gravity
 * vector, so the safe way for calculating gravity vector in
 * fixed point format is to reserve more bits for integer part
 * but sacrifice fractional bits (This is trade-off).
 *
 * For example, by using s20.11 fixed point format for calculating
 * gravity vector, 1G is equal to (1 << 11) = 2048, assuming the
 * maximum gravity on one axis is 16G, 16 x 2048 = 32768,
 * 16G x 16G = (32768 * 32768) >> 11 = 0x40000000 >> 11,
 * the 0x40000000 is just OK and won't cause int32_t overflow.
 */
#define AHRS_FXP_SQ_SHIFT       11

#if AHRS_FXP_SQ_SHIFT > AHRS_FXP_SHIFT
#error "Incorrect AHRS_FXP_SQ_SHIFT setting."
#endif // AHRS_FXP_SQ_SHIFT

/* Complementary filter setting */
#define AHRS_G_MIN_THR          0.82
#define AHRS_G_MAX_THR          1.18
#define AHRS_G_SQ_MIN_THR_FXP   (int32_t)(AHRS_G_MIN_THR * AHRS_G_MIN_THR   \
                                * (((int32_t)1 << AHRS_FXP_SQ_SHIFT)))
#define AHRS_G_SQ_MAX_THR_FXP   (int32_t)(AHRS_G_MAX_THR * AHRS_G_MAX_THR   \
                                * (((int32_t)1 << AHRS_FXP_SQ_SHIFT)))

#define AHRS_CF_GYRO_RATIO      0.98
#define AHRS_CF_K(delta_t)      (AHRS_CF_GYRO_RATIO                         \
                                / (1.0 + (float)delta_t / AHRS_SECOND))

/*
 * Gyroscope angular velocity threshold filter, +-0.5 DPS by default
 * #. Reduced Integration Time Improves Accuracy in Dead Reckoning Navigation Systems
 * #. http://www.analog.com/en/analog-dialogue/articles/reduced-integration-time-improves-accuracy.html
 * #. Minimal-Drift Heading Measurement using a MEMS Gyro for Indoor Mobile Robots
 * #. http://www.mdpi.com/1424-8220/8/11/7287/pdf
 */
#define AHRS_GYRO_DPS_THR       0.5
#define AHRS_GYRO_SENSOR_THR    (AHRS_GYRO_DPS_THR * AHRS_UNIT_1DPS)


/*
 *******************************************************************************
 * Macros definition
 *******************************************************************************
 */

#define AHRS_FXP_MUL(x, y)                       ((x * y) >> AHRS_FXP_SHIFT)
#define AHRS_FXP_SQ_MUL(x, y)                    ((x * y) >> AHRS_FXP_SQ_SHIFT)
#define AHRS_GYRO_TO_FXP_RADS(gyro_data, delta_t)                           \
        ((gyro_data * (int32_t)delta_t) * ((AHRS_FXP_SCALE  * MATH_PI)      \
                      / (180.0 * AHRS_SECOND * AHRS_UNIT_1DPS)))


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

#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG
    float AHRS_SimRollAngle;
    float AHRS_SimPitchAngle;
    float AHRS_SimYawAngle;
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

static int8_t AHRS_AccelVectorUpdate(int16_t *p_sensor_in, AHRS_ACCEL_DATA *p_accel_out);
static int8_t AHRS_VectorRotate(int32_t *p_fxp_rads, uint16_t delta_micros,
                                int32_t *p_fxp_vctr);
static int8_t AHRS_ComplementaryFilter(int32_t *p_reference_fxp_vctr,
                                       int32_t *p_fused_fxp_vctr,
                                       uint16_t delta_micros);
static int8_t AHRS_FxpVctrNorm(int32_t *p_vctr);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * AHRS_Init - Function initialize related IMU functions
 *
 * @param   [in]        *p_ahrs         The core data structure for storing
 *                                      current attitude information, this
 *                                      structure variable will be reset and
 *                                      reinitialize by this function.
 *
 * @param   [in]        *p_accel_raw    Initial accelerometer vector value which
 *                                      will be assigned as gyro scope initial
 *                                      vector value. Input NULL means using
 *                                      default vector value [0, 0, 1].
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t AHRS_Init(AHRS_DATA *p_ahrs, int16_t *p_accel_raw)
{

#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG
    AHRS_SimRollAngle = 0;
    AHRS_SimPitchAngle = 0;
    AHRS_SimYawAngle = 0;
#endif

    /* General */
    p_ahrs->delta_time = 0;
    p_ahrs->accel.fxp_vctr[AHRS_X] = 0;
    p_ahrs->accel.fxp_vctr[AHRS_Y] = 0;
    p_ahrs->accel.fxp_vctr[AHRS_Z] = 0;
    p_ahrs->accel.G_SQ_FXP = 0;
    p_ahrs->gyro_sensor_data[AHRS_X] = 0;
    p_ahrs->gyro_sensor_data[AHRS_Y] = 0;
    p_ahrs->gyro_sensor_data[AHRS_Z] = 0;
    p_ahrs->gyro_fxp_rads[AHRS_X] = 0;
    p_ahrs->gyro_fxp_rads[AHRS_Y] = 0;
    p_ahrs->gyro_fxp_rads[AHRS_Z] = 0;
    p_ahrs->ned_att.roll_angle = 0;
    p_ahrs->ned_att.pitch_angle = 0;
    p_ahrs->ned_att.heading_angle = 0;
    p_ahrs->body_att.roll_angle = 0;
    p_ahrs->body_att.pitch_angle = 0;
    p_ahrs->body_att.yaw_angle = 0;


    /* Initialize Heading vector */
    p_ahrs->heading_fxp_vctr[AHRS_X] = AHRS_FXP_ONE;
    p_ahrs->heading_fxp_vctr[AHRS_Y] = 0;
    p_ahrs->heading_fxp_vctr[AHRS_Z] = 0;

    /* Initialize Roll/Pitch vector */
    if(p_accel_raw == NULL){
        p_ahrs->level_fxp_vctr[AHRS_X] = 0;
        p_ahrs->level_fxp_vctr[AHRS_Y] = 0;
        p_ahrs->level_fxp_vctr[AHRS_Z] = AHRS_FXP_ONE;
    }
    else{
        p_ahrs->level_fxp_vctr[AHRS_X] = (int32_t)p_accel_raw[AHRS_X]
                                       * AHRS_FXP_SCALE / AHRS_UNIT_1G;
        p_ahrs->level_fxp_vctr[AHRS_Y] = (int32_t)p_accel_raw[AHRS_Y]
                                       * AHRS_FXP_SCALE / AHRS_UNIT_1G;
        p_ahrs->level_fxp_vctr[AHRS_Z] = (int32_t)p_accel_raw[AHRS_Z]
                                       * AHRS_FXP_SCALE / AHRS_UNIT_1G;
    }

    return 0;
}

/**
 * AHRS_AttAngleUpdate - Function to estimate current NED angle based on
 *                      accelerometer and gyroscope input.
 *
 * @param   [in]        *p_accel_raw    A 3x2 bytes array contains new incoming
 *                                      raw sensor data of accelerometer, the
 *                                      data order should be X->Y->Z from LSB.
 *
 * @param   [in]        *p_gyro_raw     A 3x2 bytes array contains new incoming
 *                                      raw sensor data of gyroscope, the data
 *                                      order should be X->Y->Z from LSB.
 *
 * @param   [input]     delta_micros    The delta time between previous and current
 *                                      IMU update. (0~65535 microseconds).
 *
 * @param   [out]       *p_ahrs         The core data structure for storing current
 *                                      attitude information, this structure can be
 *                                      written by IMU functions only.
 *
 *                                      p_ahrs->att_pitch_angle: Current NED pitch angle.
 *                                      p_ahrs->att_roll_angle: Current NED roll angle.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t AHRS_AttAngleUpdate(int16_t *p_accel_raw, int16_t *p_gyro_raw,
                           uint16_t delta_micros, AHRS_DATA *p_ahrs)
{
    int32_t x_vctr;
    int32_t y_vctr;
    int32_t z_vctr;
    int32_t x_vctr_sq;
    int32_t y_vctr_sq;
    int32_t z_vctr_sq;
    float gyro_raw_tmp;

    /* Store delta time */
    p_ahrs->delta_time = delta_micros;

    /* Store sensor raw data and convert to specific unit first */
    p_ahrs->accel.sensor_data[AHRS_X] = p_accel_raw[AHRS_X];
    p_ahrs->accel.sensor_data[AHRS_Y] = p_accel_raw[AHRS_Y];
    p_ahrs->accel.sensor_data[AHRS_Z] = p_accel_raw[AHRS_Z];

    p_ahrs->gyro_sensor_data[AHRS_X] = p_gyro_raw[AHRS_X];
    p_ahrs->gyro_sensor_data[AHRS_Y] = p_gyro_raw[AHRS_Y];
    p_ahrs->gyro_sensor_data[AHRS_Z] = p_gyro_raw[AHRS_Z];

    /*
     * Apply gyro threshold filter, accumulate the rigid body rotation angle,
     * and convert the gyro raw data to fixed point format for later calculation.
     */
    gyro_raw_tmp = (float)p_gyro_raw[AHRS_X];
    gyro_raw_tmp = (fabs(gyro_raw_tmp) > AHRS_GYRO_SENSOR_THR) ? gyro_raw_tmp : 0.0;
    p_ahrs->gyro_fxp_rads[AHRS_X] = AHRS_GYRO_TO_FXP_RADS(gyro_raw_tmp, delta_micros);

    p_ahrs->body_att.roll_angle += gyro_raw_tmp * (1.0 / AHRS_UNIT_1DPS) * delta_micros * 0.000001;
    if(p_ahrs->body_att.roll_angle > 360.0)
        p_ahrs->body_att.roll_angle -= 360.0;
    else if(p_ahrs->body_att.roll_angle < 0.0)
        p_ahrs->body_att.roll_angle += 360.0;

    gyro_raw_tmp = (float)p_gyro_raw[AHRS_Y];
    gyro_raw_tmp = (fabs(gyro_raw_tmp) > AHRS_GYRO_SENSOR_THR) ? gyro_raw_tmp : 0.0;
    p_ahrs->gyro_fxp_rads[AHRS_Y] = AHRS_GYRO_TO_FXP_RADS(gyro_raw_tmp, delta_micros);

    p_ahrs->body_att.pitch_angle += gyro_raw_tmp * (1.0 / AHRS_UNIT_1DPS) * delta_micros * 0.000001;
    if(p_ahrs->body_att.pitch_angle > 360.0)
        p_ahrs->body_att.pitch_angle -= 360.0;
    else if(p_ahrs->body_att.pitch_angle < 0.0)
        p_ahrs->body_att.pitch_angle += 360.0;

    gyro_raw_tmp = (float)p_gyro_raw[AHRS_Z];
    gyro_raw_tmp = (fabs(gyro_raw_tmp) > AHRS_GYRO_SENSOR_THR) ? gyro_raw_tmp : 0.0;
    p_ahrs->gyro_fxp_rads[AHRS_Z] = AHRS_GYRO_TO_FXP_RADS(gyro_raw_tmp, delta_micros);

    p_ahrs->body_att.yaw_angle += gyro_raw_tmp * (1.0 / AHRS_UNIT_1DPS) * delta_micros * 0.000001;
    if(p_ahrs->body_att.yaw_angle > 360.0)
        p_ahrs->body_att.yaw_angle -= 360.0;
    else if(p_ahrs->body_att.yaw_angle < 0.0)
        p_ahrs->body_att.yaw_angle += 360.0;

    /* Update NED vector according accelerometer sensing */
    AHRS_AccelVectorUpdate(p_ahrs->accel.sensor_data, &(p_ahrs->accel));

    /* Rotate XY leveling vector according to gyroscope sensing */
    AHRS_VectorRotate(p_ahrs->gyro_fxp_rads, delta_micros, p_ahrs->level_fxp_vctr);

    /*
     * Fusing the current level vector with accelerometer vector by complementary
     * Filter only if the amount of accelerometer vector is less than or equal to
     * specific gravitational acceleration.
     * The fused data will be stored in p_ahrs->level_fxp_vctr.
     */
    if(p_ahrs->accel.G_SQ_FXP >= AHRS_G_SQ_MIN_THR_FXP
       && p_ahrs->accel.G_SQ_FXP <= AHRS_G_SQ_MAX_THR_FXP){

        AHRS_ComplementaryFilter(p_ahrs->accel.fxp_vctr, p_ahrs->level_fxp_vctr, delta_micros);
    }
    else{
        p_ahrs->accel_exceed_cnt++;
    }

    /*
     * Notice:
     *      Although we perform fixed point multiplication, but we don't have
     *      to right shift the result here, since we will only input these values
     *      to sqrt function later, and that require left shift the input value
     *      first for inputting fixed point number to a float point based sqrt
     *      function, so, the right shift and left shift can cancel off each other.
     *
     *      Say value 1 in s15.16 fixed point format is equal to 32768,
     *      1 x 1 = ((32768 x 32768) >> 15) = 1.
     *
     *      But if we input the fixed point value to sqrt function directly
     *      like sqrt((32768 x 32768) >> 15), we will get incorrect result since
     *      sqrt((32768 x 32768) >> 15) = sqrt(32768) ~= 181, this is not what we
     *      expected, we should just input (32768 x 32768) to sqrt function and it
     *      will output 32768, 32768 >> 15 = 1.
     */
    x_vctr = p_ahrs->level_fxp_vctr[AHRS_X];
    y_vctr = p_ahrs->level_fxp_vctr[AHRS_Y];
    z_vctr = p_ahrs->level_fxp_vctr[AHRS_Z];

    x_vctr_sq = x_vctr * x_vctr;
    y_vctr_sq = y_vctr * y_vctr;
    z_vctr_sq = z_vctr * z_vctr;

    /*
     * Calculate current NED pitch and roll angle (degree),
     *      Output roll angle range:    -180 ~ 180.
     *      Output pitch angle range:   0 ~ 90.
     */
    p_ahrs->ned_att.roll_angle = MATH_RAD2DEG(atan2(y_vctr, z_vctr));
    p_ahrs->ned_att.pitch_angle = MATH_RAD2DEG(atan2(x_vctr, Math_FastSqrt(y_vctr_sq + z_vctr_sq)));

    /* Rotate Z heading vector according to gyroscope sensing */
    AHRS_VectorRotate(p_ahrs->gyro_fxp_rads, delta_micros, p_ahrs->heading_fxp_vctr);

    /* Compute heading angle, 0 ~ 360 */
    p_ahrs->ned_att.heading_angle = MATH_RAD2DEG(atan2(p_ahrs->heading_fxp_vctr[AHRS_Y],
                                                       p_ahrs->heading_fxp_vctr[AHRS_X]));
    if(p_ahrs->ned_att.heading_angle < 0.0){
         p_ahrs->ned_att.heading_angle += 360.0;
    }

#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG
    p_ahrs->ned_att.roll_angle = AHRS_SimRollAngle;
    p_ahrs->ned_att.pitch_angle = AHRS_SimPitchAngle;
    p_ahrs->ned_att.heading_angle = AHRS_SimYawAngle;
#endif

    return 0;
}

#if defined(IMU_SENSOR_ANGLE_FROM_FG) && IMU_SENSOR_ANGLE_FROM_FG
void AHRS_SetSimAngle(float roll_angle, float pitch_angle, float yaw_angle)
{
    AHRS_SimRollAngle = roll_angle;
    AHRS_SimPitchAngle = pitch_angle;
    AHRS_SimYawAngle = yaw_angle;
}
#endif

/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * AHRS_AccelVectorUpdate - Function to update gravity vector according to
 *                          inputted raw sensing of accelerometer.
 *                          The output data will be convert to fixed point.
 *
 * @param   [in]        *p_sensor_in    Raw sensing of accelerometer.
 *                                      Input data should be int16_t * 3
 *                                      = {X, Y, Z}.
 *
 * @param   [out]       *p_accel_out    p_accel_out->fxp_vctr
 *                                      p_accel_out->G_SQ
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t AHRS_AccelVectorUpdate(int16_t *p_sensor_in, AHRS_ACCEL_DATA *p_accel_out)
{
    int32_t accel_axis[AHRS_AXES];
    int32_t *p_fxp_vctr;

    p_fxp_vctr = p_accel_out->fxp_vctr;

    /* Convert to 1G unit in fixed point format */
    accel_axis[AHRS_X] = (int32_t)p_sensor_in[AHRS_X] * AHRS_FXP_SCALE / AHRS_UNIT_1G;
    accel_axis[AHRS_Y] = (int32_t)p_sensor_in[AHRS_Y] * AHRS_FXP_SCALE / AHRS_UNIT_1G;
    accel_axis[AHRS_Z] = (int32_t)p_sensor_in[AHRS_Z] * AHRS_FXP_SCALE / AHRS_UNIT_1G;

    /* Smoothing data, (SmoothData = SmoothData - (LPF_Beta * (SmoothData - RawData)) */
    p_fxp_vctr[AHRS_X] -= ((p_fxp_vctr[AHRS_X] - accel_axis[AHRS_X]) >> AHRS_ALPF_BETA);
    p_fxp_vctr[AHRS_Y] -= ((p_fxp_vctr[AHRS_Y] - accel_axis[AHRS_Y]) >> AHRS_ALPF_BETA);
    p_fxp_vctr[AHRS_Z] -= ((p_fxp_vctr[AHRS_Z] - accel_axis[AHRS_Z]) >> AHRS_ALPF_BETA);

    /*
     * Remove some fractional bits in variables and store them in different fixed
     * point format for avoiding accumulation overflow when calculate the square
     * of gravity vector (x^2 + y^2 + z^2).
     *
     * The gravity square will be referenced by other function later, please take
     * care the value in G_SQ variable has already been convert to different fixed
     * point format (s19.12 by default).
     *
     * For more details, please check the comment of AHRS_FXP_SQ_SHIFT definition.
     */
    accel_axis[AHRS_X] = (p_fxp_vctr[AHRS_X] >> (AHRS_FXP_SHIFT - AHRS_FXP_SQ_SHIFT));
    accel_axis[AHRS_Y] = (p_fxp_vctr[AHRS_Y] >> (AHRS_FXP_SHIFT - AHRS_FXP_SQ_SHIFT));
    accel_axis[AHRS_Z] = (p_fxp_vctr[AHRS_Z] >> (AHRS_FXP_SHIFT - AHRS_FXP_SQ_SHIFT));

    /* Calculate x^2 + y^2 + z^2 by using AHRS_FXP_SQ_MUL macro */
    p_accel_out->G_SQ_FXP = AHRS_FXP_SQ_MUL(accel_axis[AHRS_X], accel_axis[AHRS_X])
                          + AHRS_FXP_SQ_MUL(accel_axis[AHRS_Y], accel_axis[AHRS_Y])
                          + AHRS_FXP_SQ_MUL(accel_axis[AHRS_Z], accel_axis[AHRS_Z]);

    return 0;
}

/**
 * AHRS_VectorRotate - Function to rotate vector according to inputted
 *                    {X, Y, Z} angular velocity.
 *
 * #. The input data should be converted to fixed point format.
 *
 * @param   [in]        *p_fxp_rads     An int32_t[3] array contains current
 *                                      {X, Y, Z} angular velocity in radian/second.
 *
 * @param   [in]        delta_micros    The delta time between previous and current
 *                                      IMU update. (0~65535 microseconds)
 *
 * @param   [in/out]    *p_fxp_vctr     The vector will be rotated.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t AHRS_VectorRotate(int32_t *p_fxp_rads, uint16_t delta_micros,
                                int32_t *p_fxp_vctr)
{
    int32_t gyro_roll_fxp;
    int32_t gyro_pitch_fxp;
    int32_t gyro_yaw_fxp;
    int32_t fxp_matrix_x;
    int32_t fxp_matrix_y;
    int32_t fxp_matrix_z;

    /* Convert gyro data to radian per second in fixed point format */
    gyro_roll_fxp = p_fxp_rads[AHRS_X];
    gyro_pitch_fxp = p_fxp_rads[AHRS_Y];
    gyro_yaw_fxp = p_fxp_rads[AHRS_Z];

    fxp_matrix_x = p_fxp_vctr[AHRS_X];
    fxp_matrix_y = p_fxp_vctr[AHRS_Y];
    fxp_matrix_z = p_fxp_vctr[AHRS_Z];

    /* 3D rotation matrix with small angle approximation */
    p_fxp_vctr[AHRS_X] = p_fxp_vctr[AHRS_X]
                       + AHRS_FXP_MUL(gyro_yaw_fxp, fxp_matrix_y)
                       - AHRS_FXP_MUL(gyro_pitch_fxp, fxp_matrix_z);
    p_fxp_vctr[AHRS_Y] = p_fxp_vctr[AHRS_Y]
                       - AHRS_FXP_MUL(gyro_yaw_fxp, fxp_matrix_x)
                       + AHRS_FXP_MUL(gyro_roll_fxp, fxp_matrix_z);
    p_fxp_vctr[AHRS_Z] = p_fxp_vctr[AHRS_Z]
                       + AHRS_FXP_MUL(gyro_pitch_fxp, fxp_matrix_x)
                       - AHRS_FXP_MUL(gyro_roll_fxp, fxp_matrix_y);

    /* Normalize vector */
    AHRS_FxpVctrNorm(p_fxp_vctr);

    return 0;
}

/**
 * AHRS_ComplementaryFilter -
 *
 * @param   [in]        *p_reference_fxp_vctr

 * @param   [in/out]    *p_fused_fxp_vctr
 *
 * @param   [in]        delta_micros            The delta time between previous and current
 *                                              IMU update. (0~65535 microseconds)
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t AHRS_ComplementaryFilter(int32_t *p_reference_fxp_vctr,
                                       int32_t *p_fused_fxp_vctr,
                                       uint16_t delta_micros)
{

    p_fused_fxp_vctr[AHRS_X] = p_fused_fxp_vctr[AHRS_X] * AHRS_CF_K(delta_micros)
                             + p_reference_fxp_vctr[AHRS_X] * (1.0 - AHRS_CF_K(delta_micros));
    p_fused_fxp_vctr[AHRS_Y] = p_fused_fxp_vctr[AHRS_Y] * AHRS_CF_K(delta_micros)
                             + p_reference_fxp_vctr[AHRS_Y] * (1.0 - AHRS_CF_K(delta_micros));
    p_fused_fxp_vctr[AHRS_Z] = p_fused_fxp_vctr[AHRS_Z] * AHRS_CF_K(delta_micros)
                             + p_reference_fxp_vctr[AHRS_Z] * (1.0 - AHRS_CF_K(delta_micros));

    return 0;
}

/**
 * AHRS_FxpVctrNorm - Function to normalize vector in fixed point format.
 *
 * https://en.wikipedia.org/wiki/Unit_vector
 *
 * Original formula:
 *      old vector = [x, y, z]
 *      sq_root = sqrt(x * x + y * y + z * z)
 *      new vector = [x / sq_root, y / sq_root, z / sq_root]
 *
 * Notice: The formula has already been simplified for fixed point calculation
 *         in this function.
 *
 * @param   [in/out]    *p_vctr     A int32_t * 3 array which contains vector
 *                                  data needs to be normalized.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t AHRS_FxpVctrNorm(int32_t *p_vctr)
{
    uint32_t fxp_vctr_sq;
    float vctr_inv_sqrt;

    fxp_vctr_sq = p_vctr[AHRS_X] * p_vctr[AHRS_X]
                + p_vctr[AHRS_Y] * p_vctr[AHRS_Y]
                + p_vctr[AHRS_Z] * p_vctr[AHRS_Z];

    vctr_inv_sqrt = Math_FastInvSqrt(fxp_vctr_sq) * AHRS_FXP_SCALE;

    p_vctr[AHRS_X] = p_vctr[AHRS_X] * vctr_inv_sqrt;
    p_vctr[AHRS_Y] = p_vctr[AHRS_Y] * vctr_inv_sqrt;
    p_vctr[AHRS_Z] = p_vctr[AHRS_Z] * vctr_inv_sqrt;

    return 0;
}

