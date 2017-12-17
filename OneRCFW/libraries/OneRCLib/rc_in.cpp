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
 *          Abbreviations:
 *              AILE    - Ailerons              AUX     - Auxiliary
 *              CH      - Channel               ELEV    - Elevator
 *              PC      - Pin Change            RC      - Remote control
 *              RUDD    - Rudder                THRO    - Throttle
 *
 *          Pin/Channel mapping:
 *          Nano:
 *              Logic CH 0 = Arduino D02 / AVR PD2 / PCINT 18 (PC group 2)
 *              Logic CH 1 = Arduino D04 / AVR PD4 / PCINT 20 (PC group 2)
 *              Logic CH 2 = Arduino D07 / AVR PD7 / PCINT 23 (PC group 2)
 *              Logic CH 3 = Arduino D12 / AVR PB4 / PCINT 4 (PC group 0)
 *              Logic CH 4 = Arduino D08 / AVR PB0 / PCINT 0 (PC group 0)
 *
 *          Hardware:
 *              Pin change detector.
 *
 *          Interrupts:
 *              ISR(PCINT0_vect)
 *              ISR(PCINT2_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdint.h>
#include <avr/io.h>
#include <Arduino.h>

#include "rc_in.h"
#include "timers_drv.h"
#include "uart_stream.h"
#include "pin_change.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* RX signal LPF ratio, give 0 to disable LPF */
#define RCIN_LPF_BETA               2


/*
 *******************************************************************************
 * Macros definition
 *******************************************************************************
 */



/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* Data structure for storing RC input channel/pin information */
typedef struct rcin_channel{
    uint8_t ardu_pin;				/* Arduino digital pin ID */
    uint8_t mask;					/* PCMASK */
    PC_PIN_IDX pc_pin_idx;			/* PC pin index 0~23 */
    PC_GRP_IDX pc_grp_idx;			/* PC group index 0~2 */
    uint8_t update_sequence;        /* Pulse update count of this channel */
    uint32_t pulse_start;           /* 32 bits */
    uint16_t pulse_smooth_width;    /* 16 bits, with low pass filter */
    uint16_t neutral_ticks;         /* Default neutral ticks */
    uint16_t max_ticks;             /* To store maximum detected RC input ticks */
    uint16_t min_ticks;             /* To store minimum detected RC input ticks */
    uint16_t failsafe_ticks;        /* Default failsafe ticks */
    bool is_reversed;
}RCIN_CHANNEL;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

static uint8_t RCIN_ChannelStatus; /* Status of channels, for fail safe, use 32 bits */
static uint8_t RCIN_LatestPinValue; /* Previous value of channel pins */
static uint8_t RCIN_CycUpdateCnt;

/* logic RX channel, Arduino PIN, AVR PC PIN mapping and initialization */
static RCIN_CHANNEL RCIN_Channels[RCIN_CH_TOTAL] =
{
    /* Throttle */
    [RCIN_THRO_IDX] =
                    {
                        .ardu_pin           = 2,
                        .mask               = PC_PIN_MASK_18,
                        .pc_pin_idx         = PC_PIN_IDX_18,
                        .pc_grp_idx         = PC_GRP_IDX_2,
                        .update_sequence    = 0,
                        .pulse_start        = 0,
                        .pulse_smooth_width = 0,
                        .neutral_ticks      = RCIN_PULSE_MIDDLE_TICKS,
                        .max_ticks          = RCIN_PULSE_MAX_TICKS,
                        .min_ticks          = RCIN_PULSE_MIN_TICKS,
                        .failsafe_ticks     = TIMER1_MICROS_TO_TICKS(1000),
                        .is_reversed        = false,
                    },
    /* Ailerons */
    [RCIN_AILE_IDX] =
                    {
                        .ardu_pin           = 4,
                        .mask               = PC_PIN_MASK_20,
                        .pc_pin_idx         = PC_PIN_IDX_20,
                        .pc_grp_idx         = PC_GRP_IDX_2,
                        .update_sequence    = 0,
                        .pulse_start        = 0,
                        .pulse_smooth_width = 0,
                        .neutral_ticks      = RCIN_PULSE_MIDDLE_TICKS,
                        .max_ticks          = RCIN_PULSE_MAX_TICKS,
                        .min_ticks          = RCIN_PULSE_MIN_TICKS,
                        .failsafe_ticks     = TIMER1_MICROS_TO_TICKS(1500),
                        .is_reversed        = false,
                    },
    /* Elevator */
    [RCIN_ELEV_IDX] =
                    {
                        .ardu_pin           = 7,
                        .mask               = PC_PIN_MASK_23,
                        .pc_pin_idx         = PC_PIN_IDX_23,
                        .pc_grp_idx         = PC_GRP_IDX_2,
                        .update_sequence    = 0,
                        .pulse_start        = 0,
                        .pulse_smooth_width = 0,
                        .neutral_ticks      = RCIN_PULSE_MIDDLE_TICKS,
                        .max_ticks          = RCIN_PULSE_MAX_TICKS,
                        .min_ticks          = RCIN_PULSE_MIN_TICKS,
                        .failsafe_ticks     = TIMER1_MICROS_TO_TICKS(1500),
                        .is_reversed        = false,
                    },
    /* Rudder */
    [RCIN_RUDD_IDX] =
                    {
                        .ardu_pin           = 12,
                        .mask               = PC_PIN_MASK_4,
                        .pc_pin_idx         = PC_PIN_IDX_4,
                        .pc_grp_idx         = PC_GRP_IDX_0,
                        .update_sequence    = 0,
                        .pulse_start        = 0,
                        .pulse_smooth_width = 0,
                        .neutral_ticks      = RCIN_PULSE_MIDDLE_TICKS,
                        .max_ticks          = RCIN_PULSE_MAX_TICKS,
                        .min_ticks          = RCIN_PULSE_MIN_TICKS,
                        .failsafe_ticks     = TIMER1_MICROS_TO_TICKS(1500),
                        .is_reversed        = false,
                    },
    /* AUX 1 */
    [RCIN_AUX1_IDX] =
                    {
                        .ardu_pin           = 8,
                        .mask               = PC_PIN_MASK_0,
                        .pc_pin_idx         = PC_PIN_IDX_0,
                        .pc_grp_idx         = PC_GRP_IDX_0,
                        .update_sequence    = 0,
                        .pulse_start        = 0,
                        .pulse_smooth_width = 0,
                        .neutral_ticks      = RCIN_PULSE_MIDDLE_TICKS,
                        .max_ticks          = RCIN_PULSE_MAX_TICKS,
                        .min_ticks          = RCIN_PULSE_MIN_TICKS,
                        .failsafe_ticks     = TIMER1_MICROS_TO_TICKS(1500),
                        .is_reversed        = false,
                    },
};


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
 * RCIN_Init - Function to initialize RC pins setting and enable PC interrupt
 *             (Based on AVR pin change mechanism).
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t RCIN_Init()
{
    uint8_t ch_idx;

    RCIN_ChannelStatus = 0;
    RCIN_LatestPinValue = 0;
    RCIN_CycUpdateCnt = 0;

    Uart0_Printf(PSTR("[RCIN] Pins:"));

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){
        pinMode(RCIN_Channels[ch_idx].ardu_pin, INPUT);

        Uart0_Printf(PSTR(" %hhu"), RCIN_Channels[ch_idx].ardu_pin);
    }

    Uart0_Println(PSTR(""));

    /* Enable corresponding pin change interrupt for specific RCIN channel */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){
        PC_Setup(RCIN_Channels[ch_idx].pc_grp_idx, RCIN_Channels[ch_idx].mask, true);
    }

    return 0;
}

