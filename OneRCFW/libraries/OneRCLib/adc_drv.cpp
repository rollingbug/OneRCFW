/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    adc_drv.cpp
 * @brief   ADC functions.
 *
 *          Reference:
 *              Measuring VCC via the bandgap
 *              https://jeelabs.org/2012/05/04/measuring-vcc-via-the-bandgap/
 *
 *          Abbreviations:
 *              ADC     - Analog-to-Digital Converter
 *
 *          Pin/Channel mapping:
 *              ADC CH 0 = Arduino A6 / Analog
 *              ADC CH 1 = Arduino A1 / AVR PC1
 *              ADC CH 2 = Arduino A2 / AVR PC2
 *              ADC CH 3 = Arduino A3 / AVR PC3
 *              ADC CH 4 = Arduino A7 / Analog
 *
 *          Hardware:
 *              Analog-to-Digital Converter
 *
 *          Interrupts:
 *              None
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <Arduino.h>

#include "uart_stream.h"
#include "adc_drv.h"
#include "timers_drv.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define ADC_CH0_ARDU_PIN    A6
#define ADC_CH1_ARDU_PIN    A1
#define ADC_CH2_ARDU_PIN    A2
#define ADC_CH3_ARDU_PIN    A3
#define ADC_CH4_ARDU_PIN    A7

#define ADC_ANALOG_VOLTAGE  5.0     /* External analog VCC */
#define ADC_BANDGAP_VOLTAGE 1.1     /* Internal bandgap voltage */


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
 * Private functions declaration
 *******************************************************************************
 */

static void ADC_SetMux(uint8_t adc_channel);
static uint16_t ADC_SingleConversion();


/*
 *******************************************************************************
 * Public functions implementation
 *******************************************************************************
 */

/**
 * ADC_Init - Function to initialize Atmel ADC pins and registers.
 *
 * @param   [none]
 * @return  [none]
 *
 */
int8_t ADC_Init()
{
    /* ADC pins initialization */
    pinMode(ADC_CH0_ARDU_PIN, INPUT);
    pinMode(ADC_CH1_ARDU_PIN, INPUT);
    pinMode(ADC_CH2_ARDU_PIN, INPUT);
    pinMode(ADC_CH3_ARDU_PIN, INPUT);
    pinMode(ADC_CH4_ARDU_PIN, INPUT);

    /* Disable all interrupts */
    cli();

    /*
     * ADMUX - ADC Multiplexer Selection Register
     * Voltage Reference Selection: AVCC
     * ADLAR = 0, (Right adjust)
     * MUX[3:0] = 0 by default
     */
    ADMUX = _BV(REFS0);

    /*
     * ADCSRA - ADC Control and Status Register A
     * ADC Enable = 1,
     * ADC Start Conversion = 0,
     * ADC Auto Trigger Enable = 0,
     * ADC Interrupt Flag = 1 (To clear interrupt trigger)
     * ADIE: ADC Interrupt Enable = 0,
     * ADC Prescaler = 64 (16 Mhz / 64 = 250 KHz)
     */
    ADCSRA = (_BV(ADEN) | (_BV(ADPS2) | _BV(ADPS1)));

    /*
     * ADCSRB - ADC Control and Status Register B
     */
    ADCSRB = 0;

    /*
     * DIDR0 - Digital Input Disable Register 0
     */
    DIDR0 = ((_BV(ADC_CH0) | _BV(ADC_CH1) | _BV(ADC_CH2) | _BV(ADC_CH3) | _BV(ADC_CH4)) & 0x3F);

    /* Enable all interrupts */
    sei();

    Uart0_Println(PSTR("[ADC] A6, A1, A2, A3, A7"));

    /* Display current MCU voltage(AVCC) */
    ADC_SetMuxTo1V1();
    Timer1_DelayMillis(5);
    Uart0_Println(PSTR("[ADC] AVCC: %f"), ADC_ReadSysVoltage());

    return 0;
}

/**
 * ADC_Read - Function to trigger ADC single conversion and read ADC value.
 *
 * Notice! This function require about 57 us execution time.
 *
 * @param   [in]            adc_channel     Select ADC channel.
 *
 * @return  [uint16_t]      digital value of specific ADC channel.
 * @retval  [0~1023]
 *
 */
uint16_t ADC_Read(uint8_t adc_channel)
{
    ADC_SetMux(adc_channel);

    /* Return the ADC raw data */
    return ADC_SingleConversion();
}

/**
 * ADC_SetMuxTo1V1 -
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
void ADC_SetMuxTo1V1()
{
    ADC_SetMux(ADC_CH_1V1);
}

/**
 * ADC_ReadSysVoltage - Calculate current voltage of AVCC pin.
 *
 * We change ADC MUX to 1V1 (bandgap) and compare the voltage difference between AVCC and 1V1.
 *
 * @param   [none]
 *
 * @return  [float]     digital value of specific ADC channel.
 * @retval  [0~N]       AVCC voltage
 *
 */
float ADC_ReadSysVoltage()
{
    /* Assume we already set MUX to 1V1 before, so let's start ADC single conversion directly. */
    return (ADC_BANDGAP_VOLTAGE * 1024.0) / (float)ADC_SingleConversion();
}


/*
 *******************************************************************************
 * Private functions declaration
 *******************************************************************************
 */

/**
 * ADC_SetMux - Set ADC MUX to specific channel.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static void ADC_SetMux(uint8_t adc_channel)
{
    /* Setup MUX for specific channel */
    ADMUX &= ~(_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0));
    ADMUX |= (adc_channel & (_BV(MUX3) | _BV(MUX2) | _BV(MUX1) | _BV(MUX0)));
}

/**
 * ADC_SingleConversion - Start ADC single conversion.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static uint16_t ADC_SingleConversion()
{
    uint8_t timeout;

    timeout = 0xFF;

    /* ADC start single conversion */
    ADCSRA |= _BV(ADSC);

    /* Wait till single conversion is completed (requires about 57 us) */
    while((ADCSRA & _BV(ADSC))){

        /* Timeout check, should never expired */
        timeout--;
        if(timeout == 0){
            return 0;
        }
    }

    /* Return the ADC raw data */
    return ADC;
}


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */
