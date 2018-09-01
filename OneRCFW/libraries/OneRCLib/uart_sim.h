/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    uart_sim.cpp
 * @brief   Hardware/software simulated UART functions
 *          (based on timer 1 input capturing mechanism, only support baud 9600)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef UART_SIM_H_
#define UART_SIM_H_

#include <Arduino.h>
#include <stdint.h>

#include "pin_change.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define UARTS_FUNCTION_EN   true


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

int8_t UartS_Init(uint32_t baud_rate);
uint8_t UartS_ReadBytes(uint8_t *p_data, uint8_t bytes);
uint8_t UartS_ReadByte(uint8_t *p_data);
uint8_t UartS_ReadAvailable();
uint8_t UartS_WriteBytes(uint8_t *p_data, uint8_t bytes);
uint8_t UartS_WriteBytesNB(uint8_t *p_data, uint8_t bytes);
uint8_t UartS_WriteByte(uint8_t data);
uint8_t UartS_WriteByteNB(uint8_t data);
void UartS_RxPulseHandler(PC_GRP_IDX pc_grp_idx, uint32_t trig_time,
                          uint8_t pin_status, uint8_t pin_change);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // UART_SIM_H_

