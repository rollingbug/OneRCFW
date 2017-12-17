/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *      A amateur remote control software library. Use at your own risk.
 *
 * @file    uart_sim.cpp
 * @brief   Hardware/software simulated UART functions.
 *
 *          based on timer 0 and pin change detection mechanism,
 *          only support baud 9600.
 *
 *          Pin/Channel mapping:
 *              TX = Arduino D5 / AVR PD5 / PCINT 21 (PC group 2)
 *              RX = Arduino D6 / AVR PD6 / PCINT 22 (PC group 2)
 *
 *          Hardware:
 *              PinChange (PCINT0) - For RX - Arduino D8 - PORTB0
 *              Timer0 (OC0B) - For TX
 *              Timer0 (OC0A) - For RX
 *
 *          Interrupt:
 *              ISR(PCINT0_vect)
 *              ISR(TIMER0_COMPA_vect)
 *              ISR(TIMER0_COMPB_vect)
 *
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>

#include "uart_sim.h"
#include "timers_drv.h"
#include "pin_change.h"
#include "uart_stream.h"
#include "uart_drv.h"
#include "debug.h"

#if UARTS_FUNCTION_EN


/*
 *******************************************************************************
 * Constant value definition
 *******************************************************************************
 */

#define UARTS_LIMIT_BAUD_RATE       9600

#define UARTS_TX_FIFO_SIZE          32      /* TX FIFO size, should not exceed 255 bytes */
#define UARTS_RX_FIFO_SIZE          32      /* RX FIFO size, should not exceed 255 bytes */

#define UARTS_TX_HIGH               1
#define UARTS_TX_LOW                0

#define UARTS_RX_HIGH               1
#define UARTS_RX_LOW                0

#define UARTS_TX_PORT_REG           PORTD
#define UARTS_TX_PORT_BIT           _BV(PORTD5)

#define UARTS_RX_PIN_REG            PIND
#define UARTS_RX_PIN_BIT            _BV(PIND6)

#define UARTS_SAMPLING_DELAY_TICKS  TIMER0_MICROS_TO_TICKS(10)


/*
 *******************************************************************************
 * Data type definition
 *******************************************************************************
 */

/* Data structure for storing RC input channel/pin information */
typedef struct uarts_pin{
    uint8_t ardu_pin;				/* Arduino digital pin ID */
    uint8_t mask;					/* PCMASK */
    PC_PIN_IDX pc_pin_idx;			/* PC pin index 0~23 */
    PC_GRP_IDX pc_grp_idx;			/* PC group index 0~2 */
}UARTS_PIN;

/* definition of AVR output compare mode */
typedef enum uarts_oc_mode{
    UARTS_OC_NORMAL,
    UARTS_OC_TOGGLE,
    UARTS_OC_CLEAR,
    UARTS_OC_SET,
}UARTS_OC_MODE;


/*
 *******************************************************************************
 * Global variables
 *******************************************************************************
 */

/* Simulated UART TX and RX pin configuration */
static UARTS_PIN UartS_TxPin = {5, PC_PIN_MASK_21, PC_PIN_IDX_21, PC_GRP_IDX_2};
static UARTS_PIN UartS_RxPin = {6, PC_PIN_MASK_22, PC_PIN_IDX_22, PC_GRP_IDX_2};

static uint8_t UartS_OnePulseTicks;

static bool UartS_IsTXIdle;                         /* TX idle status */
static uint8_t UartS_TxPulseCnt;
static uint8_t UartS_TxStartTicks;
static uint8_t UartS_TxDataByte;
static uint8_t UartS_TxFifo[UARTS_TX_FIFO_SIZE];    /* TX FIFO */
static volatile uint8_t UartS_TxFifoHdrIdx;         /* TX FIFO header index */
static volatile uint8_t UartS_TxFifoTailIdx;        /* TX FIFO tail index */

static bool UartS_IsRxPinChgInterruptEn;
static uint8_t UartS_RxPulseCnt;

static uint8_t UartS_RxPulseStartTicks;
static uint8_t UartS_RxDataByte;
static uint8_t UartS_RxErrCnt;
static uint8_t UartS_RxDropCnt;
static uint8_t UartS_RxFifo[UARTS_RX_FIFO_SIZE];    /* RX FIFO */
static volatile uint8_t UartS_RxFifoHdrIdx;         /* RX FIFO header index */
static volatile uint8_t UartS_RxFifoTailIdx;        /* RX FIFO tail index */


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

static void UartS_SetTxCompareForceHigh();
static void UartS_SetTxOutputCompare(uint8_t trig_ticks, UARTS_OC_MODE mode);
static uint8_t UartS_WBytes(uint8_t *p_data, uint8_t bytes, bool is_blocking);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * UartS_Init - Function to initialize simulated UART function.
 *
 * Notice: This function should only NOT be called before the system timers
 *         initialization.
 *
 * @param   [in]        baud_rate   Baud rate setting, Eg. 9600.
 *
 * @return  [int8_t]    Function executing result.
 * @retval  [0]         Success.
 * @retval  [-1]        Fail.
 *
 */
int8_t UartS_Init(uint32_t baud_rate)
{
    /* Check simulated UART capability */
    if(baud_rate != UARTS_LIMIT_BAUD_RATE)
        return -1;

    memset((void *)UartS_TxFifo, 0, sizeof(UartS_TxFifo));
    memset((void *)UartS_RxFifo, 0, sizeof(UartS_RxFifo));

    /* Initialize basic parameters */
    UartS_OnePulseTicks = (uint8_t)(TIMER0_MICROS_TO_TICKS(1000000) / baud_rate);

    /* Initialize TX parameters */
    UartS_IsTXIdle = true;
    UartS_TxPulseCnt = 0;
    UartS_TxStartTicks = 0;
    UartS_TxDataByte = 0;
    UartS_TxFifoHdrIdx = 0;
    UartS_TxFifoTailIdx = 0;

    /* Initialize RX parameters */
    UartS_RxPulseCnt = 0;
    UartS_RxPulseStartTicks = 0;
    UartS_RxDataByte = 0;
    UartS_RxErrCnt = 0;
    UartS_RxDropCnt = 0;
    UartS_IsRxPinChgInterruptEn = true;
    UartS_RxFifoHdrIdx = 0;
    UartS_RxFifoTailIdx = 0;

    /* Initialize simulated UART TX pin to output mode and force OC0B output HIGH by default */
    pinMode(UartS_TxPin.ardu_pin, OUTPUT);
    UartS_SetTxCompareForceHigh();

    /* Initialize simulated UART RX pin to input mode */
    pinMode(UartS_RxPin.ardu_pin, INPUT);

    /* Enable PC interrupt for specific pin */
    PC_Setup(UartS_RxPin.pc_grp_idx, UartS_RxPin.mask, true);

    Uart0_Println(PSTR("[UART_SIM] Baud: %u, TX pin: %hhu, RX pin: %hhu"),
                  baud_rate, UartS_TxPin.ardu_pin, UartS_RxPin.ardu_pin);

    return 0;
}

/**
 * UartS_ReadBytes - Function to read data from RX FIFO of simulated UART.
 *
 * @param   [in]        *p_data     Array for storing RX data.
 * @param   [in]        bytes       Size of input array.
 *
 * @return  [uint8_t]   Number of read bytes
 * @retval  [0]         No RX data.
 * @retval  [1~255]     RX Byte size.
 *
 */
uint8_t UartS_ReadBytes(uint8_t *p_data, uint8_t bytes)
{
    uint8_t rx_idx;

    rx_idx = 0;

    while(bytes != 0){

        /* Read buffered data from RX FIFO */
        if(UartS_RxFifoHdrIdx != UartS_RxFifoTailIdx){
            p_data[rx_idx] = UartS_RxFifo[UartS_RxFifoHdrIdx];
            rx_idx++;

            UartS_RxFifoHdrIdx = (UartS_RxFifoHdrIdx + 1) % UARTS_RX_FIFO_SIZE;
        }
        /* Stop this loop when FIFO is empty */
        else
            break;

        bytes--;
    }

    return rx_idx;
}

