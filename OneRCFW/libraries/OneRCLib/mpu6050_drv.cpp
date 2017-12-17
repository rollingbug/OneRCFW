/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    mpu6050_drv.c
 *
 * @brief   Driver of MPU6050.
 *
 *          Abbreviations:
 *              ACCUM   - Accumulation      DPS     - Degree per second
 *              G       - gravity
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdint.h>
#include <avr/io.h>
#include <string.h>

#include "mpu6050_drv.h"
#include "i2c_drv.h"
#include "timers_drv.h"
#include "uart_stream.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */


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


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */

#if MPU6050_IS_FIFO_EN
static int8_t mpu6050_IsFIFOOverflow();
#endif

static void mpu6050_DumpRegs();

/*
 *******************************************************************************
 * Public functions implementation
 *******************************************************************************
 */

/**
 * mpu6050_Init - Function to initial MPU6050 sensor.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t mpu6050_Init()
{
    uint8_t i2c_byte;

    /* Read "Who am I" register, and value should be 0x68 */
    I2C_ReadByte(MPU6050_DEV_ID, MPU6050_REG_WHO_AM_I, &i2c_byte);
    if(i2c_byte != MPU6050_DEV_ID){
        return -1;
    }

    /* Reset MPU6050 */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_PWR_MGMT_1,
                  _BV(MPU6050_PWR_MGMT_1_RESET_BIT));

    Timer1_DelayMillis(100);

    /* Reset sensors */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_USER_CTRL,
                  _BV(MPU6050_USER_CTRL_SIG_COND_RESET_BIT));

	Timer1_DelayMillis(100);

    /* Setup reference clock, PLL with X axis gyroscope reference, and disable sleep mode */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_PWR_MGMT_1,
                  _BV(MPU6050_PWR_MGMT_1_CLK0_BIT));

    /* Setup sampling rate, 800 Hz by default */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_SMPLRT_DIV,
                  MPU6050_FIFO_SAMPLE_SETTING);

    /* Setup digital low pass filter (42 Hz)*/
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_CONFIG,
                  (MPU6050_DLPF_CFG_42HZ << MPU6050_CONFIG_DLPF_CFG0_BIT));

    /* Setup gyroscope sensing range (+-2000 degree per second) */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_GYRO_CONFIG,
                  (_BV(MPU6050_GYRO_CONFIG_FS_SEL0_BIT)
                   | _BV(MPU6050_GYRO_CONFIG_FS_SEL1_BIT)));

    /* Setup accelerometer sensing range (+- 8G) */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_ACCEL_CONFIG,
                  _BV(MPU6050_ACCEL_CONFIG_AFS_SEL1_BIT));

    /* Delay for awhile to make sure MPU6050 completed the configuration */
    Timer1_DelayMillis(100);

    /* Disable all interrupt trigger and clear interrupt status by reading the status register */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_INT_ENABLE, 0);
    I2C_ReadByte(MPU6050_DEV_ID, MPU6050_REG_INT_STATUS, &i2c_byte);

#if MPU6050_IS_FIFO_EN
    /* Enable FIFO function */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_USER_CTRL,
                  _BV(MPU6050_USER_CTRL_FIFO_EN_BIT));

    /* Enable specific FIFO data (accelerometer and gyroscope X,Y,Z axis) */
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_FIFO_EN,
                  (_BV(MPU6050_FIFO_EN_ACCEL_BIT) | _BV(MPU6050_FIFO_EN_XG_BIT)
                   | _BV(MPU6050_FIFO_EN_YG_BIT) | _BV(MPU6050_FIFO_EN_ZG_BIT)));

    /* Reset FIFO */
    I2C_ReadByte(MPU6050_DEV_ID, MPU6050_REG_USER_CTRL, &i2c_byte);
    I2C_WriteByte(MPU6050_DEV_ID, MPU6050_REG_USER_CTRL,
                  (i2c_byte | _BV(MPU6050_USER_CTRL_FIFO_RESET_BIT)));
#endif

    /* Almost ready to go */

    return 0;
}