/**
 * RCIN_SetDirection - Function to reverse the input pulse width.
 *
 * @param   [none]  *p_channel_is_reversed  A boolean array[RCIN_CH_TOTAL]
 *                                          contains the RX pulse direction
 *                                          setting of each channel.
 *                                          (Set to true for reversing the
 *                                          pulse value).
 *
 *
 *
 * @return  [none]
 *
 */
void RCIN_SetDirection(bool *p_channel_is_reversed)
{
    uint8_t ch_idx;

    if(p_channel_is_reversed == NULL)
        return;

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

        if(p_channel_is_reversed[ch_idx] == true)
            RCIN_Channels[ch_idx].is_reversed = true;
        else
            RCIN_Channels[ch_idx].is_reversed = false;
    }
}

/**
 * RCIN_SetNeutral - Function to set RCIN neutral pulse width.
 *
 * @param   [none]  *p_channel_neutral_ticks    uint16_t array[RCIN_CH_TOTAL]
 *                                              contains the RX pulse ticks of neutral point.
 *
 * @return  [none]
 *
 */
void RCIN_SetNeutral(uint16_t *p_channel_neutral_ticks)
{
    uint8_t ch_idx;
    uint16_t ticks;

    if(p_channel_neutral_ticks == NULL)
        return;

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

        ticks = p_channel_neutral_ticks[ch_idx];

        if(RCIN_IS_PULSE_VALID(ticks)){
            RCIN_Channels[ch_idx].neutral_ticks = ticks;
        }
        else{
            RCIN_Channels[ch_idx].neutral_ticks = RCIN_PULSE_MIDDLE_TICKS;
        }
    }
}

/**
 * RCIN_SetMaxMinStick - Function to set MAX and MIN available RX stick positions.
 *
 * @param   [none]  *p_max    uint16_t array[RCIN_CH_TOTAL] measured maximum input pulse width.
 *
 * @param   [none]  *p_min    uint16_t array[RCIN_CH_TOTAL] measured minimum input pulse width.
 *
 * @return  [none]
 *
 */
void RCIN_SetMaxMinStick(uint16_t *p_max, uint16_t *p_min)
{
    uint8_t ch_idx;

    if(p_max == NULL || p_min == NULL)
        return;

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

        if(RCIN_IS_PULSE_VALID(p_max[ch_idx])){
            RCIN_Channels[ch_idx].max_ticks = p_max[ch_idx];
        }
        else{
            RCIN_Channels[ch_idx].max_ticks  = RCIN_PULSE_MAX_TICKS;
        }

        if(RCIN_IS_PULSE_VALID(p_min[ch_idx])){
            RCIN_Channels[ch_idx].min_ticks = p_min[ch_idx];
        }
        else{
            RCIN_Channels[ch_idx].min_ticks  = RCIN_PULSE_MIN_TICKS;
        }
    }
}

/**
 * RCIN_SetFailsafe - Function to set RCIN failsafe pulse width.
 *
 * @param   [none]  *p_channel_failsafe_ticks   uint16_t array[RCIN_CH_TOTAL]
 *                                              contains the RX pulse ticks for failsafe.
 *                                              (Set to 0 for keeping latest success RX pulse
                                                value when RX fail is happened).
 *
 * @return  [none]
 *
 */
void RCIN_SetFailsafe(uint16_t *p_channel_failsafe_ticks)
{
    uint8_t ch_idx;
    uint16_t ticks;

    if(p_channel_failsafe_ticks == NULL)
        return;

    /* Initial ports */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

        ticks = p_channel_failsafe_ticks[ch_idx];

        if(RCIN_IS_PULSE_VALID(ticks)){
            RCIN_Channels[ch_idx].failsafe_ticks = ticks;
        }
        else{
            RCIN_Channels[ch_idx].failsafe_ticks = TIMER1_MICROS_TO_TICKS(0);
        }
    }
}

/**
 * RCIN_ReadChannels - Function to read current detected pulse width
 *                     of each RC channels.
 *
 * @param   [out]       *p_channels     uint16_t array[RCIN_CH_TOTAL] buffer
 *                                      for storing current detected pulse
 *                                      width of RC channels.
 *
 * @return  [uint8_t]   Current PPM/PWM cycle index.
 * @retval  [0~255]
 *
 */
