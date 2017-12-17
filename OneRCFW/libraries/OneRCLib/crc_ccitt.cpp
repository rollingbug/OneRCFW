/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    crc_ccitt.cpp
 *
 * @brief   CRC-16-CCITT (based on Mavlink method):
 *              - Polynomial = 0x1021 => X^16 + X^12 + X^5 + 1
 *              - Initial value = 0xFFFF
 *              - With "reverse data bytes" & "reverse CRC result before final XOR"
 *
 * https://en.wikipedia.org/wiki/Cyclic_redundancy_check#Commonly_used_and_standardized_CRCs
 * https://en.wikipedia.org/wiki/MAVLink
 * http://www.sunshine2k.de/coding/javascript/crc/crc_js.html
 * ftp://gic.dsc.ulpgc.es/Gic/PFCs/Memoria_TFM_Dima.pdf
 * https://github.com/marhar/eastbay-mavlink/blob/master/tests/demo2.py
 * http://eastbay-rc.blogspot.tw/2013/04/mavlink-protocol-notes-packet-decoding.html
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <stdint.h>

#include "crc_ccitt.h"


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
 * CRC_Accumulate - Function to accumulate CRC of input single byte data.
 *                  (CRC-16-CCITT)
 *
 * @param   [in]        data        Single byte data
 * @param   [in]        accum_crc   Current accumulate CRC value.
 *                                  The accum_crc should be initialized to
 *                                  CRC_INIT_VAL for first byte data.
 *
 * @return  [uint16_t]  CRC value
 *
 */
uint16_t CRC_Accumulate(uint8_t data, uint16_t accum_crc)
{
    uint8_t tmp;

    tmp = data ^ (uint8_t)(accum_crc & 0xFF);
    tmp ^= (tmp << 4);
    accum_crc = (accum_crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4);

    return accum_crc;
}

/**
 * CRC_Accumulate - Function to accumulate CRC of input multi byte data.
 *                  (CRC-16-CCITT)
 *
 * @param   [in]        *p_data     Multu byte data.
 * @param   [in]        data_size   Size of input data.
 * @param   [in]        accum_crc   Current accumulate CRC value.
 *                                  The accum_crc should be initialized to
 *                                  CRC_INIT_VAL for first byte data.
 *
 * @return  [uint16_t]  CRC value
 *
 */
uint16_t CRC_AccumulateLoop(uint8_t *p_data, uint8_t data_size, uint16_t accum_crc)
{
    while(data_size){

        accum_crc = CRC_Accumulate(*p_data++, accum_crc);

        data_size--;
    }

    return accum_crc;
}

/**
 * CRC_Calculate - Function to calculate CRC (CRC-16-CCITT) of data.
 *
 * @param   [in]        *p_data     Point to data buffer.
 * @param   [in]        data_size   Total byte of input data
 *
 * @return  [uint16_t]  CRC value
 *
 */
uint16_t CRC_Calculate(uint8_t *p_data, uint8_t data_size)
{
    uint16_t crc;

    crc = CRC_INIT_VAL;

    return CRC_AccumulateLoop(p_data, data_size, crc);
}




/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */
