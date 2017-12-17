/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *       A amateur remote control software library. Use at your own risk.
 *
 * @file    mcu_protocol.cpp
 * @brief
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <string.h>

#include "mcu_protocol.h"
#include "uart_drv.h"
#include "uart_stream.h"
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

/* RX frame decoding state Definition */
typedef enum mp_rx_frm_state{
    MP_RX_FRM_WAIT_SFLAG        = 0,
    MP_RX_FRM_WAIT_HDR,
    MP_RX_FRM_WAIT_PAYLOAD,
    MP_RX_FRM_WAIT_CRC,
}MP_RX_FRM_STATE;


/*
 *******************************************************************************
 * Macros definition
 *******************************************************************************
 */


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

static uint8_t MP_TxSequence;

/* The following variables are created for storing information of receiving frame */
static MP_RX_FRM_STATE MP_RxFrmState;           /* Current RX frame function state */
static uint8_t MP_RxSequence;                   /* Current RX frame sequence */
static uint8_t MP_RxFrmBufIdx;                  /* Current write index of RX buffer */
static uint8_t MP_RxFrmBuf[MP_RX_FRM_BUF_SIZE]; /* RX frame buffer */
static uint8_t MP_RxFrmPayloadLen;              /* Expected payload length of RX frame */
static uint8_t MP_RxFrmCrcIdx;                  /* Total collected CRC byte */
static uint16_t MP_RxFrmCrc16;                  /* Expected RX frame checksum */
static bool MP_RXFrmIsLost;                     /* Frame lost indicator */


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
 * MP_Init - Function to initialize MCU protocol.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t MP_Init()
{
    MP_TxSequence = 0;

    MP_RxSequence = 0;
    MP_RxFrmBufIdx = 0;
    MP_RxFrmPayloadLen = 0;
    MP_RxFrmCrcIdx = 0;
    MP_RxFrmCrc16 = CRC_INIT_VAL;
    MP_RxFrmState = MP_RX_FRM_WAIT_SFLAG;
    MP_RXFrmIsLost = false;
    memset((void *)MP_RxFrmBuf, 0, sizeof(MP_RxFrmBuf));

    Uart0_Println(PSTR("[MP] OK"));

    return 0;
}

/**
 * MP_Send - Function to send MP frame.
 *
 * @param   [in]        cmd         MP command ID (Refer to enum mp_rsp_cmd).
 * @param   [in]        *p_data     Frame data buffer.
 * @param   [in]        data_size   Size of frame data.
 *
 * @return  [uint8_t]   Transmitted data size.
 * @retval  [0]         Fail.
 * @retval  [1~255]     Transmitted bytes.
 *
 */
uint8_t MP_Send(uint8_t cmd, uint8_t *p_data, uint8_t data_size)
{
    uint16_t tx_bytes;
    uint16_t crc;
    MP_FRAME_HDR hdr;
    MP_FRAME_TAIL tail;

    if(p_data == NULL || data_size == 0)
        return 0;

    if(sizeof(hdr) + data_size + sizeof(tail) > 255)
        return 0;

    /* Header */
    hdr.s_flag = MP_FRM_SFLAG;
    hdr.cmd = cmd;
    hdr.sequence = MP_TxSequence++;
    hdr.len = data_size;

    /* Tail */
    crc = CRC_INIT_VAL;
    crc = CRC_AccumulateLoop(&hdr.cmd, sizeof(hdr) - sizeof(hdr.s_flag), crc);
    crc = CRC_AccumulateLoop(p_data, data_size, crc);
    tail.CRC16 = crc;

    tx_bytes = 0;

    tx_bytes += Uart0_WriteBytes((uint8_t *)&hdr, sizeof(hdr));
    tx_bytes += Uart0_WriteBytes(p_data, data_size);
    tx_bytes += Uart0_WriteBytes((uint8_t *)&tail, sizeof(tail));

    return tx_bytes;
}

/**
 * MP_Recv - Function to receive MP frame.
 *
 * This function should be called periodically in the system loop.
 *
 * @param   [in/out]    *p_frm_buf      A buffer to store received MP frame content.
 * @param   [in]        frm_buf_size    Byte size of input frame buffer.
 *
 * @return  [uint8_t]   Total received frame size.
 * @retval  [0]         No RX frame.
 * @retval  [1~N]       Byte size of received MP frame (Header + Payload + Checksum).
 *
 */
uint8_t MP_Recv(uint8_t *p_frm_buf, uint8_t frm_buf_size)
{
    uint8_t current_rx_cnt;
    uint8_t total_frm_size;
    uint8_t data_byte;
    MP_FRAME_HDR *p_frm_hdr;
    MP_FRAME_TAIL *p_frm_tail;

    current_rx_cnt = 0;
    total_frm_size = 0;

    /*
     * Process received byte, but break this loop once we received numbers of
     * frame data in case the keep comping data cause endless loop.
     */
    while(Uart0_ReadByte(&data_byte) && current_rx_cnt < MP_RX_FRM_BUF_SIZE){

        /* Drop all collected frame data if the frame length is larger than our local buffer size */
        if(MP_RxFrmBufIdx == MP_RX_FRM_BUF_SIZE)
            MP_RxFrmState = MP_RX_FRM_WAIT_SFLAG;

        switch(MP_RxFrmState){

            /* Detecting frame state flag */
            case MP_RX_FRM_WAIT_SFLAG:

                MP_RxFrmBufIdx = 0;
                MP_RxFrmPayloadLen = 0;
                MP_RxFrmCrcIdx = 0;
                MP_RxFrmCrc16 = CRC_INIT_VAL;

                if(data_byte == MP_FRM_SFLAG){

                    /* Store frame data in local buffer */
                    MP_RxFrmBuf[MP_RxFrmBufIdx] = data_byte;
                    MP_RxFrmBufIdx++;

                    MP_RxFrmState = MP_RX_FRM_WAIT_HDR;
                }

                break;

            /* Collecting frame header */
            case MP_RX_FRM_WAIT_HDR:

                /* Store frame data in local buffer */
                MP_RxFrmBuf[MP_RxFrmBufIdx] = data_byte;
                MP_RxFrmBufIdx++;

                MP_RxFrmCrc16 = CRC_Accumulate(data_byte, MP_RxFrmCrc16);

                if(MP_RxFrmBufIdx == sizeof(MP_FRAME_HDR)){
                    p_frm_hdr = (MP_FRAME_HDR *)MP_RxFrmBuf;

                    MP_RxFrmPayloadLen = p_frm_hdr->len;

                    if(MP_RxSequence != p_frm_hdr->sequence)
                        MP_RXFrmIsLost = true;

                    MP_RxSequence = p_frm_hdr->sequence;

                    if(p_frm_hdr->len == 0)
                        MP_RxFrmState = MP_RX_FRM_WAIT_CRC;
                    else
                        MP_RxFrmState = MP_RX_FRM_WAIT_PAYLOAD;
                }

                break;

            /* Collecting frame payload */
            case MP_RX_FRM_WAIT_PAYLOAD:

                /* Store frame data in local buffer */
                MP_RxFrmBuf[MP_RxFrmBufIdx] = data_byte;
                MP_RxFrmBufIdx++;
                MP_RxFrmPayloadLen--;

                MP_RxFrmCrc16 = CRC_Accumulate(data_byte, MP_RxFrmCrc16);

                if(MP_RxFrmPayloadLen == 0){
                    MP_RxFrmState = MP_RX_FRM_WAIT_CRC;
                }

                break;

            case MP_RX_FRM_WAIT_CRC:

                /* Store frame data in local buffer */
                MP_RxFrmBuf[MP_RxFrmBufIdx] = data_byte;
                MP_RxFrmBufIdx++;
                MP_RxFrmCrcIdx++;

                if(MP_RxFrmCrcIdx == sizeof(((MP_FRAME_TAIL *)0)->CRC16)){

                    p_frm_tail = (MP_FRAME_TAIL *)&MP_RxFrmBuf[MP_RxFrmBufIdx - MP_RxFrmCrcIdx];

                    if(p_frm_tail->CRC16 == MP_RxFrmCrc16){

                        /* Copy frame data to external buffer */
                        if(p_frm_buf != NULL && frm_buf_size >= MP_RxFrmBufIdx){
                            total_frm_size = MP_RxFrmBufIdx;
                            memcpy((void *)p_frm_buf, MP_RxFrmBuf, total_frm_size);
                        }

                    }

                    MP_RxFrmState = MP_RX_FRM_WAIT_SFLAG;
                }

                break;

            default:
                break;
        }

        if(total_frm_size != 0)
            break;

        current_rx_cnt++;
    }

    return total_frm_size;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

