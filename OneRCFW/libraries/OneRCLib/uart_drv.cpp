/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    uart_drv.cpp
 * @brief   AVR UART driver functions.
 *
 *          Hardware:
 *              USART0
 *
 *          Interrupt:
 *              ISR(USART_UDRE_vect)
 *              ISR(USART_RX_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "uart_drv.h"
#include "uart_stream.h"
#include "debug.h"


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define UART0_TX_FIFO_SIZE  128     /* TX FIFO size, should not exceed 255 bytes */
#define UART0_RX_FIFO_SIZE  128     /* RX FIFO size, should not exceed 255 bytes */


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

static uint8_t Uart0_TxFifo[UART0_TX_FIFO_SIZE];    /* TX FIFO */
static volatile uint8_t Uart0_TxFifoHdrIdx;         /* TX FIFO header index */
static volatile uint8_t Uart0_TxFifoTailIdx;        /* TX FIFO tail index */

static uint8_t Uart0_RxFifo[UART0_RX_FIFO_SIZE];    /* RX FIFO */
static volatile uint8_t Uart0_RxFifoHdrIdx;         /* RX FIFO header index */
static volatile uint8_t Uart0_RxFifoTailIdx;        /* RX FIFO tail index */


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

static uint8_t Uart0_WBytes(uint8_t *p_data, uint8_t bytes, bool is_blocking);
static void Uart0_SendDataISR();
static void Uart0_RecvDataISR();


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * Uart0_Init - Function to initialize AVR UART0
 *
 * @param   [in]        baud_rate   Baud rate setting, Eg. 9600, 115200.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t Uart0_Init(uint32_t baud_rate)
{
    uint16_t baud_setting;

    if(baud_rate == 0)
        return -1;

    /* Disable all interrupts */
    cli();

    memset((void *)Uart0_TxFifo, 0, sizeof(Uart0_TxFifo));
    memset((void *)Uart0_RxFifo, 0, sizeof(Uart0_RxFifo));

    Uart0_TxFifoHdrIdx = 0;
    Uart0_TxFifoTailIdx = 0;
    Uart0_RxFifoHdrIdx = 0;
    Uart0_RxFifoTailIdx = 0;

    /* Calculate baud setting based on double UART speed mode */
    baud_setting = (F_CPU / baud_rate / 8) - 1;

    /* Apply baud setting, 12 bits */
    UBRR0 = (baud_setting & 0xFFF);

    /* Enable double UART speed mode */
    UCSR0A |= _BV(U2X0);

    /* 8 Bits Character, 1 Stop bit, Asynchronous USART, no Parity check */
    UCSR0C = (_BV(UCSZ01) | _BV(UCSZ00));

    /* Enable Transmitter and receiver function, enable RX Complete Interrupt */
    UCSR0B = (_BV(TXEN0) | _BV(RXEN0) | _BV(RXCIE0));

    /* Enable all interrupts */
    sei();

    Uart0_Println(PSTR("[UART0] Baud: %u"), baud_rate);

    return 0;
}

/**
 * Uart0_ReadBytes - Function to read UART0 data from RX FIFO.
 *
 * @param   [in]        *p_data     Array for storing RX data.
 * @param   [in]        bytes       Size of input array.
 *
 * @return  [uint8_t]   Number of read bytes
 * @retval  [0]         No RX data.
 * @retval  [1~255]     RX Byte size.
 *
 */
uint8_t Uart0_ReadBytes(uint8_t *p_data, uint8_t bytes)
{
    uint8_t rx_idx;

    rx_idx = 0;

    while(bytes != 0){

        /* Read buffered data from RX FIFO */
        if(Uart0_RxFifoHdrIdx != Uart0_RxFifoTailIdx){
            p_data[rx_idx] = Uart0_RxFifo[Uart0_RxFifoHdrIdx];
            rx_idx++;

            Uart0_RxFifoHdrIdx = (Uart0_RxFifoHdrIdx + 1) % UART0_RX_FIFO_SIZE;
        }
        /* Stop this loop when FIFO is empty */
        else
            break;

        bytes--;
    }

    return rx_idx;
}

/**
 * Uart0_ReadByte - Function to read UART0 single byte data from RX FIFO.
 *
 * @param   [in]        *p_data     1 byte buffer for storing RX data.
 *
 * @return  [uint8_t]   Number of read bytes
 * @retval  [0]         No RX data.
 * @retval  [1]         RX Byte size.
 *
 */
uint8_t Uart0_ReadByte(uint8_t *p_data)
{
    return Uart0_ReadBytes(p_data, 1);
}

/**
 * Uart0_ReadAvailable - Function to check total number of current
 *                       received data in RX FIFO.
 *
 * @param   [none]
 *
 * @return  [uint8_t]   Number of received data in RX FIFO.
 * @retval  [0]         No RX data.
 * @retval  [1~255]     Received Byte size.
 *
 */
uint8_t Uart0_ReadAvailable()
{
    uint8_t cnt;

    cnt = (uint8_t)(UART0_RX_FIFO_SIZE + Uart0_RxFifoHdrIdx - Uart0_RxFifoTailIdx)
        % UART0_RX_FIFO_SIZE;

    return cnt;
}

/**
 * Uart0_WriteBytes - Function to write UART0 data to TX FIFO in blocking mode.
 *
 *                    Means this function will be blocked when TX FIFO is full,
 *                    and it won't return from this function before it sends out
 *                    all the data.
 *
 * @param   [in]        *p_data     Array containing TX data.
 * @param   [in]        bytes       Actual size of TX data.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
uint8_t Uart0_WriteBytes(uint8_t *p_data, uint8_t bytes)
{
    return Uart0_WBytes(p_data, bytes, true);
}

/**
 * Uart0_WriteBytesNB - Function to write UART0 data to TX FIFO in NON-blocking mode.
 *
 *                      This function will return "immediately" when TX FIFO is full.
 *
 * @param   [in]        *p_data     Array containing TX data.
 * @param   [in]        bytes       Actual size of TX data.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
uint8_t Uart0_WriteBytesNB(uint8_t *p_data, uint8_t bytes)
{
    return Uart0_WBytes(p_data, bytes, false);
}

/**
 * Uart0_WriteByte - Function to write UART0 single byte data to TX FIFO
 *                   in blocking mode.
 *
 *                   Means this function will be blocked when TX FIFO is full,
 *                   and it won't return from this function before it sends out
 *                   all the data.
 *
 * @param   [in]        data    TX data.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
uint8_t Uart0_WriteByte(uint8_t data)
{
    return Uart0_WBytes(&data, 1, true);
}

/**
 * Uart0_WriteByteNB - Function to write UART0 data to TX FIFO in NON-blocking mode.
 *
 *                     This function will return "immediately" when TX FIFO is full.
 *
 * @param   [in]        data    TX data.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
uint8_t Uart0_WriteByteNB(uint8_t data)
{
    return Uart0_WBytes(&data, 1, false);
}

/**
 * ISR(USART_UDRE_vect) - USART TX data register empty ISR.
 *
 * Require about 2 ~ 2.4 us.
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [int]   Function executing result.
 * @retval  [0]     Success.
 * @retval  [-1]    Fail.
 *
 */
ISR(USART_UDRE_vect)
{
    DEBUG_ISR_START(USART_UDRE_vect_num);

    Uart0_SendDataISR();

    DEBUG_ISR_END(USART_UDRE_vect_num);
}

/**
 * ISR(USART_RX_vect) - USART RX complete ISR.
 *
 * Require about 1.7 ~ 1.9 us.
 *
 * @param   [in]    input       Example input.
 * @param   [out]   *p_output   Example output.
 *
 * @return  [int]   Function executing result.
 * @retval  [0]     Success.
 * @retval  [-1]    Fail.
 *
 */
ISR(USART_RX_vect)
{
    DEBUG_ISR_START(USART_RX_vect_num);

    Uart0_RecvDataISR();

    DEBUG_ISR_END(USART_RX_vect_num);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * Uart0_WBytes - Low level UART0 TX FIFO controlling function.
 *
 * @param   [in]        *p_data         Array containing TX data.
 * @param   [in]        bytes           Actual size of TX data.
 * @param   [in]        is_blocking     Set true for entering blocking mode,
 *                                      means if the TX FIFO is full, then
 *                                      wait forever until we put all the data
 *                                      to buffer, otherwise give up immediately
 *                                      when buffer is full.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
static uint8_t Uart0_WBytes(uint8_t *p_data, uint8_t bytes, bool is_blocking)
{
    uint8_t cnt;
    uint8_t tail_next;

    for(cnt = 0; cnt < bytes; cnt++){

        tail_next = (Uart0_TxFifoTailIdx + 1) % UART0_TX_FIFO_SIZE;

        /* Wait if FIFO is full */
        while(tail_next == Uart0_TxFifoHdrIdx){
            if(is_blocking == false){
                return cnt;
            }
        }

        /* Put data to FIFO */
        Uart0_TxFifo[Uart0_TxFifoTailIdx] = p_data[cnt];
        Uart0_TxFifoTailIdx = tail_next;

        /* Enable data register empty interrupt */
        UCSR0B |= _BV(UDRIE0);
    }

    return cnt;
}

/**
 * Uart0_SendDataISR - Function to send data though UART interface.
 *                     This function will be called in ISR(USART_UDRE_vect).
 *                     The maximum function execution time is about 2 us.
 *
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static void Uart0_SendDataISR()
{
    /* Assign new TX data to TX register */
    UDR0 = Uart0_TxFifo[Uart0_TxFifoHdrIdx];

    /* Shift FIFO header index */
    Uart0_TxFifoHdrIdx = (Uart0_TxFifoHdrIdx + 1) % UART0_TX_FIFO_SIZE;

    /* Disable data register empty interrupt if FIFO is empty */
    if(Uart0_TxFifoHdrIdx == Uart0_TxFifoTailIdx)
        UCSR0B &= ~(_BV(UDRIE0));
}

/**
 * Uart0_RecvDataISR - Function to receive data though UART interface.
 *                     This function will be called in ISR(USART_RX_vect).
 *                     The maximum function execution time is about 1.6 us.
 *
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
static void Uart0_RecvDataISR()
{
    uint8_t tail_next;
    uint8_t rx_data;

    /* Take out new incoming data from register */
    rx_data = UDR0;

    tail_next = (Uart0_RxFifoTailIdx + 1) % UART0_RX_FIFO_SIZE;

    /* Store incoming data if there is space in FIFO */
    if(tail_next != Uart0_RxFifoHdrIdx){
        Uart0_RxFifo[Uart0_RxFifoTailIdx] = rx_data;
        Uart0_RxFifoTailIdx = tail_next;
    }
}
