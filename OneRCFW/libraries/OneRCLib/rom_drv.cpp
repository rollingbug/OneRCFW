/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    rom_drv.cpp
 *
 * @brief   AVR EEPROM driving functions.
 *          These functions are designed for AVR 328P MCU only.
 *
 *          Abbreviations:
 *              EEPROM     - Electrically-Erasable Programmable Read-Only Memory
 *
 *          Hardware:
 *              EEPROM
 *
 *          Interrupts:
 *              None
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>
#include <avr/io.h>

#include "rom_drv.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#if defined(__AVR_ATmega328P__)
#define ROM_SPACE_SIZE      1024
#else
#error "Unknown MCU type"
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
 * ROM_ReadBytes - Function to read ROM data.
 *
 * @param   [in]        rom_addr        Read ROM address.
 * @param   [in]        *p_data         Buffer for storing ROM data.
 * @param   [in]        data_size       Byte size of output buffer.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t ROM_ReadBytes(uint16_t rom_addr, uint8_t *p_data, uint16_t data_size)
{
    if(p_data == NULL)
        return -1;
    if(data_size > ROM_SPACE_SIZE)
        return -1;
    if(rom_addr > ROM_SPACE_SIZE - data_size)   /* Boundary check */
        return -1;

    while(data_size--){

        /* Make sure previous write procedure is completed */
        while(EECR & _BV(EEPE))
            ;

        /* Setup read address */
        EEAR = rom_addr++;

        /* Start read procedure */
        EECR = _BV(EERE);

        /* Store ROM data to buffer */
        *(p_data++) = EEDR;

    }

    return 0;
}

/**
 * ROM_UpdateBytes - Function to update ROM data.
 *
 * @param   [in]        rom_addr        Write ROM address.
 * @param   [in]        *p_data         Buffer contains new data.
 * @param   [in]        data_size       Byte size of input buffer.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t ROM_UpdateBytes(uint16_t rom_addr, uint8_t *p_data, uint16_t data_size)
{
    volatile uint8_t read_byte;

    if(p_data == NULL)
        return -1;
    if(data_size > ROM_SPACE_SIZE)
        return -1;
    if(rom_addr > ROM_SPACE_SIZE - data_size)   /* Boundary check */
        return -1;

    /*
     * DO NOT block the interrupt during ROM writing procedure!
     */

    while(data_size--){

        /* Make sure previous write procedure is completed */
        while(EECR & _BV(EEPE))
            ;

        /* Setup read address */
        EEAR = rom_addr;

        /* Start read procedure */
        EECR = _BV(EERE);

        /* Read ROM data */
        read_byte = EEDR;

        /*  Compare and update ROM data when needed */
        if(read_byte != *p_data){
            EEDR = *p_data;
            EECR = _BV(EEMPE);  /* Ready to erase and write */
            EECR |= _BV(EEPE);  /* Start procedure */

        }

        p_data++;
        rom_addr++;

    }

    return 0;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */
