/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    mpu6050_drv.h
 *
 * @brief   Driver of MPU6050.
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef MPU6050_DRV_H_
#define MPU6050_DRV_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* MPU6050 I2C device ID */
#define MPU6050_DEV_ID                      0x68

/* MPU6050 accelerometer and gyroscope resolution definition */
#define MPU6050_UNIT_1G                     (4096)  /* +-8 G by default */
#define MPU6050_UNIT_1DPS                   (16.4)  /* +-2000 DPS by default */

/* MPU6050 FIFO setting */
#define MPU6050_IS_FIFO_EN                  0

/* MPU6050 FIFO sampling rate, unit: Hz */
#define MPU6050_FIFO_SAMPLE_RATE            800

/* MPU6050 FIFO sampling setting, (8 KHz / N Hz) - 1 */
#define MPU6050_FIFO_SAMPLE_SETTING         ((8000 / MPU6050_FIFO_SAMPLE_RATE) - 1)

/* MPU6050 FIFO sampling period, unit: microseconds, 1000000 / X Hz = Y ms */
#define MPU6050_FIFO_SAMPLE_PERIOD          (1000000 / MPU6050_FIFO_SAMPLE_RATE)


/*
 *******************************************************************************
 * Constant value definition (MPU6050 registers address)
 *******************************************************************************
 */
