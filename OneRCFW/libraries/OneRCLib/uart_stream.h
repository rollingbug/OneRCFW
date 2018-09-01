/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    uart_stream.cpp
 * @brief   AVR UART stream functions.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef UART_STREAM_H_
#define UART_STREAM_H_

#include <stdint.h>
#include <avr/pgmspace.h>

#include "uart_drv.h"


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

void Uart0_PrintChr(char chr);
void Uart0_PrintStr(const char *p_str);
void Uart0_PrintUnsigned(uint32_t value, uint8_t base);
void Uart0_PrintSigned(int32_t value);
void Uart0_PrintFloat(float value, uint8_t digits);
void Uart0_Printf(const char *p_fmt, ...);
void Uart0_Println(const char *p_fmt, ...);

void UartS_PrintChr(char chr);
void UartS_PrintStr(const char *p_str);
void UartS_PrintUnsigned(uint32_t value, uint8_t base);
void UartS_PrintSigned(int32_t value);
void UartS_PrintFloat(float value, uint8_t digits);
void UartS_Printf(const char *p_fmt, ...);
void UartS_Println(const char *p_fmt, ...);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // UART_STREAM_H_

