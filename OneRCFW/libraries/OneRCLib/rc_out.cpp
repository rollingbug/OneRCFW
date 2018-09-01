/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    rc_out.cpp
 * @brief   RC output pulse (PWM control signal) generator.
 *
 *          Pin/Channel mapping:
 *          Nano:
 *              Logic CH 0 = Arduino D3 / AVR PD3 / OC2B (Timer2, 8 bits)
 *              Logic CH 1 = Arduino D11 / AVR PB3 / OC2A (Timer2, 8 bits)
 *              Logic CH 2 = Arduino D10 / AVR PB2 / OC1B (Timer1, 16 bits)
 *              Logic CH 3 = Arduino D9 / AVR PB1 / OC1A (Timer1, 16 bits)
 *          Pro MINI:
 *              Logic CH 0 = Arduino D9 / AVR PB1 / OC1A (Timer1, 16 bits)
 *              Logic CH 1 = Arduino D10 / AVR PB2 / OC1B (Timer1, 16 bits)
 *              Logic CH 2 = Arduino D11 / AVR PB3 / OC2A (Timer2, 8 bits)
 *              Logic CH 3 = Arduino D3 / AVR PD3 / OC2B (Timer2, 8 bits)
 *
 *          Hardware:
 *              Timer1 (Output compare 1A, 1B)
 *              Timer2 (Output compare 2A, 2B)
 *
 *          Interrupt:
 *              ISR(TIMER1_COMPA_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>
#include <avr/io.h>

#include "rc_out.h"
#include "timers_drv.h"
#include "uart_stream.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Default PPM cycle period setting, give 20 ms by default */
#define RCOUT_CYC_PERIOD_TICKS      TIMER1_MICROS_TO_TICKS(20000)   /* 20 ms */

#define RCOUT_PREPARE_MARGIN        TIMER1_MICROS_TO_TICKS(128)     /* 128 us */
#define RCOUT_RESERVED_PULSE_TICKS  TIMER1_MICROS_TO_TICKS(1500)    /* 1.5 ms */

#define RCOUT_PULSE_MIN_TICKS       TIMER1_MICROS_TO_TICKS(500)     /* 500 us */
#define RCOUT_PULSE_MAX_TICKS       TIMER1_MICROS_TO_TICKS(2500)    /* 2500 us */

#define RCOUT_IS_PUSLE_VALID(ticks) (ticks >= RCOUT_PULSE_MIN_TICKS     \
                                     && ticks <= RCOUT_PULSE_MAX_TICKS)


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* Data structure for storing RC output channel/pin information */
typedef struct rcout_channel{
    uint8_t ardu_pin;
    uint16_t now_pwm_width;
    uint16_t set_pwm_width;
    volatile uint8_t *p_oc_register;
}RCOUT_CHANNEL;

/* definition of RCOUT PWM/PPM cycle stat */
typedef enum rcout_cycle_stat{
    RCOUT_PULSE_START,
    RCOUT_PULSE_END,
    RCOUT_PULSE_ALL_FINISH,
    RCOUT_PULSE_IDLE,
}RCOUT_CYCLE_STAT;

/* definition of AVR output compare mode */
typedef enum rcout_oc_mode{
    RCOUT_OC_NORMAL,
    RCOUT_OC_TOGGLE,
    RCOUT_OC_CLEAR,
    RCOUT_OC_SET,
}RCOUT_OC_MODE;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

static RCOUT_CYCLE_STAT RCOUT_ServoPWMCycleStat;    /* PPM/PWM cycle sate */
static uint16_t RCOUT_ServoPWMCycleStartTime;       /* Start time of current cycle */
static uint16_t RCOUT_ServoPWMCycleShiftTime;       /* Shift time of current cycle */
static uint8_t RCOUT_ServoPWMUpdateCnt;             /* Total updated PPM/PWM cycles */
static uint8_t RCOUT_CurrentChannelIdx;             /* Current working output channel */