/**
 * UartS_ReadByte - Function to read single byte data from RX FIFO
 *                  of simulated UART.
 *
 * @param   [in]        *p_data     1 byte buffer for storing RX data.
 *
 * @return  [uint8_t]   Number of read bytes
 * @retval  [0]         No RX data.
 * @retval  [1]         RX Byte size.
 *
 */
uint8_t UartS_ReadByte(uint8_t *p_data)
{
    return UartS_ReadBytes(p_data, 1);
}

/**
 * UartS_ReadAvailable - Function to check total number of current
 *                       received data in RX FIFO simulated UART.
 *
 * @param   [none]
 *
 * @return  [uint8_t]   Number of received data in RX FIFO.
 * @retval  [0]         No RX data.
 * @retval  [1~255]     Received Byte size.
 *
 */
uint8_t UartS_ReadAvailable()
{

    uint8_t cnt;

    cnt = (uint8_t)(UARTS_RX_FIFO_SIZE + UartS_RxFifoHdrIdx - UartS_RxFifoTailIdx)
        % UARTS_RX_FIFO_SIZE;

    return cnt;
}

/**
 * UartS_WriteBytes - Function to write data to TX FIFO of simulated UART in
 *                    blocking mode.
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
uint8_t UartS_WriteBytes(uint8_t *p_data, uint8_t bytes)
{
    return UartS_WBytes(p_data, bytes, true);
}

/**
 * UartS_WriteBytesNB - Function to write data to TX FIFO of simulated UART
 *                      in NON-blocking mode.
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
uint8_t UartS_WriteBytesNB(uint8_t *p_data, uint8_t bytes)
{
    return UartS_WBytes(p_data, bytes, false);
}

/**
 * UartS_WriteByte - Function to write single byte data to TX FIFO of simulated UART
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
uint8_t UartS_WriteByte(uint8_t data)
{
    return UartS_WBytes(&data, 1, true);
}

/**
 * UartS_WriteByteNB - Function to write single byte data to TX FIFO of simulated UART
 *                     in NON-blocking mode.
 *
 *                     This function will return "immediately" when TX FIFO is full.
 *
 * @param   [in]        data    TX data.
 *
 * @return  [uint8_t]   Total number of bytes have been written to TX FIFO.
 * @retval  [0~255]     Bytes
 *
 */
uint8_t UartS_WriteByteNB(uint8_t data)
{
    return UartS_WBytes(&data, 1, false);
}

/**
 * UartS_RxPulseHandler - Simulated UART RX pulse signal handler.
 *
 * This function should only be called in corresponding pin change ISR.
 *
 * @param   [in]    pc_grp_idx      Group index of pin change interrupt.
 * @param   [in]    trig_time       Interrupt triggered Time-stamp.
 * @param   [in]    pin_status      Current pin value.
 * @param   [in]    pin_change      Pin value change status.
 *
 * @return  [none]
 *
 */
void UartS_RxPulseHandler(PC_GRP_IDX pc_grp_idx, uint32_t trig_time,
                          uint8_t pin_status, uint8_t pin_change)
{
    /* Looking for RX start falling edge */
    if(UartS_IsRxPinChgInterruptEn == true){
        if((UartS_RxPin.pc_grp_idx == pc_grp_idx)
        && ((pin_status & UartS_RxPin.mask) == UARTS_RX_LOW)){

            /* Reset data before we collect new data */
            UartS_RxDataByte = 0;
            UartS_RxPulseCnt = 1;   /* We've collected start bit */

            /* Shift to first data bit start position with some delay */
            UartS_RxPulseStartTicks = Timer0_GetTicks8()
                                    + UartS_OnePulseTicks
                                    + UARTS_SAMPLING_DELAY_TICKS;

            /* Enable timer interrupt (will be trigger every uart clock before STOP bit) */
            Timer0_SetTimerCompA(UartS_RxPulseStartTicks, true);

            /* Disable simulated RX pin change interrupt */
            UartS_IsRxPinChgInterruptEn = false;
            PC_QuickDisable(UartS_RxPin.pc_grp_idx, UartS_RxPin.mask);
        }
    }
}