enum MPU6050_REG{
    MPU6050_REG_SELF_TEST_X                 = 0x0D,     /* R/W  */
    MPU6050_REG_SELF_TEST_Y                 = 0x0E,     /* R/W  */
    MPU6050_REG_SELF_TEST_Z                 = 0x0F,     /* R/W  */
    MPU6050_REG_SELF_TEST_A                 = 0x10,     /* R/W  */
    MPU6050_REG_SMPLRT_DIV                  = 0x19,     /* R/W  */
    MPU6050_REG_CONFIG                      = 0x1A,     /* R/W  */
    MPU6050_REG_GYRO_CONFIG                 = 0x1B,     /* R/W  */
    MPU6050_REG_ACCEL_CONFIG                = 0x1C,     /* R/W  */
    MPU6050_REG_FIFO_EN                     = 0x23,     /* R/W  */
    MPU6050_REG_I2C_MST_CTRL                = 0x24,     /* R/W  */
    MPU6050_REG_I2C_SLV0_ADDR               = 0x25,     /* R/W  */
    MPU6050_REG_I2C_SLV0_REG                = 0x26,     /* R/W  */
    MPU6050_REG_I2C_SLV0_CTRL               = 0x27,     /* R/W  */
    MPU6050_REG_I2C_SLV1_ADDR               = 0x28,     /* R/W  */
    MPU6050_REG_I2C_SLV1_REG                = 0x29,     /* R/W  */
    MPU6050_REG_I2C_SLV1_CTRL               = 0x2A,     /* R/W  */
    MPU6050_REG_I2C_SLV2_ADDR               = 0x2B,     /* R/W  */
    MPU6050_REG_I2C_SLV2_REG                = 0x2C,     /* R/W  */
    MPU6050_REG_I2C_SLV2_CTRL               = 0x2D,     /* R/W  */
    MPU6050_REG_I2C_SLV3_ADDR               = 0x2E,     /* R/W  */
    MPU6050_REG_I2C_SLV3_REG                = 0x2F,     /* R/W  */
    MPU6050_REG_I2C_SLV3_CTRL               = 0x30,     /* R/W  */
    MPU6050_REG_I2C_SLV4_ADDR               = 0x31,     /* R/W  */
    MPU6050_REG_I2C_SLV4_REG                = 0x32,     /* R/W  */
    MPU6050_REG_I2C_SLV4_DO                 = 0x33,     /* R/W  */
    MPU6050_REG_I2C_SLV4_CTRL               = 0x34,     /* R/W  */
    MPU6050_REG_I2C_SLV4_DI                 = 0x35,     /* R    */
    MPU6050_REG_I2C_MST_STATUS              = 0x36,     /* R    */
    MPU6050_REG_INT_PIN_CFG                 = 0x37,     /* R/W  */
    MPU6050_REG_INT_ENABLE                  = 0x38,     /* R/W  */
    MPU6050_REG_INT_STATUS                  = 0x3A,     /* R    */
    MPU6050_REG_ACCEL_XOUT_H                = 0x3B,     /* R    */
    MPU6050_REG_ACCEL_XOUT_L                = 0x3C,     /* R    */
    MPU6050_REG_ACCEL_YOUT_H                = 0x3D,     /* R    */
    MPU6050_REG_ACCEL_YOUT_L                = 0x3E,     /* R    */
    MPU6050_REG_ACCEL_ZOUT_H                = 0x3F,     /* R    */
    MPU6050_REG_ACCEL_ZOUT_L                = 0x40,     /* R    */
    MPU6050_REG_TEMP_OUT_H                  = 0x41,     /* R    */
    MPU6050_REG_TEMP_OUT_L                  = 0x42,     /* R    */
    MPU6050_REG_GYRO_XOUT_H                 = 0x43,     /* R    */
    MPU6050_REG_GYRO_XOUT_L                 = 0x44,     /* R    */
    MPU6050_REG_GYRO_YOUT_H                 = 0x45,     /* R    */
    MPU6050_REG_GYRO_YOUT_L                 = 0x46,     /* R    */
    MPU6050_REG_GYRO_ZOUT_H                 = 0x47,     /* R    */
    MPU6050_REG_GYRO_ZOUT_L                 = 0x48,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_00            = 0x49,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_01            = 0x4A,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_02            = 0x4B,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_03            = 0x4C,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_04            = 0x4D,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_05            = 0x4E,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_06            = 0x4F,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_07            = 0x50,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_08            = 0x51,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_09            = 0x52,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_10            = 0x53,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_11            = 0x54,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_12            = 0x55,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_13            = 0x56,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_14            = 0x57,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_15            = 0x58,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_16            = 0x59,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_17            = 0x5A,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_18            = 0x5B,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_19            = 0x5C,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_20            = 0x5D,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_21            = 0x5E,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_22            = 0x5F,     /* R    */
    MPU6050_REG_EXT_SENS_DATA_23            = 0x60,     /* R    */
    MPU6050_REG_I2C_SLV0_DO                 = 0x63,     /* R/W  */
    MPU6050_REG_I2C_SLV1_DO                 = 0x64,     /* R/W  */
    MPU6050_REG_I2C_SLV2_DO                 = 0x65,     /* R/W  */
    MPU6050_REG_I2C_SLV3_DO                 = 0x66,     /* R/W  */
    MPU6050_REG_I2C_MST_DELAY_CTRL          = 0x67,     /* R/W  */
    MPU6050_REG_SIGNAL_PATH_RESET           = 0x68,     /* R/W  */
    MPU6050_REG_USER_CTRL                   = 0x6A,     /* R/W  */
    MPU6050_REG_PWR_MGMT_1                  = 0x6B,     /* R/W  */
    MPU6050_REG_PWR_MGMT_2                  = 0x6C,     /* R/W  */
    MPU6050_REG_FIFO_COUNTH                 = 0x72,     /* R/W  */
    MPU6050_REG_FIFO_COUNTL                 = 0x73,     /* R/W  */
    MPU6050_REG_FIFO_R_W                    = 0x74,     /* R/W  */
    MPU6050_REG_WHO_AM_I                    = 0x75,     /* R    */
};


/*
 *******************************************************************************
 * Constant value definition (MPU6050 registers bit fields)
 *******************************************************************************
 */

