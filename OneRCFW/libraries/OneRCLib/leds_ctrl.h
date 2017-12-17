/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    leds_ctrl.h
 * @brief   LED controlling functions.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef LEDS_CTRL_H_
#define LEDS_CTRL_H_


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define LEDS_TOTAL      2


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

enum LEDS_IDX{
    LEDS_MASTER_IDX     = 0,
    LEDS_SLAVE_IDX,
};


/*
 *******************************************************************************
 * Public functions declaration
 *******************************************************************************
 */

void LEDS_Init();
int8_t LEDS_PwrON(uint8_t led_idx);
int8_t LEDS_PwrOFF(uint8_t led_idx);
int8_t LEDS_Blink(uint8_t led_idx, uint32_t on_millis, uint32_t off_millis);
int8_t LEDS_Lightning(uint8_t led_idx, uint32_t off_millis,
                      uint32_t lighting_on_millis, uint32_t lighting_off_millis);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // LEDS_CTRL_H_
