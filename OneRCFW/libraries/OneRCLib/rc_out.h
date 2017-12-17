/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    rc_out.h
 * @brief   RC output pulse (PWM control signal) generator.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef RC_OUT_H_
#define RC_OUT_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

/* Channel and pin change definition */
#define RCOUT_CH_TOTAL  4

/* Logic index of RC output channels */
#define RCOUT_THRO_IDX  0
#define RCOUT_AILE_IDX  1
#define RCOUT_ELEV_IDX  2
#define RCOUT_RUDD_IDX  3


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

int8_t RCOUT_Init();
uint8_t RCOUT_GetCycUpdateCnt();
int8_t RCOUT_SetServoPWM(uint16_t *p_channels_width, uint8_t channel_total);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // RC_OUT_H_