/**
 * mpu6050_ReadXYZDirectly - Function to read current XYZ raw sensing of accelerometer
 *                           and gyroscope from MPU6050 registers directly.
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
int8_t mpu6050_ReadXYZDirectly(int16_t *p_accel_xyz, int16_t *p_gyro_xyz)
{
    uint8_t accel_raw[6] = {0};
    uint8_t gyro_raw[6] = {0};
    int8_t ret_val;

    ret_val = I2C_ReadBytes(MPU6050_DEV_ID, MPU6050_REG_ACCEL_XOUT_H,
                            sizeof(accel_raw), accel_raw);
    if(ret_val == -1)
        return -1;

    ret_val = I2C_ReadBytes(MPU6050_DEV_ID, MPU6050_REG_GYRO_XOUT_H,
                            sizeof(gyro_raw), gyro_raw);
    if(ret_val == -1)
        return -1;

    /* Convert endian and correct bias */
    p_accel_xyz[0] = ((((int16_t)accel_raw[0]) << 8) | accel_raw[1]);
    p_accel_xyz[1] = ((((int16_t)accel_raw[2]) << 8) | accel_raw[3]);
    p_accel_xyz[2] = ((((int16_t)accel_raw[4]) << 8) | accel_raw[5]);
    p_gyro_xyz[0] = ((((int16_t)gyro_raw[0]) << 8) | gyro_raw[1]);
    p_gyro_xyz[1] = ((((int16_t)gyro_raw[2]) << 8) | gyro_raw[3]);
    p_gyro_xyz[2] = ((((int16_t)gyro_raw[4]) << 8) | gyro_raw[5]);

    return 0;
}

/**
 * mpu6050_ReadXYZFromFIFO - Function to read XYZ raw sensing of
 *                           accelerometer and gyroscope from FIFO.
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
#if MPU6050_IS_FIFO_EN
int8_t mpu6050_ReadXYZFromFIFO(int16_t *p_accel_xyz, int16_t *p_gyro_xyz)
{
    uint8_t read_buf[12] = {0};
    uint16_t fifo_count;

    /* Check overflow interrupt flag */
    if(mpu6050_IsFIFOOverflow()){

        Uart0_Println(PSTR("MPU6050 FIFO overflow"));

        while(1);

        return -1;
    }

    /* Get number of bytes stored in the FIFO buffer first */
    I2C_ReadBytes(MPU6050_DEV_ID, MPU6050_REG_FIFO_COUNTH, 2, read_buf);
    fifo_count = (((uint16_t)read_buf[0]) << 8) | read_buf[1];

    if(fifo_count < sizeof(read_buf)){
        return -1;
    }

    /* Read data from FIFO */
    I2C_ReadBytes(MPU6050_DEV_ID, MPU6050_REG_FIFO_R_W,
                  sizeof(read_buf), read_buf);

    /* Convert endian and type */
    p_accel_xyz[0] = ((((int16_t)read_buf[0]) << 8) | read_buf[1]);
    p_accel_xyz[1] = ((((int16_t)read_buf[2]) << 8) | read_buf[3]);
    p_accel_xyz[2] = ((((int16_t)read_buf[4]) << 8) | read_buf[5]);
    p_gyro_xyz[0] = ((((int16_t)read_buf[6]) << 8) | read_buf[7]);
    p_gyro_xyz[1] = ((((int16_t)read_buf[8]) << 8) | read_buf[9]);
    p_gyro_xyz[2] = ((((int16_t)read_buf[10]) << 8) | read_buf[11]);

    return 0;
}
#endif

