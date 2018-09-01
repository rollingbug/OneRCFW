/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    i2c_drv.h
 * @brief   AVR TWI (I2C) driver functions.
 *          Support repeat start and burst read/write mode.
 *
 *          Reference:      Atmel 328P datasheet
 *          Table 22-2.     Status codes for Master Transmitter Mode.
 *          Figure 22-10.   Interfacing the Application to the TWI in a Typical
 *                          Transmission.
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef I2C_DRV_H_
#define I2C_DRV_H_

#include <stdint.h>


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

int8_t I2C_Init(uint32_t i2c_hz);
int8_t I2C_WriteBytes(uint8_t dev_id, uint8_t reg_addr,
                      uint8_t bytes, uint8_t *p_data);
int8_t I2C_WriteByte(uint8_t dev_id, uint8_t reg_addr, uint8_t data);
int8_t I2C_ReadBytes(uint8_t dev_id, uint8_t reg_addr,
                     uint8_t bytes, uint8_t *p_data);
int8_t I2C_ReadByte(uint8_t dev_id, uint8_t reg_addr, uint8_t *p_data);

#endif // I2C_DRV_H_
