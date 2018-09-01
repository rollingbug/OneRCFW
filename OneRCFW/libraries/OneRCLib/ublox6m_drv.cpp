/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    ublox6m_drv.cpp
 * @brief   Ublox NEO 6M GPS module driver.
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <stdint.h>

#include "ublox6m_drv.h"
#include "uart_stream.h"
#include "uart_sim.h"
#include "timers_drv.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define UBLOX6M_RX_TIMEOUT_MS   1000


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* State of UBX receiver */
typedef enum ublox6m_rx_state{
    UBLOX6M_RX_WAIT_SYNC_1 = 0,
    UBLOX6M_RX_WAIT_SYNC_2,
    UBLOX6M_RX_WAIT_CLASS_ID,
    UBLOX6M_RX_WAIT_MSG_ID,
    UBLOX6M_RX_WAIT_LENGTH,
    UBLOX6M_RX_WAIT_PAYLOAD,
    UBLOX6M_RX_WAIT_CHKSUM,
}UBLOX6M_RX_STATE;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

static uint8_t ublox6m_RxBufIdx;
static uint8_t ublox6m_RxPayloadLenIdx;
static uint16_t ublox6m_RxPayloadLen;
static uint8_t ublox6m_RxChksumIdx;
static UBLOX6M_RX_STATE ublox6m_RxState;


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

static uint16_t ublox6m_SendUBX(uint8_t class_id, uint8_t msg_id,
                                uint8_t *p_payload, uint16_t length);
static uint16_t ublox6m_WaitUBXData(uint16_t expected_payload_size,
                                    uint8_t class_id, uint8_t msg_id,
                                    uint8_t *p_frm_buf, uint16_t frm_buf_size);
static uint16_t ublox6m_WaitUBXAck(uint8_t expected_class_id, uint8_t expected_msg_id,
                                   uint8_t *p_frm_buf, uint16_t frm_buf_size);