/* Data structure for storing RC output channel/pin information */
static RCOUT_CHANNEL RCOUT_Channels[RCOUT_CH_TOTAL] =
{
#ifdef ARDUINO_AVR_PRO
    [RCOUT_THRO_IDX] = {9,  0, 0, &OCR1AL},     /* Throttle */
    [RCOUT_AILE_IDX] = {10, 0, 0, &OCR1BL},     /* Ailerons */
    [RCOUT_ELEV_IDX] = {11, 0, 0, &OCR2A},      /* Elevator */
    [RCOUT_RUDD_IDX] = {3,  0, 0, &OCR2B},      /* Rudder */

#else
    [RCOUT_THRO_IDX] = {9,  0, 0, &OCR1AL},     /* Throttle */
    [RCOUT_AILE_IDX] = {10, 0, 0, &OCR1BL},     /* Ailerons */
    [RCOUT_ELEV_IDX] = {11, 0, 0, &OCR2A},      /* Elevator */
    [RCOUT_RUDD_IDX] = {3,  0, 0, &OCR2B},      /* Rudder */

#endif // ARDUINO_AVR_PRO
};


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

static void RCOUT_LoadNewPWMPulse();
static void RCOUT_SetOutputCompare(intptr_t oc_register_addr, RCOUT_OC_MODE mode);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * RCOUT_Init - Function to initialize RCOUT functions.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
int8_t RCOUT_Init()
{
    uint8_t ch_idx;

    RCOUT_CurrentChannelIdx = 0;
    RCOUT_ServoPWMCycleStartTime = 0;
    RCOUT_ServoPWMCycleShiftTime = 0;
    RCOUT_ServoPWMCycleStat = RCOUT_PULSE_IDLE;
    RCOUT_ServoPWMUpdateCnt = 0;

    Uart0_Printf(PSTR("[RCOUT] Pins:"));

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCOUT_CH_TOTAL; ch_idx++){
        pinMode(RCOUT_Channels[ch_idx].ardu_pin, OUTPUT);

        Uart0_Printf(PSTR(" %hhu"), RCOUT_Channels[ch_idx].ardu_pin);
    }

    Uart0_Println(PSTR(""));

    /* Set OC1A compare timer to launch the PPM cycle if need */
    if(RCOUT_ServoPWMCycleStat == RCOUT_PULSE_IDLE){

        /* Change current cycle stat */
        RCOUT_ServoPWMCycleStat = RCOUT_PULSE_START;

        /* Let's fire the first timer interrupt after 2 ms */
        OCR1A = TCNT1 + TIMER1_MICROS_TO_TICKS(2000);

        RCOUT_ServoPWMCycleStartTime = OCR1A;
        RCOUT_ServoPWMCycleShiftTime = OCR1A;

        /* Clear OC1A and OC1B interrupt flag */
        TIFR1 |= (_BV(OCF1B) | _BV(OCF1A));

        /* Enable timer 1A compare interrupt and just waiting for trigger */
        TIMSK1 |= _BV(OCIE1A);
    }

    return 0;
}

/**
 * RCOUT_GetCycUpdateCnt - Function to get current RCOUT PPM/PWM cycle index,
 *                         this function is created to help us determine whether
 *                         the RCOUT signal is updated or not.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Current PPM/PWM cycle index.
 * @retval  [0~255]
 *
 */
uint8_t RCOUT_GetCycUpdateCnt()
{
    return RCOUT_ServoPWMUpdateCnt;
}

/**
 * RCOUT_SetServoPWM - A public function for adjusting width of
 *                     output PWM signals.
 *
 * @param   [in]        *p_channels_width   uint16_t array[] contains
 *                                          new pulse width setting.
 *                                          (The unit is timer1 ticks).
 *
 * @param   [in]        channel_total       Total channel number.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t RCOUT_SetServoPWM(uint16_t *p_channels_width, uint8_t channel_total)
{
    uint8_t ch_idx;
    uint8_t old_SREG;

    if(channel_total != RCOUT_CH_TOTAL)
        return -1;

    /* Check pulse width */
    for(ch_idx = 0; ch_idx < RCOUT_CH_TOTAL; ch_idx++){
        if(RCOUT_IS_PUSLE_VALID(p_channels_width[ch_idx]) == 0)
            RCOUT_Channels[ch_idx].set_pwm_width = 0;
    }

    old_SREG = SREG;
    cli();

    for(ch_idx = 0; ch_idx < RCOUT_CH_TOTAL; ch_idx++){
        RCOUT_Channels[ch_idx].set_pwm_width = p_channels_width[ch_idx];
    }

    SREG = old_SREG;

    return 0;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * RCOUT_LoadNewPWMPulse - Function to load new PMW pulse width setting.
 *
 * @param   [none]
 *
 * @return  [none]
 */
static void RCOUT_LoadNewPWMPulse()
{
    uint8_t ch_idx;

    for(ch_idx = 0; ch_idx < RCOUT_CH_TOTAL; ch_idx++)
        RCOUT_Channels[ch_idx].now_pwm_width = RCOUT_Channels[ch_idx].set_pwm_width;

    RCOUT_ServoPWMUpdateCnt++;
}

/**
 * RCOUT_SetOutputCompare - Function to switch AVR OC1A, OC1B, OC2A, OC2B
 *                          operating mode.
 *
 * @param   [in]    oc_register_addr    Address of OCxx register.
 *
 * @param   [in]    mode                Specific output compare mode.
 *                                      RCOUT_OC_NORMAL: normal mode.
 *                                      RCOUT_OC_TOGGLE: toggle mode.
 *                                      RCOUT_OC_CLEAR: Output LOW when triggered.
 *                                      RCOUT_OC_SET: Output HIGH when triggered.
 *
 * @return  [none]
 *
 */
static void RCOUT_SetOutputCompare(intptr_t oc_register_addr, RCOUT_OC_MODE mode)
{
    /* Predefined configuration for switch "output compare mode". */
    static const uint8_t OC1A_Settings[] =
    {
        [RCOUT_OC_NORMAL]   = (uint8_t)~(_BV(COM1A1) | _BV(COM1A0)),
        [RCOUT_OC_TOGGLE]   = (uint8_t)~(_BV(COM1A1)),
        [RCOUT_OC_CLEAR]    = (uint8_t)~(_BV(COM1A0)),
        [RCOUT_OC_SET]      = (uint8_t)(_BV(COM1A1) | _BV(COM1A0)),
    };
    static const uint8_t OC1B_Settings[] =
    {
        [RCOUT_OC_NORMAL]   = (uint8_t)~(_BV(COM1B1) | _BV(COM1B0)),
        [RCOUT_OC_TOGGLE]   = (uint8_t)~(_BV(COM1B1)),
        [RCOUT_OC_CLEAR]    = (uint8_t)~(_BV(COM1B0)),
        [RCOUT_OC_SET]      = (uint8_t)(_BV(COM1B1) | _BV(COM1B0)),
    };
    static const uint8_t OC2A_Settings[] =
    {
        [RCOUT_OC_NORMAL]   = (uint8_t)~(_BV(COM2A1) | _BV(COM2A0)),
        [RCOUT_OC_TOGGLE]   = (uint8_t)~(_BV(COM2A1)),
        [RCOUT_OC_CLEAR]    = (uint8_t)~(_BV(COM2A0)),
        [RCOUT_OC_SET]      = (uint8_t)(_BV(COM2A1) | _BV(COM2A0)),
    };
    static const uint8_t OC2B_Settings[] =
    {
        [RCOUT_OC_NORMAL]   = (uint8_t)~(_BV(COM2B1) | _BV(COM2B0)),
        [RCOUT_OC_TOGGLE]   = (uint8_t)~(_BV(COM2B1)),
        [RCOUT_OC_CLEAR]    = (uint8_t)~(_BV(COM2B0)),
        [RCOUT_OC_SET]      = (uint8_t)(_BV(COM2B1) | _BV(COM2B0)),
    };

    switch(oc_register_addr){
        case ((intptr_t)&OCR1AL):

            TCCR1A |= (_BV(COM1A1) | _BV(COM1A0));
            TCCR1A &= OC1A_Settings[mode];

            break;

        case ((intptr_t)&OCR1BL):

            TCCR1A |= (_BV(COM1B1) | _BV(COM1B0));
            TCCR1A &= OC1B_Settings[mode];

            break;

        case ((intptr_t)&OCR2A):

            TCCR2A |= (_BV(COM2A1) | _BV(COM2A0));
            TCCR2A &= OC2A_Settings[mode];

            break;

        case ((intptr_t)&OCR2B):

            TCCR2A |= (_BV(COM2B1) | _BV(COM2B0));
            TCCR2A &= OC2B_Settings[mode];

            break;

        default:
            break;
    }
}

