/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    i2c_drv.cpp
 * @brief   AVR TWI (I2C) driver functions.
 *          Support repeat start and burst read/write mode.
 *
 *          Reference:      Atmel 328P datasheet
 *          Table 22-2.     Status codes for Master Transmitter Mode.
 *          Figure 22-10.   Interfacing the Application to the TWI in a Typical
 *                          Transmission.
 *
 *          Hardware:
 *              TWI (I2C)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <util/twi.h>
#include <avr/interrupt.h>

#include "i2c_drv.h"
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
 * Public functions declaration
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */

static int8_t I2C_SetCtrlAndWait(uint8_t ctrl_reg);
static int8_t I2C_StartTx(uint8_t dev_id);
static int8_t I2C_StopTx();
static int8_t I2C_SelectReg(uint8_t reg_addr);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * I2C_Init - Function to initialize AVR TWI (I2C) HW component
 *
 * @param   [in]        i2c_hz      Frequency setting of I2C SCL.
 *                                  (Inputting 100000 represent 100 KHz)
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t I2C_Init(uint32_t i2c_hz)
{
    if(i2c_hz == 0)
        return -1;

    /* Disable all interrupts */
    cli();

    /* SCL frequency = CPU clock / (16 + 2 * (TWBR) * Prescaler) */
    TWCR = 0;                                       /* Disable I2C */
    TWSR = 0;                                       /* Bit0:1, prescaler = 1 */
    TWBR = (uint8_t)(((F_CPU / i2c_hz) - 16) / 2);  /* SCL Division factor */
    TWCR = _BV(TWEN);                               /* Enable I2C */

    /* Enable all interrupts */
    sei();

    Uart0_Println(PSTR("[I2C] Rate: %u"), i2c_hz);

    return 0;
}

/**
 * I2C_WriteBytes - Send multi bytes data to specific sensor register
 *                  through I2C interface in burst writing mode.
 *
 * Please notice the actual behavior of "burst writing mode" may be
 * different depend on how the sensor vendor implement the interface
 * for "burst write", the HW sensor may or may not increase the register
 * automatically for different scenario.
 *
 * @param   [in]        dev_id      Device ID of sensor.
 * @param   [in]        reg_addr    The address of specific sensor register.
 * @param   [in]        bytes       Total bytes to write.
 * @param   [in]        *p_data     The actual writing data.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 */
int8_t I2C_WriteBytes(uint8_t dev_id, uint8_t reg_addr,
                      uint8_t bytes, uint8_t *p_data)
{
    int8_t result;
    uint8_t *p_in;

    /* Send START signal and device address with WRITE flag */
    result = I2C_StartTx((dev_id << 1));
    if(result == -1)
        goto I2C_STOP;

    /* Specify device register */
    result = I2C_SelectReg(reg_addr);
    if(result == -1)
        goto I2C_STOP;

    /* Start burst write */
    p_in = p_data;

    while(bytes){

        TWDR = *p_in++;
        result = I2C_SetCtrlAndWait((_BV(TWINT) | (_BV(TWEN))));
        if(result == -1 || TW_STATUS != TW_MT_DATA_ACK){
            result = -1;
            goto I2C_STOP;
        }

        bytes--;
    }

I2C_STOP:

    /* Send STOP signal */
    I2C_StopTx();

    return result;
}

/**
 * I2C_WriteByte - Send single byte data to specific sensor register
 *                 through I2C interface.
 *
 * @param   [in]        dev_id      Device ID of sensor.
 * @param   [in]        reg_addr    The address of specific sensor register.
 * @param   [in]        data        The actual writing data.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t I2C_WriteByte(uint8_t dev_id, uint8_t reg_addr, uint8_t data)
{
    return I2C_WriteBytes(dev_id, reg_addr, 1, &data);
}

/**
 * I2C_ReadBytes -  Read multi bytes data from specific sensor register
 *                  through I2C interface in burst writing mode.
 *
 * Please notice the actual behavior of "burst reading mode" may be
 * different depend on how the sensor vendor implement the interface
 * for "burst read", the HW sensor may or may not increase the register
 * automatically for different scenario.
 *
 * @param   [in]        dev_id      Device ID of sensor.
 * @param   [in]        reg_addr    The address of specific sensor register.
 * @param   [in]        bytes       Total bytes to read.
 * @param   [out]       *p_data     Buffer for storing reading data.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 */
