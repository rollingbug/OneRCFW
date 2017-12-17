/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    pid.cpp
 * @brief   PID controller (proportional integral derivative controller)
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#ifndef PID_H_
#define PID_H_

#include <stdint.h>


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

typedef struct pid_value{
    float integral;
    float derivative;
    float output;
    float prev_error;
    uint16_t delta_time;
}PID_VALUE;

typedef struct pid_config{
    float KP;
    float KI;
    float KD;
    float scale_factor;

    float integral_max;
    float output_max;
}PID_CONFIG;

typedef struct pid_data{
    PID_VALUE value;
    PID_CONFIG config;
}PID_DATA;


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

void PID_Create(PID_DATA *p_pid);
void PID_SetTuning(PID_DATA *p_pid, float KP, float KI, float KD);
void PID_SetScaleFactor(PID_DATA *p_pid, float scale_factor);
void PID_SetIntegralMax(PID_DATA *p_pid, float integral_max);
void PID_SetOutputMax(PID_DATA *p_pid, float output_max);
void PID_Reset(PID_DATA *p_pid);
float PID_Update(PID_DATA *p_pid, float error, uint16_t delta_time, bool is_integral_en);


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */


#endif // PID_H_