/**
 * ISR(TIMER0_COMPA_vect) - Timer 0A output compare ISR.
 *
 * We use timer 0A output compare timer for capturing RX pulse value.
 * Requires 4 ~ 6 us
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(TIMER0_COMPA_vect)
{
    uint8_t data_bit;
    uint8_t tail_next;

    DEBUG_ISR_START(TIMER0_COMPA_vect_num);

    /* Fetch UART data bit */
    data_bit = (UARTS_RX_PIN_REG & UARTS_RX_PIN_BIT) ? UARTS_RX_HIGH : UARTS_RX_LOW;

    /* Stop bit */
    if(UartS_RxPulseCnt == 9){
        Timer0_SetTimerCompA(UartS_RxPulseStartTicks, false);

        UartS_IsRxPinChgInterruptEn = true;
        PC_QuickEnable(UartS_RxPin.pc_grp_idx, UartS_RxPin.mask);

        /* Make sure the stop bit is valid then process RX data */
        if(data_bit == UARTS_RX_HIGH){

            tail_next = (UartS_RxFifoTailIdx + 1) % UARTS_RX_FIFO_SIZE;

            /* Store incoming data if there is space in FIFO */
            if(tail_next != UartS_RxFifoHdrIdx){
                UartS_RxFifo[UartS_RxFifoTailIdx] = UartS_RxDataByte;
                UartS_RxFifoTailIdx = tail_next;
            }
            else
                UartS_RxDropCnt++;
        }
        else{
            UartS_RxErrCnt++;
        }
    }
    else{
        /* Shift timer trigger point to next pulse */
        UartS_RxPulseStartTicks += UartS_OnePulseTicks;
        Timer0_SetTimerCompA(UartS_RxPulseStartTicks, true);

        /* Collect data bits (We've skipped the start bit directly) */
        UartS_RxDataByte |= (data_bit << (UartS_RxPulseCnt - 1));
    }

    UartS_RxPulseCnt++;

    DEBUG_ISR_END(TIMER0_COMPA_vect_num);
}

/**
 * ISR(TIMER0_COMPB_vect) - Timer 0B output compare ISR.
 *
 * We use timer 0B output compare timer for generating TX pulse signal.
 * Requires about 2.875 ~ 6.25 us.
 *
 * @param   [none]
 *
 * @return  [none]
 *
 */