/* 0x19, MPU6050_REG_SMPLRT_DIV */
enum MPU6050_SMPLRT_DIV_BITS{
    MPU6050_SMPLRT_DIV_SMPLRT_DIV0_BIT      = 0x00,     /* SMPLRT_DIV[7:0], 8-bit unsigned value. */
    MPU6050_SMPLRT_DIV_SMPLRT_DIV1_BIT      = 0x01,     /* The Sample Rate is determined by dividing the gyroscope output rate by this value. */
    MPU6050_SMPLRT_DIV_SMPLRT_DIV2_BIT      = 0x02,     /* Sample Rate = Gyroscope Output Rate / (1 + SMPLRT_DIV) */
    MPU6050_SMPLRT_DIV_SMPLRT_DIV3_BIT      = 0x03,     /* where Gyroscope Output Rate = 8kHz when the DLPF is disabled (DLPF_CFG = 0 or 7), */
    MPU6050_SMPLRT_DIV_SMPLRT_DIV4_BIT      = 0x04,     /* 1kHz when the DLPF is enabled (see Register 26). */
    MPU6050_SMPLRT_DIV_SMPLRT_DIV5_BIT      = 0x05,
    MPU6050_SMPLRT_DIV_SMPLRT_DIV6_BIT      = 0x06,
    MPU6050_SMPLRT_DIV_SMPLRT_DIV7_BIT      = 0x07,
};

/* 0x1A, MPU6050_REG_CONFIG */
enum MPU6050_CONFIG_BITS{
    MPU6050_CONFIG_DLPF_CFG0_BIT            = 0x00,     /* 3-bit unsigned value. Configures the DLPF setting. */
    MPU6050_CONFIG_DLPF_CFG1_BIT            = 0x01,     /* DLPF_CFG[2:0] */
    MPU6050_CONFIG_DLPF_CFG2_BIT            = 0x02,     /* DLPF_CFG[2:0] */
    MPU6050_CONFIG_EXT_SYNC_SET0_BIT        = 0x03,     /* 3-bit unsigned value. Configures the FSYNC pin sampling. */
    MPU6050_CONFIG_EXT_SYNC_SET1_BIT        = 0x04,     /* EXT_SYNC_SET[2:0], 0:disabled, 1:TEMP_OUT_L[0], 2:GYRO_XOUT_L[0], 3:GYRO_YOUT_L[0] */
    MPU6050_CONFIG_EXT_SYNC_SET2_BIT        = 0x05,     /* 4:GYRO_ZOUT_L[0], 5:ACCEL_XOUT_L[0], 6:ACCEL_YOUT_L[0], 7:ACCEL_ZOUT_L[0] */
    MPU6050_CONFIG_RESERVED1_BIT            = 0x06,
    MPU6050_CONFIG_RESERVED2_BIT            = 0x07,
};

/* 0x1B, MPU6050_REG_GYRO_CONFIG */
enum MPU6050_GYRO_CONFIG_BITS{
    MPU6050_GYRO_CONFIG_RESERVED1_BIT       = 0x00,
    MPU6050_GYRO_CONFIG_RESERVED2_BIT       = 0x01,
    MPU6050_GYRO_CONFIG_RESERVED3_BIT       = 0x02,
    MPU6050_GYRO_CONFIG_FS_SEL0_BIT         = 0x03,     /* The full scale range of the gyroscope outputs */
    MPU6050_GYRO_CONFIG_FS_SEL1_BIT         = 0x04,     /* FS_SEL[1:0], 0:+-250, 1:+-500, 2:+-1000, 3:+-2000 (dps) */
    MPU6050_GYRO_CONFIG_ZG_ST_BIT           = 0x05,     /* Z axis gyroscope self test. */
    MPU6050_GYRO_CONFIG_YG_ST_BIT           = 0x06,     /* Y axis gyroscope self test. */
    MPU6050_GYRO_CONFIG_XG_ST_BIT           = 0x07,     /* X axis gyroscope self test. */
};

/* 0x1C, MPU6050_REG_ACCEL_CONFIG */
enum MPU6050_ACCEL_CONFIG_BITS{
    MPU6050_ACCEL_CONFIG_RESERVED1_BIT      = 0x00,
    MPU6050_ACCEL_CONFIG_RESERVED2_BIT      = 0x01,
    MPU6050_ACCEL_CONFIG_RESERVED3_BIT      = 0x02,
    MPU6050_ACCEL_CONFIG_AFS_SEL0_BIT       = 0x03,     /* The full scale range of the accelerometer outputs */
    MPU6050_ACCEL_CONFIG_AFS_SEL1_BIT       = 0x04,     /* AFS_SEL[1:0], 0:+-2g, 1:+-4g, 2:+-8g, 3:+-16g */
    MPU6050_ACCEL_CONFIG_ZA_ST_BIT          = 0x05,     /* Z axis accelerometer self test. */
    MPU6050_ACCEL_CONFIG_YA_ST_BIT          = 0x06,     /* Y axis accelerometer self test. */
    MPU6050_ACCEL_CONFIG_XA_ST_BIT          = 0x07,     /* X axis accelerometer self test. */
};