static uint16_t ublox6m_RecvUBX(uint8_t *p_frm_buf, uint16_t frm_buf_size);
static uint16_t ublox6m_CalChecksum(uint8_t *p_data, uint16_t length,
                                    uint16_t init_chksum);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * ublox6m_SendUBX - Function to initialize Ublox NEO 6M GPS module.
 *
 * @param   [none]
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 */
int8_t ublox6m_Init()
{
    uint8_t tmp_idx;
    uint8_t ubx_frm_buf[128];
    uint16_t rx_ubx_size;
    uint8_t device_name[] = UBLOX6M_DEV_NAME;
    uint8_t nmea_msg_rate[][2] = UBLOX6M_NMEA_RATE;
    uint8_t nmea_msg_cnt = sizeof(nmea_msg_rate) / sizeof(nmea_msg_rate[0]);
    UBLOX6M_MSG_HDR *p_hdr;
    UBLOX6M_PL_MON_VER_ACCESS *p_mon_ver_payload;
    UBLOX6M_PL_CFG_MSGS_ACCESS *p_cfg_msgs_payload;
    UBLOX6M_PL_CFG_RATE_ACCESS *p_cfg_rate_payload;

    /* Initialize general variables */
    ublox6m_RxBufIdx = 0;
    ublox6m_RxPayloadLenIdx = 0;
    ublox6m_RxPayloadLen = 0;
    ublox6m_RxChksumIdx = 0;
    ublox6m_RxState = UBLOX6M_RX_WAIT_SYNC_1;

    p_hdr = (UBLOX6M_MSG_HDR *)ubx_frm_buf;

    /*
     * Access GPS module software and hardware version.
     */
    ublox6m_SendUBX(UBLOX6M_MSG_CLASS_MON, UBLOX6M_MSG_MON_VER, NULL, 0);

    rx_ubx_size = ublox6m_WaitUBXData(sizeof(UBLOX6M_PL_MON_VER_ACCESS),
                                      UBLOX6M_MSG_CLASS_MON, UBLOX6M_MSG_MON_VER,
                                      ubx_frm_buf, sizeof(ubx_frm_buf));

    /* Display GPS module software and hardware version */
    if(rx_ubx_size == 0){

        Uart0_Println(PSTR("[%s] Unable to access version"), device_name);

        return -1;
    }
    else{
        p_mon_ver_payload = (UBLOX6M_PL_MON_VER_ACCESS *)(p_hdr->payload);
        p_mon_ver_payload->sw_version[sizeof(p_mon_ver_payload->sw_version) - 1] = '\0';
        p_mon_ver_payload->hw_version[sizeof(p_mon_ver_payload->hw_version) - 1] = '\0';
        Uart0_Println(PSTR("[%s] SW: %s, HW: %s"),
                    device_name,
                    p_mon_ver_payload->sw_version,
                    p_mon_ver_payload->hw_version);
    }

    /*
     * Enable/Disable specific NMEA message by change the message output rate to 0 or N.
     * We enable GPGGA and GPRMC message by default.
     */
    for(tmp_idx = 0; tmp_idx < nmea_msg_cnt; tmp_idx++){

        p_cfg_msgs_payload = (UBLOX6M_PL_CFG_MSGS_ACCESS *)(p_hdr->payload);

        p_cfg_msgs_payload->cfg_msg_class = UBLOX6M_MSG_CLASS_NEMA_STD;
        p_cfg_msgs_payload->cfg_msg_id = nmea_msg_rate[tmp_idx][0];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_I2C] = nmea_msg_rate[tmp_idx][1];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_UART0] = nmea_msg_rate[tmp_idx][1];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_UART1] = nmea_msg_rate[tmp_idx][1];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_USB] = nmea_msg_rate[tmp_idx][1];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_SPI] = nmea_msg_rate[tmp_idx][1];
        p_cfg_msgs_payload->cfg_msg_rate[UBLOX6M_PORT_RESERVED] = nmea_msg_rate[tmp_idx][1];

        ublox6m_SendUBX(UBLOX6M_MSG_CLASS_CFG, UBLOX6M_MSG_CFG_MSG,
                        (uint8_t *)p_cfg_msgs_payload, sizeof(UBLOX6M_PL_CFG_MSGS_ACCESS));

        rx_ubx_size = ublox6m_WaitUBXAck(UBLOX6M_MSG_CLASS_CFG, UBLOX6M_MSG_CFG_MSG,
                                         ubx_frm_buf, sizeof(ubx_frm_buf));
        if(rx_ubx_size == 0){

            break;
        }
    }

    Uart0_Printf(PSTR("[%s] Set NMEA: "), device_name);
    if(tmp_idx != nmea_msg_cnt){

        Uart0_Println(PSTR("Fail"));

        return -1;
    }
    else{
        Uart0_Println(PSTR("OK"));
    }

    /*
     * set GPS measurement rate to 200 ms (5 Hz) by default.
     */
    p_cfg_rate_payload = (UBLOX6M_PL_CFG_RATE_ACCESS *)(p_hdr->payload);
    p_cfg_rate_payload->measRate = UBLOX6M_MEAS_GPS_RATE;
    p_cfg_rate_payload->navRate = UBLOX6M_MEAS_NAV_RATE;
    p_cfg_rate_payload->timeRef = UBLOX6M_MEAS_TIME_REF;
    ublox6m_SendUBX(UBLOX6M_MSG_CLASS_CFG, UBLOX6M_MSG_CFG_RATE,
                    (uint8_t *)p_cfg_rate_payload, sizeof(UBLOX6M_PL_CFG_RATE_ACCESS));

    rx_ubx_size = ublox6m_WaitUBXAck(UBLOX6M_MSG_CLASS_CFG, UBLOX6M_MSG_CFG_RATE,
                                     ubx_frm_buf, sizeof(ubx_frm_buf));

    Uart0_Printf(PSTR("[%s] "), device_name);
    if(rx_ubx_size == 0){

        Uart0_Println(PSTR("Unable to set GPS rate"));

        return -1;
    }
    else{
        Uart0_Println(PSTR("GPS rate: %hu ms"), UBLOX6M_MEAS_GPS_RATE);
    }

    return 0;
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * ublox6m_SendUBX - Function to transmit specific UBX frame.
 *
 * @param   [in]        class_id        Class ID of TX UBX frame.
 * @param   [in]        msg_id          Message ID of TX UBX frame.
 * @param   [in]        *p_payload      A buffer to store TX UBX frame payload without checksum.
 * @param   [in]        length          Length of TX UBX frame payload without checksum.
 *
 * @return  [uint16_t]  Total transmitted frame size.
 * @retval  [0~N]       Total transmitted frame size.
 */