ISR(TIMER0_COMPB_vect)
{
    static const UARTS_OC_MODE data_trig_mode[] =
    {
        UARTS_OC_CLEAR,
        UARTS_OC_SET
    };
    uint8_t data_bit;

    DEBUG_ISR_START(TIMER0_COMPB_vect_num);

    UartS_TxStartTicks += UartS_OnePulseTicks;

    /* Transmit start pulse, signal low */
    if(UartS_TxPulseCnt == 0){

        /* There is no data, keep idle high signal */
        if(UartS_TxFifoHdrIdx == UartS_TxFifoTailIdx){
            Timer0_SetTimerCompB(UartS_TxStartTicks, false);
            UartS_IsTXIdle = true;
        }
        /* Generate low pulse */
        else{
            UartS_SetTxOutputCompare(UartS_TxStartTicks, UARTS_OC_CLEAR);
            UartS_TxPulseCnt++;

            /* Assign new TX data to TX register */
            UartS_TxDataByte = UartS_TxFifo[UartS_TxFifoHdrIdx];

            /* Shift FIFO header index */
            UartS_TxFifoHdrIdx = (UartS_TxFifoHdrIdx + 1) % UARTS_TX_FIFO_SIZE;
        }
    }
    /* Transmit stop pulse, signal high */
    else if(UartS_TxPulseCnt == 9){
        UartS_SetTxOutputCompare(UartS_TxStartTicks, UARTS_OC_SET);
        UartS_TxPulseCnt = 0;

    }
    /* Data bit pulse */
    else{
        data_bit = (UartS_TxDataByte >> (UartS_TxPulseCnt - 1)) & 0x01;
        UartS_SetTxOutputCompare(UartS_TxStartTicks, data_trig_mode[data_bit]);

        UartS_TxPulseCnt++;
    }

    DEBUG_ISR_END(TIMER0_COMPB_vect_num);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

static void UartS_SetTxCompareForceHigh()
{
    UartS_SetTxOutputCompare(0, UARTS_OC_SET);
    TCCR0B |= _BV(FOC0B);
}

/**
 * UartS_SetTxOutputCompare - Function to switch AVR OC0B operating mode.
 *
 * @param   [in]    trig_ticks          Trigger ticks.
 *
 * @param   [in]    mode                Specific output compare mode.
 *                                      RCOUT_OC_NORMAL: normal mode.
 *                                      RCOUT_OC_TOGGLE: toggle mode.
 *                                      RCOUT_OC_CLEAR: Output LOW when triggered.
 *                                      RCOUT_OC_SET: Output HIGH when triggered.
 *
 * @return  [none]
 *
 */
static void UartS_SetTxOutputCompare(uint8_t trig_ticks, UARTS_OC_MODE mode)
{
    /* Predefined configuration for switch "output compare mode". */
    static const uint8_t OC0B_Settings[] =
    {
        [UARTS_OC_NORMAL]   = (uint8_t)~(_BV(COM0B1) | _BV(COM0B0)),
        [UARTS_OC_TOGGLE]   = (uint8_t)~(_BV(COM0B1)),
        [UARTS_OC_CLEAR]    = (uint8_t)~(_BV(COM0B0)),
        [UARTS_OC_SET]      = (uint8_t)(_BV(COM0B1) | _BV(COM0B0)),
    };

    /* Update trigger target */
    OCR0B = trig_ticks;

    /* Setup output compare mode */
    TCCR0A |= (_BV(COM0B1) | _BV(COM0B0));
    TCCR0A &= OC0B_Settings[mode];
}

/**
 * UartS_WBytes - Low level Simulated UART TX FIFO controlling function.
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
static uint8_t UartS_WBytes(uint8_t *p_data, uint8_t bytes, bool is_blocking)
{
    uint8_t old_SREG;
    uint8_t cnt;
    uint8_t tail_next;

    for(cnt = 0; cnt < bytes; cnt++){

        tail_next = (UartS_TxFifoTailIdx + 1) % UARTS_TX_FIFO_SIZE;

        /* Wait if FIFO is full */
        while(tail_next == UartS_TxFifoHdrIdx){
            if(is_blocking == false){
                return cnt;
            }
        }

        /* Put data to FIFO */
        UartS_TxFifo[UartS_TxFifoTailIdx] = p_data[cnt];
        UartS_TxFifoTailIdx = tail_next;

        /* Enable TX data timer if needed */
        if(UartS_IsTXIdle == true){

            /* Store current AVR Status register then disable global interrupt */
            old_SREG = SREG;
            cli();

            if(UartS_IsTXIdle == true){
                UartS_IsTXIdle = false;
                UartS_TxStartTicks = Timer0_GetTicks8() + UartS_OnePulseTicks;
                Timer0_SetTimerCompB(UartS_TxStartTicks, true);
            }

            /* Enable global interrupt */
            SREG = old_SREG;
        }
    }

    return cnt;
}

#endif // UARTS_FUNCTION_EN