uint8_t RCIN_ReadChannels(uint16_t *p_channels)
{
    uint8_t old_SREG;
    uint8_t ch_idx;

    /* Store current AVR Status register then disable global interrupt */
    old_SREG = SREG;
    cli();

    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){
        p_channels[ch_idx] = RCIN_Channels[ch_idx].pulse_smooth_width;

        if(RCIN_Channels[ch_idx].is_reversed == true
           && RCIN_Channels[ch_idx].pulse_smooth_width != 0){
            p_channels[ch_idx] += ((RCIN_Channels[ch_idx].neutral_ticks - p_channels[ch_idx]) * 2);
        }
    }

    /* Enable global interrupt */
    SREG = old_SREG;

    return RCIN_CycUpdateCnt;
}

/**
 * RCIN_GetChannelsDiff - Function convert absolute pulse width to relative
 *                        offset from RX signal middle point.
 *
 * Say if the input value is 4000 ticks (2000us),
 * the output value will be 4000 - 3000 = 1000 ticks = 500us.
 * If the input value is 2000 ticks (1000us),
 * the output will be 2000 - 3000 = -1000 ticks = -500 us.
 * If the input value is equal to out of MAX/MIN range,
 * the output value will be assigned to "ZERO" directly.
 *
 * @param   [input]     *p_pulse_in     A "UNSIGNED" uint16_t array[RCIN_CH_TOTAL]
 *                                      contains values of RX signal pulse widths.
 *                                      (The unit should be timer tick).
 *
 * @param   [output]    *p_pulse_diff   A SIGNED int16_t array[RCIN_CH_TOTAL] for storing
 *                                      output pulse offset values which are calculated
 *                                      by this function.
 *
 * @return  [none]
 *
 */
void RCIN_GetChannelsDiff(uint16_t *p_pulse_in, int16_t *p_pulse_diff)
{
    uint8_t ch_idx;

    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){
        if(RCIN_IS_PULSE_VALID(p_pulse_in[ch_idx]))
            p_pulse_diff[ch_idx] = (int16_t)(p_pulse_in[ch_idx] - RCIN_Channels[ch_idx].neutral_ticks);
        else
            p_pulse_diff[ch_idx] = 0;
    }
}

/**
 * RCIN_GetChannelsAngle - Function convert absolute pulse width to corresponding
 *                         angle value.
 *
 * Say the 2000 ticks input may be converted to -45 degree.
 * Say the 3000 ticks input may be converted to 0 degree.
 * Say the 4000 ticks input may be converted to 45 degree.
 *
 * @param   [input]     *p_pulse_in     A "UNSIGNED" uint16_t array[RCIN_CH_TOTAL]
 *                                      contains values of RX signal pulse widths.
 *                                      (The unit should be timer tick).
 *
 * @param   [output]    *p_angle        A float array[RCIN_CH_TOTAL] for storing
 *                                      corresponding angle value. If one of the
 *                                      input pulse width is invalid, the output
 *                                      degree will be set to zero by default.
 *
 * @return  [none]
 *
 */
void RCIN_GetChannelsAngle(uint16_t *p_pulse_in, float *p_angle)
{
    uint8_t ch_idx;

    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){
        if(RCIN_IS_PULSE_VALID(p_pulse_in[ch_idx])){
            p_angle[ch_idx] = (float)(p_pulse_in[ch_idx] - RCIN_Channels[ch_idx].neutral_ticks);
            p_angle[ch_idx] *= (float)((TIMER1_MICROS_TO_TICKS(2000) - TIMER1_MICROS_TO_TICKS(1000)) / 90.0);
        }
        else{
            p_angle[ch_idx] = 0.0;
        }
    }
}

/**
 * RCIN_FailChk - Function to check and reset current RC channel
 *                signal status, this function should be called
 *                periodically.
 *
 * The execution time of this function is about 14 ~ 16.75 us.
 *
 * @param   [none]
 *
 * @return  [bool]  RCIN critical error indication.
 *
 * @retval  [0]     There is no any critical error.
 * @retval  [1]     Yes, there is critical error.
 *
 */