static uint16_t ublox6m_SendUBX(uint8_t class_id, uint8_t msg_id,
                                uint8_t *p_payload, uint16_t length)
{
    UBLOX6M_MSG_HDR msg_hdr;
    UBLOX6M_MSG_TAIL msg_tail;
    uint16_t checksum;
    uint16_t tx_bytes;

    if(p_payload == NULL && length != 0)
        return -1;

    if(length > 0xFF)
        return -1;

    tx_bytes = 0;

    msg_hdr.sync_char1 = UBLOX6M_UBX_HDR_SYNC1;
    msg_hdr.sync_char2 = UBLOX6M_UBX_HDR_SYNC2;
    msg_hdr.msg_class = class_id;
    msg_hdr.msg_id = msg_id;
    msg_hdr.length = length;

    checksum = 0;
    checksum = ublox6m_CalChecksum((uint8_t *)&msg_hdr.msg_class,
                                   sizeof(msg_hdr) - sizeof(msg_hdr.sync_char1) - sizeof(msg_hdr.sync_char2),
                                   checksum);
    checksum = ublox6m_CalChecksum(p_payload, length, checksum);

    msg_tail.chksum = checksum;

    tx_bytes += UartS_WriteBytes((uint8_t *)&msg_hdr, sizeof(msg_hdr));
    tx_bytes += UartS_WriteBytes(p_payload, (uint8_t)length);
    tx_bytes += UartS_WriteBytes((uint8_t *)&msg_tail, sizeof(msg_tail));

    return tx_bytes;
}

/**
 * ublox6m_WaitUBXData - Function to receive specific UBX frame with timeout mechanism.
 *                      (The timeout setting is 1000 ms by default).
 *
 * @param   [in]        expected_payload_size   Length of expected RX UBX frame.
 * @param   [in]        class_id                Class ID of expected RX UBX frame.
 * @param   [in]        msg_id                  Message ID of expected RX UBX frame.
 * @param   [in]        *p_frm_buf              A buffer to store received UBX frame content.
 * @param   [in]        frm_buf_size            Size of frame buffer.
 *
 * @return  [uint16_t]  Total received frame size.
 * @retval  [0]         No RX frame before timeout.
 * @retval  [1~N]       Byte size of received UBX frame (Header + Payload + Checksum).
 */
static uint16_t ublox6m_WaitUBXData(uint16_t expected_payload_size,
                                    uint8_t class_id, uint8_t msg_id,
                                    uint8_t *p_frm_buf, uint16_t frm_buf_size)
{
    uint16_t frm_size;
    uint32_t wait_start;
    UBLOX6M_MSG_HDR *p_hdr;

    frm_size = 0;
    wait_start = Timer1_GetMillis();

    while(Timer1_GetMillis() - wait_start < UBLOX6M_RX_TIMEOUT_MS){

        frm_size = ublox6m_RecvUBX(p_frm_buf, frm_buf_size);

        if(frm_size != 0){
            p_hdr = (UBLOX6M_MSG_HDR *)p_frm_buf;

            if(p_hdr->length == expected_payload_size){

                if(p_hdr->msg_class == class_id && p_hdr->msg_id == msg_id){
                    break;
                }

            }
        }
    }

    return frm_size;
}

