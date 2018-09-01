/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    debug.cpp
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#include <Arduino.h>
#include <stdint.h>
#include <avr/interrupt.h>

#include "timers_drv.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define DEBUG_ISR_ENABLE    false

extern uint8_t Debug_ISR_NestCnt;


#if DEBUG_ISR_ENABLE

#define DEBUG_ISR_START(vector_num)             \
        do{                                     \
                Debug_ISR_NestCnt++;            \
                PORTC |= _BV(PORTC0);           \
        }while(0)

#define DEBUG_ISR_END(vector_num)               \
        do{                                     \
            Debug_ISR_NestCnt--;                \
            if(Debug_ISR_NestCnt == 0)          \
                PORTC &= ~_BV(PORTC0);          \
        }while(0)

#else

#define DEBUG_ISR_START(vector_num)
#define DEBUG_ISR_END(vector_num)

#endif // #if DEBUG_ISR_ENABLE

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


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * function_example - Function example
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [int]   Function executing result.
 * @retval  [0]     Success.
 * @retval  [-1]    Fail.
 *
 */


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // DEBUG_H_
