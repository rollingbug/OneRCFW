/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    failsafe.cpp
 * @brief
 *
 *          Hardware:
 *              WDT
 *
 *          Interrupt:
 *              ISR(WDT_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>

#include "failsafe.h"
#include "rc_in.h"
#include "uart_stream.h"
#include "debug.h"


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


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * FailSafe_Init - Function to initialize failsafe(watchdog) function.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t FailSafe_Init()
{
    cli();
    wdt_reset();

    MCUSR &= ~_BV(WDRF);

    /*
     * Clear WDT interrupt flag, enable WDT change flag,
     * enable WDT interrupt, set to 250ms cycle
     */
    WDTCSR = (_BV(WDCE) | _BV(WDE) | _BV(WDIF));
    WDTCSR = (_BV(WDIE) | _BV(WDP2));

    sei();

    Uart0_Println(PSTR("[FSafe] WDT: 250ms"));

    return 0;
}

/**
 * FailSafe_Reboot - Function to trig hardware system reset via watchdog.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
void FailSafe_Reboot()
{
    /* Print reset warning before performing system reset*/
    Uart0_Println(PSTR("[FSafe] WDT: rebooting ..."));

    cli();
    wdt_reset();

    /*
     * Set watchdog change enable flag,
     * Change WDT timeout to 1 second, and enable system reset mode.
     */
    WDTCSR = (_BV(WDCE) | _BV(WDE) | _BV(WDIF));
    WDTCSR = (_BV(WDE) | _BV(WDP1) | _BV(WDP2));

    /* Wait watchdog trigger and performing HW reset*/
    while(1)
        ;

    /* Should never reach here*/
    sei();
}

/**
 * ISR(WDT_vect) - Watchdog ISR
 *
 * This function will be called per 250 ms.
 * Requires at least 14 us.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(WDT_vect)
{
    DEBUG_ISR_START(WDT_vect_num);

    /* Be careful, we will enable interrupt temporarily inside RCIN_FailChk function. */
    RCIN_FailChk();

    DEBUG_ISR_END(WDT_vect_num);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

