/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    pid.cpp
 * @brief   PID controller (proportional integral derivative controller)
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <string.h>

#include "pid.h"


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
 * PID_Create - Function to create new PID controller.
 *
 * @param   [in]        *p_pid      Core data of PID controller which
 *                                  will be reset by this function.
 *
 * @return  [none]
 *
 */
void PID_Create(PID_DATA *p_pid)
{
    /* DO NOT use memset to reset float variable */
    p_pid->config.KP = 0;
    p_pid->config.KI = 0;
    p_pid->config.KD = 0;
    p_pid->config.scale_factor = 1.0;
    p_pid->config.output_max = 0;
    p_pid->config.integral_max = 0;

    p_pid->value.integral = 0;
    p_pid->value.derivative = 0;
    p_pid->value.delta_time = 0;
    p_pid->value.prev_error = 0;
    p_pid->value.output = 0;
}

/**
 * PID_SetTuning - Function to set KP, KI, and KD gain of specific
 *                 PID controller.
 *
 * @param   [out]       *p_pid      Core data of specific PID controller which
 *                                  will be updated by this function.
 *
 * @param   [in]        KP          New proportional gain setting.
 * @param   [in]        KI          New integral gain setting.
 * @param   [in]        KD          New derivative gain setting.
 *
 * @return  [none]
 *
 */
void PID_SetTuning(PID_DATA *p_pid, float KP, float KI, float KD)
{
    p_pid->config.KP = KP;
    p_pid->config.KI = KI;
    p_pid->config.KD = KD;
}

/**
 * PID_SetScaleFactor - Function to set scale factor of specific PID controller.
 *
 * @param   [out]       *p_pid          Core data of specific PID controller which
 *                                      will be updated by this function.
 *
 * @param   [in]        scale_factor    New scale factor setting.
 *
 * @return  [none]
 *
 */
void PID_SetScaleFactor(PID_DATA *p_pid, float scale_factor)
{
    p_pid->config.scale_factor = scale_factor;
}

/**
 * PID_SetIntegralMax - Function to set maximum/minimum integral value of
 *                      specific PID controller.
 *                      (integral_max >= integral value >= -integral_max)
 *
 * @param   [out]       *p_pid          Core data of specific PID controller which
 *                                      will be updated by this function.
 *
 * @param   [in]        integral_max    MAX/MIN integral threshold setting.
 *
 * @return  [none]
 *
 */
void PID_SetIntegralMax(PID_DATA *p_pid, float integral_max)
{
    p_pid->config.integral_max = integral_max;
}

/**
 * PID_SetOutputMax - Function to set maximum/minimum PID output value of
 *                    specific PID controller.
 *                    (output_max >= output >= -output_max)
 *
 * @param   [out]       *p_pid          Core data of specific PID controller which
 *                                      will be updated by this function.
 *
 * @param   [in]        output_max      MAX/MIN output threshold setting.
 *
 * @return  [none]
 *
 */
void PID_SetOutputMax(PID_DATA *p_pid, float output_max)
{
    p_pid->config.output_max = output_max;
}

/**
 * PID_Reset - Function to reset PID controller.
 *
 * @param   [out]       *p_pid      Core data of specific PID controller which
 *                                  will be updated by this function.
 *
 * @return  [none]
 *
 */
void PID_Reset(PID_DATA *p_pid)
{
    p_pid->value.prev_error = 0;
    p_pid->value.derivative = 0;
    p_pid->value.integral = 0;
}

/**
 * PID_Update - Function to perform PID calculation and generate the ideal
 *              output control value according latest ERROR input and related
 *              gain setting of specific PID controller.
 *
 * @param   [out]       *p_pid          Core data of specific PID controller which
 *                                      will be updated by this function.
 *
 * @param   [in]        error           The latest measured error.
 * @param   [in]        delta_time      The PID delta time.
 * @param   [in]        is_integral_en  Set turn to integrate input error.
 *
 * @return  [float]     Ideal control output which is decided by PID controller.
 *
 */
float PID_Update(PID_DATA *p_pid, float error, uint16_t delta_time, bool is_integral_en)
{
    float delta_t;
    float scale_factor;
    float error_tmp;

    scale_factor = p_pid->config.scale_factor;
    delta_t = delta_time * 0.000001;

    /* Rescale the error if need */
    error_tmp = error * scale_factor;

    /* Calculate derivative */
    p_pid->value.derivative = ((error - p_pid->value.prev_error) / delta_t) * scale_factor;

    /* Calculate integral if needed */
    if(is_integral_en == true){
        p_pid->value.integral += (error * delta_t) * scale_factor;
    }

    /* Check integral range */
    if(p_pid->value.integral > p_pid->config.integral_max)
        p_pid->value.integral = p_pid->config.integral_max;
    else if(p_pid->value.integral < -(p_pid->config.integral_max))
        p_pid->value.integral = -(p_pid->config.integral_max);

    /* PID update */
    p_pid->value.output = error_tmp * p_pid->config.KP
                        + p_pid->value.integral * p_pid->config.KI
                        + p_pid->value.derivative * p_pid->config.KD;

    /* Check output range */
    if(p_pid->value.output > p_pid->config.output_max)
        p_pid->value.output = p_pid->config.output_max;
    else if(p_pid->value.output < -(p_pid->config.output_max))
        p_pid->value.output = -(p_pid->config.output_max);

    p_pid->value.prev_error = error;
    p_pid->value.delta_time = delta_time;

    return p_pid->value.output;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