int8_t I2C_ReadBytes(uint8_t dev_id, uint8_t reg_addr,
                     uint8_t bytes, uint8_t *p_data)
{
    int8_t result;
    uint8_t *p_out;

    /* Send START signal and device address with WRITE flag */
    result = I2C_StartTx(dev_id << 1);
    if(result == -1)
        goto I2C_STOP;

    /* Specify device register */
    result = I2C_SelectReg(reg_addr);
    if(result == -1)
        goto I2C_STOP;

    /* Send repeated START signal and send device address with READ flag */
    result = I2C_StartTx((dev_id << 1) | 0x01);
    if(result == -1)
        goto I2C_STOP;

    /* Start burst read */
    p_out = p_data;
    while(--bytes){

        /* Wait data and reply ACK */
        result = I2C_SetCtrlAndWait((_BV(TWINT) | _BV(TWEN) | _BV(TWEA)));
        if(result == -1 || TW_STATUS != TW_MR_DATA_ACK){
            result = -1;
            goto I2C_STOP;
        }

        *p_out++ = TWDR;
    }

    /* Wait for last data and reply NACK */
    result = I2C_SetCtrlAndWait((_BV(TWINT) | _BV(TWEN)));
    if(result == -1 || TW_STATUS != TW_MR_DATA_NACK)
        goto I2C_STOP;

    *p_out = TWDR;

I2C_STOP:

    /* Send STOP signal */
    I2C_StopTx();

    return result;
}

/**
 * I2C_ReadByte - Read single byte data to specific sensor register
 *                through I2C interface.
 *
 * @param   [in]        dev_id      Device ID of sensor.
 * @param   [in]        reg_addr    The address of specific sensor register.
 * @param   [out]       *p_data     Buffer for storing reading data (1 Byte).
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t I2C_ReadByte(uint8_t dev_id, uint8_t reg_addr, uint8_t *p_data)
{
    return I2C_ReadBytes(dev_id, reg_addr, 1, p_data);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * I2C_SetCtrlAndWait - Function to setup AVR TWI control register for enabling
 *                      specific I2C function and protocol, and polling the status
 *                      till interrupt has trigged or timeout has expired.
 *
 * @param   [in]        ctrl_reg    The actual setting will be written to control
 *                                  register.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         The expected function has be executed successfully.
 * @retval  [-1]        Fail, timeout has expired.
 *
 */
static int8_t I2C_SetCtrlAndWait(uint8_t ctrl_reg)
{
    uint8_t timeout;

    timeout = 0xFF;

    TWCR = ctrl_reg;
    while(!(TWCR & _BV(TWINT))){

        timeout--;
        if(timeout == 0){
            TWCR = 0;
            return -1;
        }
    }

    return 0;
}

/**
 * I2C_StartTx - Function to setup AVR TWI component for transmitting I2C
 *               (repeated) start signal and transmit a frame contains device
 *               ID with R/W bit.
 *
 * @param   [in]        dev_id  Device ID of sensor that we want to communicate.
 *                              [Bit 7:1]   Device ID
 *                              [Bit 0]     R/W flag, set to 1 for reading.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t I2C_StartTx(uint8_t dev_id)
{
    int8_t result;

    /* Send (repeated) START signal */
    result = I2C_SetCtrlAndWait((_BV(TWINT) | _BV(TWSTA) | _BV(TWEN)));
    if(result == -1 || (TW_STATUS != TW_START && TW_STATUS != TW_REP_START))
        return -1;

    /* Send device ID with read/write bit and check the status of ACK */
    TWDR = dev_id;
    result = I2C_SetCtrlAndWait((_BV(TWINT) | _BV(TWEN)));
    if(result == -1 || (TW_STATUS != TW_MT_SLA_ACK && TW_STATUS != TW_MR_SLA_ACK))
        return -1;

    return 0;
}

/**
 * I2C_StopTx - Function to setup AVR TWI component for transmitting I2C
 *              stop signal.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t I2C_StopTx()
{
    /* Send STOP signal */
    TWCR = (_BV(TWINT) | _BV(TWEN) | _BV(TWSTO));

    return 0;
}

/**
 * I2C_SelectReg - Function to setup AVR TWI component for transmitting I2C
 *                 frame contains register address which we wan to read or write.
 *
 *                 This function should be called after I2C_StopTx function.
 *
 * @param   [in]        reg_addr    The address of specific sensor register.
 *
 * @return  [uint8_t]   Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 */
static int8_t I2C_SelectReg(uint8_t reg_addr)
{
    int8_t result;

    /* Send register address */
    TWDR = reg_addr;

    result = I2C_SetCtrlAndWait((_BV(TWINT) | (_BV(TWEN))));
    if(result == -1 || TW_STATUS != TW_MT_DATA_ACK)
        return -1;

    return 0;
}