/**
 * RCOUT_GenServoPWM - Function to generate "servo PWM" signals.
 *
 * This function should be called in ISR(TIMER1_COMPA_vect).
 * By default, it will generate number of "servo PWM" signal
 * in different predefined IO pins sequentially.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
void RCOUT_GenServoPWM()
{
    RCOUT_CHANNEL *p_channel;
    uint16_t pulse_width;

    p_channel = &RCOUT_Channels[RCOUT_CurrentChannelIdx];

    if(p_channel->now_pwm_width == 0)
        pulse_width = RCOUT_RESERVED_PULSE_TICKS;
    else
        pulse_width = p_channel->now_pwm_width;

    switch(RCOUT_ServoPWMCycleStat){
        /* Ready to HIGH the signal */
        case RCOUT_PULSE_START:

            /* Hardware based servo PWM signal */
            if(p_channel->p_oc_register != NULL){
                /* Special for the channel which is assigned in OC1A pin */
                if(p_channel->p_oc_register == &OCR1AL){

                    RCOUT_ServoPWMCycleShiftTime += RCOUT_PREPARE_MARGIN;
                    OCR1A = RCOUT_ServoPWMCycleShiftTime;
                }
                /* General OC pins */
                else{
                    RCOUT_ServoPWMCycleShiftTime += RCOUT_PREPARE_MARGIN;

                    if(p_channel->p_oc_register == &OCR1BL)
                        OCR1B = RCOUT_ServoPWMCycleShiftTime;
                    else
                        *(p_channel->p_oc_register) = (uint8_t)RCOUT_ServoPWMCycleShiftTime;

                    RCOUT_ServoPWMCycleShiftTime += (pulse_width - RCOUT_PREPARE_MARGIN);
                    OCR1A = RCOUT_ServoPWMCycleShiftTime;
                }

                /*
                 * Set output to HIGH only when pulse width is not equal to 0.
                 * We should only change output compare mode after we update OCRxx counter.
                 */
                if(p_channel->now_pwm_width != 0){
                    RCOUT_SetOutputCompare((intptr_t)p_channel->p_oc_register,
                                           RCOUT_OC_SET);
                }
            }
            /* Software based PWM signal*/
            else{
                /* TODO */
            }

            RCOUT_ServoPWMCycleStat = RCOUT_PULSE_END;

            break;

        /* Ready to LOW the signal */
        case RCOUT_PULSE_END:

            /* Hardware based servo PWM signal */
            if(p_channel->p_oc_register != NULL){
                if(p_channel->p_oc_register == &OCR1AL){
                    RCOUT_ServoPWMCycleShiftTime += pulse_width;
                    OCR1A = RCOUT_ServoPWMCycleShiftTime;
                }
                else{
                    RCOUT_ServoPWMCycleShiftTime += RCOUT_PREPARE_MARGIN;

                    if(p_channel->p_oc_register == &OCR1BL)
                        OCR1B = RCOUT_ServoPWMCycleShiftTime;
                    else
                        *(p_channel->p_oc_register) = (uint8_t)RCOUT_ServoPWMCycleShiftTime;

                    OCR1A = RCOUT_ServoPWMCycleShiftTime;
                }

                RCOUT_SetOutputCompare((intptr_t)p_channel->p_oc_register,
                                       RCOUT_OC_CLEAR);
            }
            /* Software based servo PWM signal*/
            else{
                /* TODO */
            }

            RCOUT_CurrentChannelIdx++;
            if(RCOUT_CurrentChannelIdx == RCOUT_CH_TOTAL){
                RCOUT_CurrentChannelIdx = 0;
                RCOUT_ServoPWMCycleStat = RCOUT_PULSE_ALL_FINISH;
            }
            else{
                RCOUT_ServoPWMCycleStat = RCOUT_PULSE_START;
            }

            break;
        /* Already complete all pulse and wait for next cycle */
        case RCOUT_PULSE_ALL_FINISH:

            RCOUT_LoadNewPWMPulse();
            RCOUT_ServoPWMCycleStartTime += RCOUT_CYC_PERIOD_TICKS;
            RCOUT_ServoPWMCycleShiftTime = RCOUT_ServoPWMCycleStartTime - RCOUT_PREPARE_MARGIN;

            OCR1A = RCOUT_ServoPWMCycleShiftTime;

            RCOUT_ServoPWMCycleStat = RCOUT_PULSE_START;

            break;
        default:
            break;
    }
}