/* 0x23, MPU6050_REG_FIFO_EN */
enum MPU6050_REG_FIFO_EN_BITS{
    MPU6050_FIFO_EN_SLV0_BIT                = 0x00,     /* When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96) associated with Slave 0 to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_SLV1_BIT                = 0x01,     /* When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96) associated with Slave 1 to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_SLV2_BIT                = 0x02,     /* When set to 1, this bit enables EXT_SENS_DATA registers (Registers 73 to 96) associated with Slave 2 to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_ACCEL_BIT               = 0x03,     /* When set to 1, this bit enables ACCEL_XOUT_H, ACCEL_XOUT_L, ACCEL_YOUT_H, ACCEL_YOUT_L, ACCEL_ZOUT_H, and ACCEL_ZOUT_L (Registers 59 to 64) to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_ZG_BIT                  = 0x04,     /* When set to 1, this bit enables GYRO_ZOUT_H and GYRO_ZOUT_L (Registers 71 and 72) to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_YG_BIT                  = 0x05,     /* When set to 1, this bit enables GYRO_YOUT_H and GYRO_YOUT_L (Registers 69 and 70) to be written into the FIFO buffer. */
    MPU6050_FIFO_EN_XG_BIT                  = 0x06,     /* When set to 1, this bit enables GYRO_XOUT_H and GYRO_XOUT_L (Registers 67 and 68) to be written into the FIFO buffer. */
    MPU6050_FIFO_TEMP_BIT                   = 0x07,     /* When set to 1, this bit enables TEMP_OUT_H and TEMP_OUT_L (Registers 65 and 66) to be written into the FIFO buffer. */
};

/* 0x38, MPU6050_REG_INT_ENABLE */
enum MPU6050_REG_INT_ENABLE_BITS{
    MPU6050_INT_ENABLE_DATA_RDY_BIT         = 0x00,     /* When set to 1, this bit enables the Data Ready interrupt, which occurs each time a write operation to all of the sensor registers has been completed. */
    MPU6050_INT_ENABLE_RESERVED1_BIT        = 0x01,
    MPU6050_INT_ENABLE_RESERVED2_BIT        = 0x02,
    MPU6050_INT_ENABLE_I2C_MST_INT_BIT      = 0x03,     /* When set to 1, this bit enables any of the I2C Master interrupt sources to generate an interrupt. */
    MPU6050_INT_ENABLE_FIFO_OFLOW_BIT       = 0x04,     /* When set to 1, this bit enables a FIFO buffer overflow to generate an interrupt. */
    MPU6050_INT_ENABLE_RESERVED3_BIT        = 0x05,
    MPU6050_INT_ENABLE_RESERVED4_BIT        = 0x06,
    MPU6050_INT_ENABLE_RESERVED5_BIT        = 0x07,
};

/* 0x3A, MPU6050_REG_INT_STATUS */
enum MPU6050_REG_INT_STATUS_BITS{
    MPU6050_INT_STATUS_DATA_RDY_BIT         = 0x00,     /* This bit automatically sets to 1 when a Data Ready interrupt is generated. */
    MPU6050_INT_STATUS_RESERVED1_BIT        = 0x01,
    MPU6050_INT_STATUS_RESERVED2_BIT        = 0x02,
    MPU6050_INT_STATUS_I2C_MST_INT_BIT      = 0x03,     /* This bit automatically sets to 1 when an I2C Master interrupt has been generated. */
    MPU6050_INT_STATUS_FIFO_OFLOW_BIT       = 0x04,     /* This bit automatically sets to 1 when a FIFO buffer overflow interrupt has been generated. */
    MPU6050_INT_STATUS_RESERVED3_BIT        = 0x05,
    MPU6050_INT_STATUS_RESERVED4_BIT        = 0x06,
    MPU6050_INT_STATUS_RESERVED5_BIT        = 0x07,
};

