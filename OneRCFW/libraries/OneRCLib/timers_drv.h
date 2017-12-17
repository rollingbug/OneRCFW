/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    timers_drv.h
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef TIMERS_DRV_H_
#define TIMERS_DRV_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Timer0, Pre-scaler setting, support 0, 1, 8, 64, 256, 1024,
 * 1024, but please do not change default setting, otherwise it
 * may cause unpredictable problem (Set to 8 by default).
 *******************************************************************************
 */
#define TIMER0_PRESCALER                8

/* Timer0, Frequency, 16 Mhz / 8 = 2 Mhz by default */
#define TIMER0_FREQ                     (F_CPU / TIMER0_PRESCALER)

/* Timer0 ticks per microsecond, 2 Mhz / 1 Mhz = 2, 2 ticks */
#define TIMER0_TICKS_PER_US             (TIMER0_FREQ / 1000000)

/* Timer0, convert ticks to micro seconds */
#define TIMER0_TICKS_TO_MICROS(ticks)   (ticks / TIMER0_TICKS_PER_US)

/* Timer0, convert micro seconds to ticks */
#define TIMER0_MICROS_TO_TICKS(micros)  (micros * TIMER0_TICKS_PER_US)


/*
 *******************************************************************************
 * Timer1, Pre-scaler setting, support 0, 1, 8, 64, 256, 1024,
 * but please do not change default setting, otherwise it may
 * cause unpredictable problem (Set to 8 by default).
 *******************************************************************************
 */
#define TIMER1_PRESCALER                8

/* Timer1, Frequency, 16 Mhz / 8 = 2 Mhz by default */
#define TIMER1_FREQ                     (F_CPU / TIMER1_PRESCALER)

/* Timer1 ticks per microsecond, 2 Mhz / 1 Mhz = 2, 2 ticks */
#define TIMER1_TICKS_PER_US             (TIMER1_FREQ / 1000000)

/*
 * Timer1 ticks per millisecond, 2 Mhz / 1 Khz = 2000, 2000 ticks,
 * but we give 2048 for avoiding division calculation.
 */
#define TIMER1_TICKS_PER_MS             (2048)

/* Timer1, convert ticks to micro seconds */
#define TIMER1_TICKS_TO_MICROS(ticks)   (ticks / TIMER1_TICKS_PER_US)

/* Timer1, convert micro seconds to ticks */
#define TIMER1_MICROS_TO_TICKS(micros)  (micros * TIMER1_TICKS_PER_US)

/* Timer1, convert ticks to milliseconds */
#define TIMER1_TICKS_TO_MILLIS(ticks)   (ticks / TIMER1_TICKS_PER_MS)

/* Timer1, convert milliseconds to ticks */
#define TIMER1_MILLIS_TO_TICKS(millis)  (millis * TIMER1_TICKS_PER_MS)


/*
 *******************************************************************************
 * Timer2, Pre-scaler setting, support 0, 1, 8, 32, 64, 128, 256,
 * 1024, but please do not change default setting, otherwise it
 * may cause unpredictable problem (Set to 8 by default).
 *******************************************************************************
 */

#define TIMER2_PRESCALER                8


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

int8_t Timers_Init();

uint8_t Timer0_GetTicks8();
int8_t Timer0_SetTimerCompA(uint8_t trig_time, bool is_enable_interrupt);
int8_t Timer0_SetTimerCompB(uint8_t trig_time, bool is_enable_interrupt);

uint16_t Timer1_GetTicks16();
uint32_t Timer1_GetTicks32();
uint32_t Timer1_GetMicros();
uint32_t Timer1_GetMillis();
void Timer1_DelayMillis(uint32_t millis);
int8_t Timer1_SetInputTriggerEdge(bool is_rising_edge);
int8_t Timer1_SetInputCapture(bool is_rising_edge, bool is_noise_cancel,
                              bool is_enable_interrupt);
uint16_t Timer1_ReadInputCaptureTime();


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

#endif // TIMERS_DRV_H_