/**
 * ublox6m_WaitUBXAck - Function to receive UBX ACK frame with timeout mechanism.
 *                      (The timeout setting is 1000 ms by default).
 *
 * @param   [in]        expected_class_id       Expected Class ID of the Acknowledged Message.
 * @param   [in]        expected_msg_id         Expected Message ID of the Acknowledged Message.
 * @param   [in]        *p_frm_buf              A buffer to store received UBX ACK frame content.
 * @param   [in]        frm_buf_size            Size of frame buffer.
 *
 * @return  [uint16_t]  Total received frame size.
 * @retval  [0]         No RX frame before timeout.
 * @retval  [1~N]       Byte size of received UBX ACK frame (Header + Payload + Checksum).
 */
static uint16_t ublox6m_WaitUBXAck(uint8_t expected_class_id, uint8_t expected_msg_id,
                                   uint8_t *p_frm_buf, uint16_t frm_buf_size)
{
    uint16_t rx_ubx_size;
    UBLOX6M_MSG_HDR *p_hdr;
    UBLOX6M_PL_CFG_ACK *p_ack_payload;

    rx_ubx_size = ublox6m_WaitUBXData(sizeof(UBLOX6M_PL_CFG_ACK),
                                      UBLOX6M_MSG_CLASS_ACK, UBLOX6M_MSG_ACK_ACK,
                                      p_frm_buf, frm_buf_size);

    if(rx_ubx_size != 0){
        p_hdr = (UBLOX6M_MSG_HDR *)p_frm_buf;
        p_ack_payload = (UBLOX6M_PL_CFG_ACK *)p_hdr->payload;

        if(p_ack_payload->ack_msg_class != expected_class_id
           || p_ack_payload->ack_msg_id != expected_msg_id){

            rx_ubx_size = 0;
        }
    }

    return rx_ubx_size;
}

/**
 * ublox6m_RecvUBX - Function to collect UBX frame byte and output a completed RX frame.
 *
 * @param   [in]        *p_frm_buf      A buffer to store received UBX frame content.
 * @param   [in]        frm_buf_size    Size of frame buffer.
 *
 * @return  [uint16_t]  Total received frame size.
 * @retval  [0]         No RX frame before timeout.
 * @retval  [1~N]       Byte size of received UBX frame (Header + Payload + Checksum).
 *
 */