/* 0x6A, MPU6050_REG_USER_CTRL */
enum MPU6050_USER_CTRL_BITS{
    MPU6050_USER_CTRL_SIG_COND_RESET_BIT    = 0x00,
    MPU6050_USER_CTRL_I2C_MST_RESET_BIT     = 0x01,
    MPU6050_USER_CTRL_FIFO_RESET_BIT        = 0x02,
    MPU6050_USER_CTRL_RESERVED2_BIT         = 0x03,
    MPU6050_USER_CTRL_I2C_IF_DIS_BIT        = 0x04,
    MPU6050_USER_CTRL_I2C_MST_EN_BIT        = 0x05,
    MPU6050_USER_CTRL_FIFO_EN_BIT           = 0x06,
    MPU6050_USER_CTRL_RESERVED1_BIT         = 0x07,
};

/* 0x6B, MPU6050_REG_PWR_MGMT_1 */
enum MPU6050_PWR_MGMT_1_BITS{
    MPU6050_PWR_MGMT_1_CLK0_BIT             = 0x00,
    MPU6050_PWR_MGMT_1_CLK1_BIT             = 0x01,
    MPU6050_PWR_MGMT_1_CLK2_BIT             = 0x02,
    MPU6050_PWR_MGMT_1_TEMP_DIS_BIT         = 0x03,
    MPU6050_PWR_MGMT_1_RESERVED1_BIT        = 0x04,
    MPU6050_PWR_MGMT_1_CYCLE_BIT            = 0x05,
    MPU6050_PWR_MGMT_1_SLEEP_BIT            = 0x06,
    MPU6050_PWR_MGMT_1_RESET_BIT            = 0x07,
};

/* 0x75, MPU6050_REG_WHO_AM_I */
enum MPU6050_REG_WHO_AM_I_BITS{
    MPU6050_WHO_AM_I_RESERVED1_BIT          = 0x00,
    MPU6050_WHO_AM_I_0_BIT                  = 0x01,     /* Contains the 6-bit I2C address of the MPU-60X0. */
    MPU6050_WHO_AM_I_1_BIT                  = 0x02,     /* The Power-On-Reset value of Bit6:Bit1 is 110 100. */
    MPU6050_WHO_AM_I_2_BIT                  = 0x03,
    MPU6050_WHO_AM_I_3_BIT                  = 0x04,
    MPU6050_WHO_AM_I_4_BIT                  = 0x05,
    MPU6050_WHO_AM_I_5_BIT                  = 0x06,
    MPU6050_WHO_AM_I_RESERVED2_BIT          = 0x07,
};


/*
 *******************************************************************************
 * Constant value definition (MPU6050 setting value)
 *******************************************************************************
 */

/* Digital Low Pass Filter (DLPF) setting for both the gyroscopes and accelerometers */
enum MPU6050_DLPF_CFG_VALUE{
    MPU6050_DLPF_CFG_256HZ                  = 0x00,
    MPU6050_DLPF_CFG_188HZ                  = 0x01,
    MPU6050_DLPF_CFG_98HZ                   = 0x02,
    MPU6050_DLPF_CFG_42HZ                   = 0x03,
    MPU6050_DLPF_CFG_20HZ                   = 0x04,
    MPU6050_DLPF_CFG_10HZ                   = 0x05,
    MPU6050_DLPF_CFG_5HZ                    = 0x06,
    MPU6050_DLPF_CFG_RESERVED               = 0x07,
};


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */

int8_t mpu6050_Init();
int8_t mpu6050_Calibration(int16_t *p_accel_bias, int16_t *p_gyro_bias);
int8_t mpu6050_SetBais(int16_t *p_accel_bias, int16_t *p_gyro_bias);
int8_t mpu6050_ReadXYZFromFIFO(int16_t *p_accel_xyz, int16_t *p_gyro_xyz);
int8_t mpu6050_ReadXYZDirectly(int16_t *p_accel_xyz, int16_t *p_gyro_xyz);


#endif // MPU6050_DRV_H_
