/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    leds_ctrl.cpp
 * @brief   LED controlling functions.
 *
 *          LED/Pin/ mapping:
 *              Master LED = Arduino D13 / AVR PB5
 *              Slave LED = Arduino D12 / AVR PB4
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>
#include <avr/io.h>

#include "leds_ctrl.h"
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

typedef enum led_state{
    LED_NORMAL_OFF_STATE   = 0,
    LED_NORMAL_ON_STATE,
    LED_LIGHTING_ON_STATE,
    LED_LIGHTING_OFF_STATE,
}LED_STATE;

typedef struct led{
    uint8_t ardu_pin;
    volatile uint8_t *p_pin_reg;
    uint8_t reg_bit;
    LED_STATE led_state;
    uint32_t prev_time;
}LED;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

LED LEDS_Data[LEDS_TOTAL] =
{
    [LEDS_MASTER_IDX] = {13, &PORTB, _BV(PORTB5), LED_NORMAL_OFF_STATE, 0}, /* Master, Arduino pin 13, PB5 */
    [LEDS_SLAVE_IDX] = {14, &PORTC, _BV(PORTC0), LED_NORMAL_OFF_STATE, 0},  /* Slave, Arduino pin 14(A0), PB4 */
};


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */

/**
 * LEDS_Init - Function to initialize LED pins and power OFF LED by default.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
void LEDS_Init()
{
    uint8_t led_idx;

    Uart0_Printf(PSTR("[LEDS] Pins:"));

    /* Initial ports */
    for(led_idx = 0; led_idx < LEDS_TOTAL; led_idx++){
        pinMode(LEDS_Data[led_idx].ardu_pin, OUTPUT);
        LEDS_PwrOFF(led_idx);
        LEDS_Data[led_idx].prev_time = Timer1_GetMillis();

        Uart0_Printf(PSTR(" %hhu"), LEDS_Data[led_idx].ardu_pin);
    }

    Uart0_Println(PSTR(""));
}

/**
 * LEDS_PwrON - Function to power ON specific LED.
 *
 * @param   [in]        led_idx     Index of specific LED.
 *                                  Refer to enum LEDS_IDX{...}.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t LEDS_PwrON(uint8_t led_idx)
{
    if(led_idx >= LEDS_TOTAL)
        return -1;

    *(LEDS_Data[led_idx].p_pin_reg) |= LEDS_Data[led_idx].reg_bit;

    return 0;
}

/**
 * LEDS_PwrOFF - Function to power OFF specific LED.
 *
 * @param   [in]        led_idx     Index of specific LED.
 *                                  Refer to enum LEDS_IDX{...}.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t LEDS_PwrOFF(uint8_t led_idx)
{
    if(led_idx >= LEDS_TOTAL)
        return -1;

    *(LEDS_Data[led_idx].p_pin_reg) &= ~(LEDS_Data[led_idx].reg_bit);

    return 0;
}

/**
 * LEDS_Blink - Function to blink specific LED.
 *
 * @param   [in]        led_idx     Index of specific LED.
 *                                  Refer to enum LEDS_IDX{...}.
 *
 * @param   [in]        on_millis   ON period setting in millisecond.
 *
 * @param   [in]        off_millis  OFF period setting in millisecond.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t LEDS_Blink(uint8_t led_idx, uint32_t on_millis, uint32_t off_millis)
{
    uint32_t delta_time;
    LED *p_led;

    if(led_idx >= LEDS_TOTAL)
        return -1;

    p_led = &LEDS_Data[led_idx];

    delta_time = Timer1_GetMillis() - p_led->prev_time;

    /* ON state */
    if(p_led->led_state == LED_NORMAL_ON_STATE){

        if(delta_time >= on_millis){
            *(p_led->p_pin_reg) &= ~(p_led->reg_bit);
            p_led->led_state = LED_NORMAL_OFF_STATE;
            p_led->prev_time = Timer1_GetMillis();
        }

    }
    /* OFF state */
    else{
        if(delta_time >= off_millis){
            *(p_led->p_pin_reg) |= (p_led->reg_bit);
            p_led->led_state = LED_NORMAL_ON_STATE;
            p_led->prev_time = Timer1_GetMillis();
        }
    }

    return 0;
}

/**
 * LEDS_Lightning - Function to blink specific LED like lightning.
 *
 * @param   [in]        led_idx                 Index of specific LED.
 *                                              Refer to enum LEDS_IDX{...}.
 *
 * @param   [in]        off_millis              OFF period setting in millisecond.
 *
 * @param   [in]        lighting_on_millis      lighting ON period setting in millisecond.
 * @param   [in]        lighting_off_millis     lighting OFF period setting in millisecond.
 *
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t LEDS_Lightning(uint8_t led_idx, uint32_t off_millis,
                      uint32_t lighting_on_millis, uint32_t lighting_off_millis)
{
    uint32_t delta_time;
    LED *p_led;

    if(led_idx >= LEDS_TOTAL)
        return -1;

    p_led = &LEDS_Data[led_idx];

    delta_time = Timer1_GetMillis() - p_led->prev_time;

    switch(p_led->led_state){
        case LED_NORMAL_OFF_STATE:

            /* Turn ON LED if needed */
            if(delta_time >= off_millis){
                *(p_led->p_pin_reg) |= (p_led->reg_bit);
                p_led->led_state = LED_NORMAL_ON_STATE;
                p_led->prev_time = Timer1_GetMillis();
            }

            break;
        case LED_NORMAL_ON_STATE:

            /* Turn OFF LED if needed */
            if(delta_time >= lighting_on_millis){
                *(p_led->p_pin_reg) &= ~(p_led->reg_bit);
                p_led->led_state = LED_LIGHTING_OFF_STATE;
                p_led->prev_time = Timer1_GetMillis();
            }

            break;
        case LED_LIGHTING_OFF_STATE:

            /* Turn ON LED if needed */
            if(delta_time >= lighting_off_millis){
                *(p_led->p_pin_reg) |= (p_led->reg_bit);
                p_led->led_state = LED_LIGHTING_ON_STATE;
                p_led->prev_time = Timer1_GetMillis();
            }

            break;
        case LED_LIGHTING_ON_STATE:

            /* Turn ON LED if needed */
            if(delta_time >= lighting_on_millis){
                *(p_led->p_pin_reg) &= ~(p_led->reg_bit);
                p_led->led_state = LED_NORMAL_OFF_STATE;
                p_led->prev_time = Timer1_GetMillis();
            }

            break;
        default:
            break;
    }

    return 0;
}


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