static uint16_t ublox6m_RecvUBX(uint8_t *p_frm_buf, uint16_t frm_buf_size)
{
    UBLOX6M_MSG_HDR *p_hdr;
    UBLOX6M_MSG_TAIL *p_tail;
    uint16_t current_rx_cnt;
    uint16_t total_frm_size;
    uint8_t data_byte;
    uint16_t checksum;

    current_rx_cnt = 0;
    total_frm_size = 0;

    /*
     * Process received byte, but break this loop once we received numbers of
     * frame data in case the keep comping data cause endless loop.
     */
    while(UartS_ReadByte(&data_byte) && current_rx_cnt < frm_buf_size){

        /* Restart when exceeds frame buffer size limitation */
        if(ublox6m_RxBufIdx == frm_buf_size)
            ublox6m_RxState = UBLOX6M_RX_WAIT_SYNC_1;

        switch(ublox6m_RxState){

            /* Wait for first flag of UBX header. */
            case UBLOX6M_RX_WAIT_SYNC_1:

                ublox6m_RxBufIdx = 0;

                if(data_byte == UBLOX6M_UBX_HDR_SYNC1){

                    ublox6m_RxPayloadLenIdx = 0;
                    ublox6m_RxPayloadLen = 0;
                    ublox6m_RxChksumIdx = 0;

                    p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                    ublox6m_RxBufIdx++;
                    ublox6m_RxState = UBLOX6M_RX_WAIT_SYNC_2;
                }

                break;

            /* Wait for second flag of UBX header. */
            case UBLOX6M_RX_WAIT_SYNC_2:

                if(data_byte == UBLOX6M_UBX_HDR_SYNC2){
                    p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                    ublox6m_RxBufIdx++;
                    ublox6m_RxState = UBLOX6M_RX_WAIT_CLASS_ID;
                }
                else{
                    ublox6m_RxState = UBLOX6M_RX_WAIT_SYNC_1;
                }

                break;

            /* Wait for class ID of UBX header. */
            case UBLOX6M_RX_WAIT_CLASS_ID:

                p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                ublox6m_RxBufIdx++;

                ublox6m_RxState = UBLOX6M_RX_WAIT_MSG_ID;

                break;

            /* Wait for message ID of UBX header. */
            case UBLOX6M_RX_WAIT_MSG_ID:

                p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                ublox6m_RxBufIdx++;

                ublox6m_RxState = UBLOX6M_RX_WAIT_LENGTH;

                break;

            /* Wait for length (2 bytes) of UBX header. */
            case UBLOX6M_RX_WAIT_LENGTH:

                p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                ublox6m_RxBufIdx++;
                ublox6m_RxPayloadLenIdx++;

                if(ublox6m_RxPayloadLenIdx == 2){

                    p_hdr = (UBLOX6M_MSG_HDR *)p_frm_buf;
                    ublox6m_RxPayloadLen = p_hdr->length;

                    if(p_hdr->length == 0)
                        ublox6m_RxState = UBLOX6M_RX_WAIT_CHKSUM;
                    else
                        ublox6m_RxState = UBLOX6M_RX_WAIT_PAYLOAD;
                }

                break;

            /* Collect payload (N bytes) of UBX frame. */
            case UBLOX6M_RX_WAIT_PAYLOAD:

                p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                ublox6m_RxBufIdx++;
                ublox6m_RxPayloadLen--;

                if(ublox6m_RxPayloadLen == 0)
                    ublox6m_RxState = UBLOX6M_RX_WAIT_CHKSUM;

                break;

            /* Collect checksum (2 bytes) of UBX frame. */
            case UBLOX6M_RX_WAIT_CHKSUM:

                p_frm_buf[ublox6m_RxBufIdx] = data_byte;
                ublox6m_RxBufIdx++;
                ublox6m_RxChksumIdx++;

                if(ublox6m_RxChksumIdx == 2){

                    p_hdr = (UBLOX6M_MSG_HDR *)p_frm_buf;
                    p_tail = (UBLOX6M_MSG_TAIL *)(p_frm_buf + sizeof(UBLOX6M_MSG_HDR) + p_hdr->length);

                    checksum = 0;
                    checksum = ublox6m_CalChecksum((uint8_t *)(&p_hdr->msg_class),
                                                   (sizeof(UBLOX6M_MSG_HDR) - sizeof(p_hdr->sync_char1) - sizeof(p_hdr->sync_char2)) + p_hdr->length,
                                                    checksum);

                    /* Complete frame if checksum is valid */
                    if(p_tail->chksum == checksum){
                        total_frm_size = ublox6m_RxBufIdx;
                    }

                    ublox6m_RxState = UBLOX6M_RX_WAIT_SYNC_1;
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

/**
 * ublox6m_CalChecksum - Function to calculate UBX frame checksum.
 *
 * @param   [in]        *p_data         Buffer contains UBX frame.
 *                                      (Start from header.msg_class).
 *
 * @param   [in]        length          UBX frame data length.
 *                                      (Not including sync_char1, sync_char2 and sync_char1).

 * @param   [in]        init_chksum     Current accumulated checksum value.
 *                                      (Should be initialized to 0 by default).
 *
 * @return  [uint16_t]  Accumulated checksum value.
 *
 * @retval  [0~N]       Accumulated checksum value.
 */
static uint16_t ublox6m_CalChecksum(uint8_t *p_data, uint16_t length,
                                    uint16_t init_chksum)
{
    uint8_t CK_A;
    uint8_t CK_B;
    uint16_t idx;

    CK_A = (uint8_t)(init_chksum & 0xFF);
    CK_B = (uint8_t)(init_chksum >> 8);

    if(p_data != NULL){

        for(idx = 0; idx < length; idx++){
            CK_A += p_data[idx];
            CK_B += CK_A;
        }
    }

    return (((uint16_t)CK_B << 8) | CK_A);
}
