/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    pin_change.cpp
 * @brief   Input pin change initialization function and interrupt handler.
 *
 *          Hardware:
 *              Pin change detector.
 *
 *          Interrupt:
 *              ISR(PCINT0_vect)
 *              ISR(PCINT1_vect)
 *              ISR(PCINT2_vect)
 *
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "pin_change.h"
#include "timers_drv.h"
#include "uart_stream.h"
#include "uart_sim.h"
#include "rc_in.h"
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

static uint8_t PC_PrevPinState[PC_GRP_TOTAL];


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
 * PC_Init - Function to initialize pin change detection functions.
 *
 * We disable all PC functions by default.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t PC_Init()
{
    /* Disable global interrupt */
    cli();

    memset((void *)PC_PrevPinState, 0, sizeof(PC_PrevPinState));

    /* Disable external interrupt request */
    EIMSK = 0x00;

    /* Clear pin change mask (Disable all PC interrupts) */
    PCMSK0 = 0;
    PCMSK1 = 0;
    PCMSK2 = 0;

    /* Disable pin change interrupt (group 0~2) */
    PCICR &= ~(_BV(PCIF0) | _BV(PCIF1) | _BV(PCIF2));

    /* Enable global interrupt */
    sei();

    Uart0_Println(PSTR("[PinChg] OK"));

    return 0;
}

/**
 * PC_Setup - Function to enable/disable specific PC pin interrupt.
 *
 * We disable all PC functions by default.
 *
 * @param   [input]     pc_grp_idx      Group index of specific PC pin.
 * @param   [input]     pc_pin_mask     Bitmask of specific PC pin.
 * @param   [input]     is_enable       Set true to enable corresponding PC interrupt.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t PC_Setup(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask, bool is_enable)
{
    volatile uint8_t pc_mask;

    /* Setup PCMSKx register */
    if(is_enable){
        PC_QuickEnable(pc_grp_idx, pc_pin_mask);
    }
    else{
        PC_QuickDisable(pc_grp_idx, pc_pin_mask);
    }

    switch(pc_grp_idx){
        case PC_GRP_IDX_0:

            pc_mask = PCMSK0;

            break;

        case PC_GRP_IDX_1:

            pc_mask = PCMSK1;

            break;

        case PC_GRP_IDX_2:

            pc_mask = PCMSK2;

            break;

        default:
            pc_mask = 0;

            break;
    }

    /* Setup PCICR register */
    if(pc_mask)
        PCICR |= _BV(pc_grp_idx);
    else
        PCICR &= ~_BV(pc_grp_idx);

    return 0;
}

/**
 * PC_QuickEnable - Function to quick enable specific PC pin interrupt.
 *
 * Please call PC_Setup() one time first for any specific PC pin.
 *
 * @param   [input]     pc_grp_idx      Group index of specific PC pin.
 * @param   [input]     pc_pin_mask     Bitmask of specific PC pin.
 *
 * @return  [none]
 *
 */
void PC_QuickEnable(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask)
{
    switch(pc_grp_idx){
        case PC_GRP_IDX_0:

            PCMSK0 |= pc_pin_mask;

            break;

        case PC_GRP_IDX_1:

            PCMSK1 |= pc_pin_mask;

            break;

        case PC_GRP_IDX_2:

            PCMSK2 |= pc_pin_mask;

            break;

        default:

            break;
    }
}

/**
 * PC_QuickDisable - Function to quick disable specific PC pin interrupt.
 *
 * Please call PC_Setup() one time first for any specific PC pin.
 *
 * @param   [input]     pc_grp_idx      Group index of specific PC pin.
 * @param   [input]     pc_pin_mask     Bitmask of specific PC pin.
 *
 * @return  [none]
 *
 */
void PC_QuickDisable(PC_GRP_IDX pc_grp_idx, uint8_t pc_pin_mask)
{
    switch(pc_grp_idx){
        case PC_GRP_IDX_0:

            PCMSK0 &= ~pc_pin_mask;

            break;

        case PC_GRP_IDX_1:

            PCMSK1 &= ~pc_pin_mask;

            break;

        case PC_GRP_IDX_2:

            PCMSK2 &= ~pc_pin_mask;

            break;

        default:
            break;
    }
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * ISR(PCINT0_vect) - Pin change group 0 ISR
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(PCINT0_vect)
{
    const PC_GRP_IDX grp_idx = PC_GRP_IDX_0;
    uint32_t trig_time = Timer1_GetTicks32();
    uint8_t pin_status = PC_GRP_0_IO_VALUE();
    uint8_t pin_change = PC_PrevPinState[grp_idx] ^ pin_status;

    DEBUG_ISR_START(PCINT0_vect_num);

    PC_PrevPinState[grp_idx] = pin_status;

    /* Be carefully, we will enable interrupt temporarily inside RCIN_PulseHandler function. */
    RCIN_PulseHandler(grp_idx, trig_time, pin_status, pin_change);

    DEBUG_ISR_END(PCINT0_vect_num);
}

/**
 * ISR(PCINT1_vect) - Pin change group 1 ISR
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(PCINT1_vect)
{
    const PC_GRP_IDX grp_idx = PC_GRP_IDX_1;
    uint32_t trig_time = Timer1_GetTicks32();
    uint8_t pin_status = PC_GRP_1_IO_VALUE();
    uint8_t pin_change = PC_PrevPinState[grp_idx] ^ pin_status;

    DEBUG_ISR_START(PCINT1_vect_num);

    PC_PrevPinState[grp_idx] = pin_status;

    /*
     * void handler(grp_idx, grp_shift, trig_time, pin_change)
     * {
     * }
     */

    DEBUG_ISR_END(PCINT1_vect_num);
}

/**
 * ISR(PCINT2_vect) - Pin change group 2 ISR
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(PCINT2_vect)
{
    const PC_GRP_IDX grp_idx = PC_GRP_IDX_2;
    uint32_t trig_time = Timer1_GetTicks32();
    uint8_t pin_status = PC_GRP_2_IO_VALUE();
    uint8_t pin_change = PC_PrevPinState[grp_idx] ^ pin_status;

    DEBUG_ISR_START(PCINT2_vect_num);

    PC_PrevPinState[grp_idx] = pin_status;

#if UARTS_FUNCTION_EN
    /* Check simulated UART RX pin */
    UartS_RxPulseHandler(grp_idx, trig_time, pin_status, pin_change);
#endif

    /* Be carefully, we will enable interrupt temporarily inside RCIN_PulseHandler function. */
    RCIN_PulseHandler(grp_idx, trig_time, pin_status, pin_change);

    DEBUG_ISR_END(PCINT2_vect_num);
}

