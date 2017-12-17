/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    rc_in.cpp
 * @brief   RC input pulse (PWM control signal) detector.
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef RC_IN_H_
#define RC_IN_H_

#include "pin_change.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Logic index of RC input channels */
#define RCIN_THRO_IDX   0
#define RCIN_AILE_IDX   1
#define RCIN_ELEV_IDX   2
#define RCIN_RUDD_IDX   3
#define RCIN_AUX1_IDX   4

/* Channels IO port */
#define RCIN_CH_PORT    PIND

/* Channel and pin change definition */
#define RCIN_CH_TOTAL   5

/* Fail safe check */
#define RCIN_FAIL_COND  (                               \
                            (1 << RCIN_THRO_IDX)        \
                            | (1 << RCIN_AILE_IDX)      \
                            | (1 << RCIN_ELEV_IDX)      \
                            | (1 << RCIN_RUDD_IDX)      \
                        )

#define RCIN_PULSE_MIN_TICKS        TIMER1_MICROS_TO_TICKS(900)     /* 900 us */
#define RCIN_PULSE_MAX_TICKS        TIMER1_MICROS_TO_TICKS(2100)    /* 2100 us */

/* Middle point of RC IN signal, the ideal pulse width is 1500 us */
#define RCIN_PULSE_MIDDLE_TICKS     ((RCIN_PULSE_MAX_TICKS              \
                                     + RCIN_PULSE_MIN_TICKS) / 2)

#define RCIN_IS_PULSE_VALID(ticks)  (ticks >= RCIN_PULSE_MIN_TICKS      \
                                     && ticks <= RCIN_PULSE_MAX_TICKS)


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

int8_t RCIN_Init();
bool RCIN_FailChk();
void RCIN_SetDirection(bool *p_channel_is_reversed);
void RCIN_SetNeutral(uint16_t *p_channel_neutral_ticks);
void RCIN_SetMaxMinStick(uint16_t *p_max, uint16_t *p_min);
void RCIN_SetFailsafe(uint16_t *p_channel_failsafe_ticks);
uint8_t RCIN_ReadChannels(uint16_t *p_channels);
void RCIN_GetChannelsDiff(uint16_t *p_pulse_in, int16_t *p_pulse_diff);

void RCIN_PulseHandler(PC_GRP_IDX pc_grp_idx, uint32_t trig_time,
                       uint8_t pin_status, uint8_t pin_change);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // RC_IN_H_