/**
 * mpu6050_DumpRegs - Function to dump all the MPU6050 registers value.
 *                    (For debugging purpose only)
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static void mpu6050_DumpRegs()
{
    unsigned char reg_value;
    unsigned char reg_cnt = 0;
    unsigned char reg_addr[] = {
        MPU6050_REG_SELF_TEST_X, MPU6050_REG_SELF_TEST_Y, MPU6050_REG_SELF_TEST_Z,
        MPU6050_REG_SELF_TEST_A, MPU6050_REG_SMPLRT_DIV, MPU6050_REG_CONFIG,
        MPU6050_REG_GYRO_CONFIG, MPU6050_REG_ACCEL_CONFIG, MPU6050_REG_FIFO_EN,
        MPU6050_REG_I2C_MST_CTRL, MPU6050_REG_I2C_SLV0_ADDR, MPU6050_REG_I2C_SLV0_REG,
        MPU6050_REG_I2C_SLV0_CTRL, MPU6050_REG_I2C_SLV1_ADDR, MPU6050_REG_I2C_SLV1_REG,
        MPU6050_REG_I2C_SLV1_CTRL, MPU6050_REG_I2C_SLV2_ADDR, MPU6050_REG_I2C_SLV2_REG,
        MPU6050_REG_I2C_SLV2_CTRL, MPU6050_REG_I2C_SLV3_ADDR, MPU6050_REG_I2C_SLV3_REG,
        MPU6050_REG_I2C_SLV3_CTRL, MPU6050_REG_I2C_SLV4_ADDR, MPU6050_REG_I2C_SLV4_REG,
        MPU6050_REG_I2C_SLV4_DO, MPU6050_REG_I2C_SLV4_CTRL, MPU6050_REG_I2C_SLV4_DI,
        MPU6050_REG_I2C_MST_STATUS, MPU6050_REG_INT_PIN_CFG, MPU6050_REG_INT_ENABLE,
        MPU6050_REG_INT_STATUS, MPU6050_REG_ACCEL_XOUT_H, MPU6050_REG_ACCEL_XOUT_L,
        MPU6050_REG_ACCEL_YOUT_H, MPU6050_REG_ACCEL_YOUT_L, MPU6050_REG_ACCEL_ZOUT_H,
        MPU6050_REG_ACCEL_ZOUT_L, MPU6050_REG_TEMP_OUT_H, MPU6050_REG_TEMP_OUT_L,
        MPU6050_REG_GYRO_XOUT_H, MPU6050_REG_GYRO_XOUT_L, MPU6050_REG_GYRO_YOUT_H,
        MPU6050_REG_GYRO_YOUT_L, MPU6050_REG_GYRO_ZOUT_H, MPU6050_REG_GYRO_ZOUT_L,
        MPU6050_REG_EXT_SENS_DATA_00, MPU6050_REG_EXT_SENS_DATA_01, MPU6050_REG_EXT_SENS_DATA_02,
        MPU6050_REG_EXT_SENS_DATA_03, MPU6050_REG_EXT_SENS_DATA_04, MPU6050_REG_EXT_SENS_DATA_05,
        MPU6050_REG_EXT_SENS_DATA_06, MPU6050_REG_EXT_SENS_DATA_07, MPU6050_REG_EXT_SENS_DATA_08,
        MPU6050_REG_EXT_SENS_DATA_09, MPU6050_REG_EXT_SENS_DATA_10, MPU6050_REG_EXT_SENS_DATA_11,
        MPU6050_REG_EXT_SENS_DATA_12, MPU6050_REG_EXT_SENS_DATA_13, MPU6050_REG_EXT_SENS_DATA_14,
        MPU6050_REG_EXT_SENS_DATA_15, MPU6050_REG_EXT_SENS_DATA_16, MPU6050_REG_EXT_SENS_DATA_17,
        MPU6050_REG_EXT_SENS_DATA_18, MPU6050_REG_EXT_SENS_DATA_19, MPU6050_REG_EXT_SENS_DATA_20,
        MPU6050_REG_EXT_SENS_DATA_21, MPU6050_REG_EXT_SENS_DATA_22, MPU6050_REG_EXT_SENS_DATA_23,
        MPU6050_REG_I2C_SLV0_DO, MPU6050_REG_I2C_SLV1_DO, MPU6050_REG_I2C_SLV2_DO,
        MPU6050_REG_I2C_SLV3_DO, MPU6050_REG_I2C_MST_DELAY_CTRL, MPU6050_REG_SIGNAL_PATH_RESET,
        MPU6050_REG_USER_CTRL, MPU6050_REG_PWR_MGMT_1, MPU6050_REG_PWR_MGMT_2,
        MPU6050_REG_FIFO_COUNTH, MPU6050_REG_FIFO_COUNTL, MPU6050_REG_FIFO_R_W,
        MPU6050_REG_WHO_AM_I,
    };

    reg_cnt = 0;
    while(reg_cnt < sizeof(reg_addr)){

        I2C_ReadByte(0x68, reg_addr[reg_cnt], &reg_value);

        Uart0_Println(PSTR("[%hhX]\t= %hhX"), reg_addr[reg_cnt], reg_value);

        reg_cnt++;
    }
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * mpu6050_IsFIFOOverflow - Function to check current MPU6050 FIFO
 *                          overflow interrupt flag.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         The FIFO is not overflowed.
 * @retval  [1]         The FIFO is overflowed.
 *
 */
#if MPU6050_IS_FIFO_EN
static int8_t mpu6050_IsFIFOOverflow()
{
    uint8_t int_status;

    I2C_ReadByte(MPU6050_DEV_ID, MPU6050_REG_INT_STATUS, &int_status);

    return (int_status & _BV(MPU6050_INT_STATUS_FIFO_OFLOW_BIT)) ? 1 : 0;
}
#endif