bool RCIN_FailChk()
{
    uint8_t ch_idx;
    uint32_t pin_bitmask;
    bool is_error;

    /* Enable (nested) interrupt for reducing measurement delay of other channels */
    sei();

    is_error = false;

    /*
     * Reset channel width setting if the channel have not
     * been updated for a long time.
     */
    for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

        pin_bitmask = (1 << ch_idx);

        if((RCIN_ChannelStatus & pin_bitmask) == 0){

            /* Apply predefined failsafe control setting if needed */
            if(RCIN_Channels[ch_idx].failsafe_ticks != TIMER1_MICROS_TO_TICKS(0))
                RCIN_Channels[ch_idx].pulse_smooth_width = RCIN_Channels[ch_idx].failsafe_ticks;

            is_error = true;
        }
    }

    /* Clear current channel status */
    RCIN_ChannelStatus = 0;

    /* Disable global interrupt before we return to ISR */
    cli();

    return is_error;
}

/**
 * RCIN_PulseHandler - Function to monitor and decode RC input pulse signal.
 *
 * This function should be called in corresponding "Pin change" ISR.
 *
 *
 * @param   [input]     pc_grp_idx      Index of triggered pin change group.
 * @param   [input]     trig_time       Timestamp when interrupt is triggered (microsecond).
 * @param   [input]     pin_status      Current IO pin value (High/Low).
 * @param   [input]     pin_change      Pin change status.
 *
 * @return  [none]
 *
 */
void RCIN_PulseHandler(PC_GRP_IDX pc_grp_idx, uint32_t trig_time,
                       uint8_t pin_status, uint8_t pin_change)
{
    uint8_t ch_idx;
    uint32_t pulse_width;
    RCIN_CHANNEL *p_channel;

    /* Enable (nested) interrupt for reducing measurement delay of other channels */
    sei();

    /*
     * Notice! The global interrupt is already enabled.
     */

    /* Measure channel pulse only if pin value is changed */
    if(pin_change){

        /* Scan and compare all channel status, update related information
         * if the status of specific channel/pin is changed.
         */
        for(ch_idx = 0; ch_idx < RCIN_CH_TOTAL; ch_idx++){

            p_channel = &RCIN_Channels[ch_idx];

            if((pin_change & p_channel->mask) && (pc_grp_idx == p_channel->pc_grp_idx)){

                /* Rising edge, record pulse start time */
                if(pin_status & p_channel->mask){
                    p_channel->pulse_start = trig_time;

                    /* Disable global interrupt temporarily */
                    cli();

                    /* Increase cycle counter if this is the first pulse in new cycle */
                    if((uint8_t)(RCIN_CycUpdateCnt + 1) == p_channel->update_sequence)
                        RCIN_CycUpdateCnt++;
                    else
                        p_channel->update_sequence = RCIN_CycUpdateCnt;

                    /* Enable interrupt again */
                    sei();
                }
                /* Falling edge, update detected pulse width */
                else{

                    pulse_width = trig_time - p_channel->pulse_start;

                    /* Only update pulse value when it is valid */
                    if(RCIN_IS_PULSE_VALID(pulse_width)){

                        /* Disable global interrupt temporarily */
                        cli();

                        /* Apply low-pass filter */
                        if(p_channel->pulse_smooth_width == 0){
                            p_channel->pulse_smooth_width = (int16_t)pulse_width;
                        }
                        else{
                            p_channel->pulse_smooth_width -= ((int16_t)(p_channel->pulse_smooth_width
                                                                        - pulse_width) >> RCIN_LPF_BETA);
                        }

                        p_channel->update_sequence++;

                        /* Enable interrupt again */
                        sei();

                        RCIN_ChannelStatus |= (1 << ch_idx);

                    }
                }

            }
        }
    }

    /* Disable global interrupt before we return to ISR */
    cli();
}

/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */
