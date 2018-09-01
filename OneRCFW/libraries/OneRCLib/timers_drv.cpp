/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    timers_drv.cpp
 * @brief
 *
 *          Abbreviations:
 *              FREQ    - Frequency
 *
 *          Hardware:
 *              Timer1, Timer2
 *
 *          Interrupt:
 *              ISR(TIMER1_OVF_vect)
 *              ISR(TIMER1_COMPA_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "timers_drv.h"
#include "rc_out.h"
#include "uart_stream.h"
#include "uart_sim.h"
#include "debug.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#if TIMER0_PRESCALER == 0
    #define TIMER0_CLK_SEL      (0)
#elif TIMER0_PRESCALER == 1
    #define TIMER0_CLK_SEL      (_BV(CS00))
#elif TIMER0_PRESCALER == 8
    #define TIMER0_CLK_SEL      (_BV(CS01))
#elif TIMER0_PRESCALER == 64
    #define TIMER0_CLK_SEL      (_BV(CS01) | _BV(CS00))
#elif TIMER0_PRESCALER == 256
    #define TIMER0_CLK_SEL      (_BV(CS02))
#elif TIMER0_PRESCALER == 1024
    #define TIMER0_CLK_SEL      (_BV(CS02) | _BV(CS00))
#else
    #error "Unsupported Timer 0 prescaler setting\n"
#endif

#if TIMER1_PRESCALER == 0
    #define TIMER1_CLK_SEL      (0)
#elif TIMER1_PRESCALER == 1
    #define TIMER1_CLK_SEL      (_BV(CS10))
#elif TIMER1_PRESCALER == 8
    #define TIMER1_CLK_SEL      (_BV(CS11))
#elif TIMER1_PRESCALER == 64
    #define TIMER1_CLK_SEL      (_BV(CS11) | _BV(CS10))
#elif TIMER1_PRESCALER == 256
    #define TIMER1_CLK_SEL      (_BV(CS12))
#elif TIMER1_PRESCALER == 1024
    #define TIMER1_CLK_SEL      (_BV(CS12) | _BV(CS10))
#else
    #error "Unsupported Timer 1 prescaler setting\n"
#endif

#if TIMER2_PRESCALER == 0
    #define TIMER2_CLK_SEL      (0)
#elif TIMER2_PRESCALER == 1
    #define TIMER2_CLK_SEL      (_BV(CS20))
#elif TIMER2_PRESCALER == 8
    #define TIMER2_CLK_SEL      (_BV(CS21))
#elif TIMER2_PRESCALER == 32
    #define TIMER2_CLK_SEL      (_BV(CS21) | _BV(CS20))
#elif TIMER2_PRESCALER == 64
    #define TIMER2_CLK_SEL      (_BV(CS22))
#elif TIMER2_PRESCALER == 128
    #define TIMER2_CLK_SEL      (_BV(CS22) | _BV(CS20))
#elif TIMER2_PRESCALER == 256
    #define TIMER2_CLK_SEL      (_BV(CS22) | _BV(CS21))
#elif TIMER2_PRESCALER == 1024
    #define TIMER2_CLK_SEL      (_BV(CS22) | _BV(CS21) | _BV(CS20))
#else
    #error "Unsupported Timer 2 prescaler setting\n"
#endif


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

static uint16_t Timer1_OVFCnt;
static uint32_t Timer1_MicroCnt;
static uint32_t Timer1_MilliCnt;


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

extern void RCOUT_GenServoPWM();

static int8_t Timer0_Init();
static int8_t Timer1_Init();
static int8_t Timer2_Init();


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * Timers_Init - Function to initial and sync AVR timer 1 and 2.
 *
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Timers_Init()
{
    /* Disable all interrupts */
    cli();

    /* Halt all timer for synchronizing timer counter */
    GTCCR = (_BV(TSM) | _BV(PSRASY) | _BV(PSRSYNC));

    Timer0_Init();
    Timer1_Init();
    Timer2_Init();

    /* Resume all timers */
    GTCCR = 0;

    /* Enable all interrupts */
    sei();

    Uart0_Println(PSTR("[TMRs] Enable Tmr1, Tmr2. Disable Tmr 0"));

    return 0;
}

/**
 * Timer0_GetTicks8 - Function to get current TICK of timer 0.
 *                    (8 bits).
 *
 * @param   [none]
 *
 * @return  [uint8_t]   Current current tick number.
 *
 */
uint8_t Timer0_GetTicks8()
{
    return TCNT0;
}

/**
 * Timer0_SetTimerCompA - Function to set timer0A output compare function.
 *                        (Normal mode without output signal).
 *
 * Requires 1 ~ 1.125 us.
 *
 * @param   [in]    trig_time               Expected trigger point.
 * @param   [in]    is_enable_interrupt     Set to true for enabling compare interrupt.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Timer0_SetTimerCompA(uint8_t trig_time, bool is_enable_interrupt)
{
    uint8_t old_SREG;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    /* Assign new trigger point */
    OCR0A = trig_time;

    /* Clear interrupt flag */
    TIFR0 = _BV(OCF0A);

    /* Enable or disable interrupt */
    if(is_enable_interrupt == true){
        TIMSK0 |= _BV(OCIE0A);
    }
    else{
        TIMSK0 &= ~_BV(OCIE0A);
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    return 0;
}

/**
 * Timer0_SetTimerCompB - Function to set timer0B output compare function.
 *                        (Normal mode without output signal).
 *
 * @param   [in]    trig_time               Expected trigger point.
 * @param   [in]    is_enable_interrupt     Set to true for enabling compare interrupt.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Timer0_SetTimerCompB(uint8_t trig_time, bool is_enable_interrupt)
{
    uint8_t old_SREG;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    /* Assign new trigger point */
    OCR0B = trig_time;

    /* Clear interrupt flag */
    TIFR0 = _BV(OCF0B);

    /* Enable or disable interrupt */
    if(is_enable_interrupt == true){
        TIMSK0 |= _BV(OCIE0B);
    }
    else{
        TIMSK0 &= ~_BV(OCIE0B);
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    return 0;
}

/**
 * Timer1_GetTicks16 - Function to get accumulated TICKs of timer 1.
 *                     (16 bits).
 *
 * @param   [none]
 *
 * @return  [uint16_t]  Current accumulated ticks number.
 *
 */
uint16_t Timer1_GetTicks16()
{
    return TCNT1;
}

/**
 * Timer1_GetTicks32 - Function to get accumulated TICKs of timer 1.
 *                     (32 bits).
 *
 * @param   [none]
 *
 * @return  [uint32_t]  Current accumulated ticks number.
 *
 */
uint32_t Timer1_GetTicks32()
{
    uint8_t old_SREG;
    volatile uint16_t ticks_lsb16;
    uint16_t ticks_msb16;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    ticks_lsb16 = TCNT1;
    ticks_msb16 = Timer1_OVFCnt;

    /*
     * If timer overflow is just happened,
     * increase MSB16 and re-fetch TCNT1 (LSB16).
     */
    if(TIFR1 & _BV(TOV1)){
        ticks_lsb16 = TCNT1;
        ticks_msb16 += 1;
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    /* Combine MSB16 and LSB16 then return 32 bits data */
    return ((((uint32_t)ticks_msb16) << 16) | ticks_lsb16);
}

/**
 * Timer1_GetMicros - Function to get accumulated microseconds of timer 1.
 *                    (32 bits).
 *
 * @param   [none]
 *
 * @return  [uint32_t]  Current accumulated ticks number.
 *
 */
uint32_t Timer1_GetMicros()
{
    uint8_t old_SREG;
    volatile uint32_t micros;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    micros = TIMER1_TICKS_TO_MICROS(TCNT1);
    micros += Timer1_MicroCnt;

    /*
     * If timer overflow is just happened,
     * re-fetch TCNT1 and add global microsecond counter.
     */
    if(TIFR1 & _BV(TOV1)){
        micros = TIMER1_TICKS_TO_MICROS(TCNT1);
        micros += Timer1_MicroCnt + TIMER1_TICKS_TO_MICROS(0x10000);
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    return micros;
}

/**
 * Timer1_GetMillis - Function to get accumulated milliseconds of timer 1.
 *                    (32 bits).
 *
 * @param   [none]
 *
 * @return  [uint32_t]  Current accumulated ticks number.
 *
 */
uint32_t Timer1_GetMillis()
{
    uint8_t old_SREG;
    volatile uint32_t millis;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    millis = TIMER1_TICKS_TO_MILLIS(TCNT1);
    millis += Timer1_MilliCnt;

    /*
     * If timer overflow is just happened,
     * re-fetch TCNT1 and add global millisecond counter.
     */
    if(TIFR1 & _BV(TOV1)){
        millis = TIMER1_TICKS_TO_MILLIS(TCNT1);
        millis += Timer1_MilliCnt + TIMER1_TICKS_TO_MILLIS(0x10000);
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    return millis;
}

/**
 * Timer1_DelayMillis - Function to pause program N milliseconds.
 *                      (This function should not be called in ISR)
 *
 * @param   [in]    millis      N milliseconds delay.
 *
 * @return  [none]
 *
 */
void Timer1_DelayMillis(uint32_t millis)
{
    uint32_t start;

    start = Timer1_GetMillis();

    while(Timer1_GetMillis() - start < millis)
        ;
}

/**
 * Timer1_SetInputTriggerEdge - Function to set input capture trigger edge mode.
 *
 * @param   [in]    is_rising_edge      true: Trigger for rising edge.
 *                                      false: Trigger for falling edge.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Timer1_SetInputTriggerEdge(bool is_rising_edge)
{
    /* Setup edge trigger mode */
    if(is_rising_edge == true){
        TCCR1B |= _BV(ICES1);
    }
    else{
        TCCR1B &= ~_BV(ICES1);
    }

    return 0;
}

/**
 * Timer1_SetInputCapture - Function to set parameters of timer 1
 *                          input capture function.
 *
 * Please be carefully to enable this function in system, the input
 * capture function may generate interrupt frequently and may break
 * the timing of system.
 *
 *
 * @param   [in]    is_rising_edge          true: Trigger for rising edge.
 *                                          false: Trigger for falling edge.
 *
 * @param   [in]    is_noise_cancel         Set true for enabling noise
 *                                          canceling function.
 *
 * @param   [in]    is_enable_interrupt     Set true for enabling input
 *                                          capture interrupt.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Timer1_SetInputCapture(bool is_rising_edge, bool is_noise_cancel,
                              bool is_enable_interrupt)
{
    uint8_t old_SREG;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    /* Setup input capture interrupt */
    if(is_enable_interrupt == true)
        TIMSK1 |= _BV(ICIE1);
    else
        TIMSK1 &= ~_BV(ICIE1);

    /* Setup noise canceler function */
    if(is_noise_cancel == true)
        TCCR1B |= _BV(ICNC1);
    else
        TCCR1B &= ~_BV(ICNC1);

    /* Set edge trigger mode */
    Timer1_SetInputTriggerEdge(is_rising_edge);

    /* Clear input capture interrupt flag */
    TIFR1 = _BV(ICF1);

    /* Enable global interrupt */
    SREG = old_SREG;

    return 0;
}

/**
 * Timer1_ReadInputCaptureTime - Function to read the time-stamp of latest captured input signal.
 *
 * @param   [none]
 *
 * @return  [uint16_t]      time-stamp of captured signal.
 * @retval  [0~65535]
 *
 */
uint16_t Timer1_ReadInputCaptureTime()
{
    return ICR1;
}

/**
 * ISR(TIMER1_OVF_vect) - Timer 1 overflow ISR.
 *
 * This ISR will be called when every time timer 1 overflow
 * event is triggered.
 * Requires about 3.375 ~ 3.5 us.
 *
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(TIMER1_OVF_vect)
{
    DEBUG_ISR_START(TIMER1_OVF_vect_num);

    Timer1_OVFCnt++;
    Timer1_MicroCnt += TIMER1_TICKS_TO_MICROS(0x10000);
    Timer1_MilliCnt += TIMER1_TICKS_TO_MILLIS(0x10000);

    DEBUG_ISR_END(TIMER1_OVF_vect_num);
}

/**
 * ISR(TIMER1_COMPA_vect) - Timer/Counter1 Compare Match A ISR.
 *
 * Requires 6.5 ~ 8.875 us.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(TIMER1_COMPA_vect)
{
    DEBUG_ISR_START(TIMER1_COMPA_vect_num);

    RCOUT_GenServoPWM();

    DEBUG_ISR_END(TIMER1_COMPA_vect_num);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * Timer0_Init - Function to initial AVR timer 0 (8 bits timer).
 *               Just DISABLE timer0 by default.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Timer0_Init()
{
    TCCR0A = 0;
    TCCR0B = TIMER0_CLK_SEL;    /* Pre-scaler = 8, 16MHz / 8 = 2 MHz, per tick = 0.5 us */
    TCNT0 = 0;
    TIFR0 = 0xFF;               /* Clear timer interrupt status */
    TIMSK0 = 0;                 /* Disable all timer0 interrupts */

    return 0;
}

/**
 * Timer1_Init - Function to initial AVR timer 1 (16 bits timer).
 *               Pre-scaler:        8
 *               Frequency:         16MHz / 8 = 2 MHz
 *               Tick resolution:   1s / 2000 = 0.5 us
 *               Interrupts:        OVF.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
static int8_t Timer1_Init()
{
    Timer1_OVFCnt = 0;
    Timer1_MicroCnt = 0;
    Timer1_MilliCnt = 0;

    TCCR1A = 0;                 /* Normal Mode, we will enable output compare match mode later */
    TCCR1B = TIMER1_CLK_SEL;    /* Pre-scaler = 8, 16MHz / 8 = 2 MHz, per tick = 0.5 us */
    TCCR1C = 0;                 /* Force compare register, not important in this step */
    TCNT1 = 0;                  /* Reset timer counter */
    TIFR1 = 0xFF;               /* Clear timer interrupt status */
    TIMSK1 = _BV(TOIE1);        /* Enable overflow interrupt */

    return 0;
}

/**
 * Timer2_Init - Function to initial AVR timer 2 (8 bits timer).
 *               Pre-scaler:        None
 *               Frequency:         None
 *               Tick resolution:   None
 *               Interrupts:        None
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 */
static int8_t Timer2_Init()
{
    TCCR2A = 0;                 /* Normal Mode, we will enable output compare match mode later */
    TCCR2B = TIMER2_CLK_SEL;    /* 8 pre-scaler, 16MHz / 8 = 2 MHz, per tick = 0.5 us */
    TCNT2 = 0;                  /* Reset timer counter */
    OCR2A = 0;                  /* Compare target, not important in this step */
    OCR2B = 0;
    TIFR2 = 0xFF;               /* Clear timer interrupt status */
    TIMSK2 = 0;                 /* Disable compare 2A and 2B interrupt */

    return 0;
}
