/**
 *******************************************************************************
 *      ______  _   __  ______  ____     ______        ___    ______   ____
 *     / __  / / \ / / / ____/ / __ \   /  ___/       /  /   /_   _/  / __ \
 *    / /_/ / /   \ / / ____/ /  -- /  /  /__   __   /  /__  _/  /_  / __ <
 *   /_____/ /_/ \_/ /_____/ /__/ \_\ /_____/  /_/  /_____/ /_____/ /_____/
 *
 *     An amateur remote control software library. Use at your own risk.
 *
 * @file    uart_stream.cpp
 * @brief   AVR UART stream functions.
 * @author  Y.S.Kuo in Hsinchu
 *******************************************************************************
 */

#include <Arduino.h>
#include <math.h>
#include <stdarg.h>

#include "uart_drv.h"
#include "uart_sim.h"
#include "uart_stream.h"


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

typedef void (UART_PRT_CHR_FUNC)(char chr);
typedef void (UART_PRT_STR_FUNC)(const char *p_str);


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
static void UartStrm_PrintUnsigned(UART_PRT_STR_FUNC *p_prt_str_func,
                                   uint32_t value, uint8_t base);
static void UartStrm_PrintSigned(UART_PRT_STR_FUNC *p_prt_str_func, int32_t value);
static void UartStrm_PrintFloat(UART_PRT_CHR_FUNC *p_prt_chr_func,
                                UART_PRT_STR_FUNC *p_prt_str_func,
                                float value, uint8_t digits);
static void UartStrm_PrintFormat(UART_PRT_CHR_FUNC *p_prt_chr_func,
                                 UART_PRT_STR_FUNC *p_prt_str_func,
                                 const char *p_fmt, va_list args);


/*
 *******************************************************************************
 * Public functions
 *******************************************************************************
 */

/**
 * Uart0_PrintChr - Function to print single character.
 *
 * @param   [in]        chr         Displaying character.
 *
 * @return  [none]
 *
 */
void Uart0_PrintChr(char chr)
{
    Uart0_WriteByte((uint8_t)chr);
}

/**
 * Uart0_PrintStr - Function to print string.
 *
 * @param   [in]        *p_str      Displaying string.
 *
 * @return  [none]
 *
 */
void Uart0_PrintStr(const char *p_str)
{
    char ch;

    while((ch = *(p_str++))){
        Uart0_WriteByte(ch);
    }
}

/**
 * Uart0_PrintUnsigned - Function to print unsigned integer.
 *
 * @param   [in]    value       Display integer.
 * @param   [in]    base        Numerical base used to represent the value
 *                              as a string.
 *                               2: Binary.
 *                               8: Octal.
 *                              10: Decimal.
 *                              16: Hexadecimal
 *
 * @return  [none]
 *
 */
void Uart0_PrintUnsigned(uint32_t value, uint8_t base)
{
    UartStrm_PrintUnsigned(Uart0_PrintStr, value, base);
}

/**
 * Uart0_PrintSigned - Function to print signed integer.
 *
 * @param   [in]    value       Display integer.
 *
 * @return  [none]
 *
 */
void Uart0_PrintSigned(int32_t value)
{
    UartStrm_PrintSigned(Uart0_PrintStr, value);
}

/**
 * Uart0_PrintFloat - Function to print floating number.
 *
 * @param   [in]    value       Display floating number.
 * @param   [in]    digits      Maximum fractional digits to display.
 *
 * @return  [none]
 *
 */
void Uart0_PrintFloat(float value, uint8_t digits)
{
    UartStrm_PrintFloat(Uart0_PrintChr, Uart0_PrintStr, value, digits);
}

/**
 * Uart0_Printf - Public function to print formatted data
 *
 * @param   [in]    *p_fmt      Display format string.
 *                              Please notice this function only accept string
 *                              which is allocated in PROGRAM memory (FLASH).
 *
 *                              For example: Uart0_Printf(PSTR("test"), ...).
 *
 *                              Current support format:
 *                                  %hu, %hd, %hx, %hX,
 *                                  %hhu, %hhd, %hhx, %hhX,
 *                                  %u, %d, %h, %x, %X, %o, %f, %%
 *
 * @param   [in]    ...         Additional arguments.
 *
 * @return  [none]
 *
 */
void Uart0_Printf(const char *p_fmt, ...)
{
	va_list args;

	va_start(args, p_fmt);
    UartStrm_PrintFormat(Uart0_PrintChr, Uart0_PrintStr, p_fmt, args);
	va_end(args);
}

/**
 * Uart0_Println - Public function to print formatted data followed by "\r\n".
 *
 * @param   [in]    *p_fmt      Display format string.
 *                              Please notice this function only accept string
 *                              which is allocated in PROGRAM memory (FLASH).
 *
 *                              For example: Uart0_Printf(PSTR("test"), ...).
 *
 *                              Current support format:
 *                                  %hu, %hd, %hx, %hX,
 *                                  %hhu, %hhd, %hhx, %hhX,
 *                                  %u, %d, %h, %x, %X, %o, %f, %%
 *
 * @param   [in]    ...         Additional arguments.
 *
 * @return  [none]
 *
 */
void Uart0_Println(const char *p_fmt, ...)
{
	va_list args;

	va_start(args, p_fmt);
    UartStrm_PrintFormat(Uart0_PrintChr, Uart0_PrintStr, p_fmt, args);
    Uart0_PrintStr("\n\r");
	va_end(args);
}

/**
 * UartS_PrintChr - Function to print single character.
 *
 * @param   [in]        chr         Displaying character.
 *
 * @return  [none]
 *
 */
void UartS_PrintChr(char chr)
{
    UartS_WriteByte((uint8_t)chr);
}

/**
 * UartS_PrintStr - Function to print string.
 *
 * @param   [in]        *p_str      Displaying string.
 *
 * @return  [none]
 *
 */
void UartS_PrintStr(const char *p_str)
{
    char ch;

    while((ch = *(p_str++))){
        UartS_WriteByte(ch);
    }
}

/**
 * UartS_PrintUnsigned - Function to print unsigned integer.
 *
 * @param   [in]    value       Display integer.
 * @param   [in]    base        Numerical base used to represent the value
 *                              as a string.
 *                               2: Binary.
 *                               8: Octal.
 *                              10: Decimal.
 *                              16: Hexadecimal
 *
 * @return  [none]
 *
 */
void UartS_PrintUnsigned(uint32_t value, uint8_t base)
{
    UartStrm_PrintUnsigned(UartS_PrintStr, value, base);
}

/**
 * UartS_PrintSigned - Function to print signed integer.
 *
 * @param   [in]    value       Display integer.
 *
 * @return  [none]
 *
 */
void UartS_PrintSigned(int32_t value)
{
    UartStrm_PrintSigned(UartS_PrintStr, value);
}

/**
 * UartS_PrintFloat - Function to print floating number.
 *
 * @param   [in]    value       Display floating number.
 * @param   [in]    digits      Maximum fractional digits to display.
 *
 * @return  [none]
 *
 */
void UartS_PrintFloat(float value, uint8_t digits)
{
    UartStrm_PrintFloat(UartS_PrintChr, UartS_PrintStr, value, digits);
}

/**
 * UartS_Printf - Public function to print formatted data
 *
 * @param   [in]    *p_fmt      Display format string.
 *                              Please notice this function only accept string
 *                              which is allocated in PROGRAM memory (FLASH).
 *
 *                              For example: Uart0_Printf(PSTR("test"), ...).
 *
 *                              Current support format:
 *                                  %hu, %hd, %hx, %hX,
 *                                  %hhu, %hhd, %hhx, %hhX,
 *                                  %u, %d, %h, %x, %X, %o, %f, %%
 *
 * @param   [in]    ...         Additional arguments.
 *
 * @return  [none]
 *
 */
void UartS_Printf(const char *p_fmt, ...)
{
	va_list args;

	va_start(args, p_fmt);
    UartStrm_PrintFormat(UartS_PrintChr, UartS_PrintStr, p_fmt, args);
	va_end(args);
}

/**
 * Uart0_Println - Public function to print formatted data followed by "\r\n".
 *
 * @param   [in]    *p_fmt      Display format string.
 *                              Please notice this function only accept string
 *                              which is allocated in PROGRAM memory (FLASH).
 *
 *                              For example: Uart0_Printf(PSTR("test"), ...).
 *
 *                              Current support format:
 *                                  %hu, %hd, %hx, %hX,
 *                                  %hhu, %hhd, %hhx, %hhX,
 *                                  %u, %d, %h, %x, %X, %o, %f, %%
 *
 * @param   [in]    ...         Additional arguments.
 *
 * @return  [none]
 *
 */
void UartS_Println(const char *p_fmt, ...)
{
	va_list args;

	va_start(args, p_fmt);
    UartStrm_PrintFormat(UartS_PrintChr, UartS_PrintStr, p_fmt, args);
    UartS_PrintStr("\n\r");
	va_end(args);
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * UartStrm_PrintUnsigned - Function to print unsigned integer.
 *
 * @param   [in]    value       Display integer.
 * @param   [in]    base        Numerical base used to represent the value
 *                              as a string.
 *                               2: Binary.
 *                               8: Octal.
 *                              10: Decimal.
 *                              16: Hexadecimal
 *
 * @return  [none]
 *
 */
static void UartStrm_PrintUnsigned(UART_PRT_STR_FUNC *p_prt_str_func,
                                   uint32_t value, uint8_t base)
{
    int8_t buffer[sizeof(uint32_t) * 8 + 1];
    int8_t *p_str;
    uint32_t tmp;

    /* Only larger than 1 is valid */
    if(base < 2)
        base = 2;

    /* Start from tail */
    p_str = &buffer[sizeof(buffer) - 1];

    *p_str = '\0';

    do{
        /* tmp = value % base, value /= base */
        tmp = value;
        value /= base;
        tmp -= (value * base);

        if(tmp < 10)
            tmp += '0';         /* '0' ~ '9' */
        else
            tmp += ('A' - 10);  /* HEX, 'A' ~ 'F' */

        *(--p_str) = tmp;

    }while(value);

    p_prt_str_func((const char *)p_str);
}

/**
 * UartStrm_PrintSigned - Function to print signed integer.
 *
 * @param   [in]    value       Display integer.
 *
 * @return  [none]
 *
 */
static void UartStrm_PrintSigned(UART_PRT_STR_FUNC *p_prt_str_func, int32_t value)
{
    if(value < 0){
        Uart0_PrintChr('-');
        value = -value;
    }

    UartStrm_PrintUnsigned(p_prt_str_func, (uint32_t)value, 10);
}

/**
 * UartStrm_PrintFloat - Function to print floating number.
 *
 * @param   [in]    value       Display floating number.
 * @param   [in]    digits      Maximum fractional digits to display.
 *
 * @return  [none]
 *
 */
static void UartStrm_PrintFloat(UART_PRT_CHR_FUNC *p_prt_chr_func,
                                UART_PRT_STR_FUNC *p_prt_str_func,
                                float value, uint8_t digits)
{
    uint32_t ipart;
    uint8_t tmp;
    float round;

    if(isnan(value)){
        p_prt_str_func("nan");
        return;
    }
    else if(isinf(value)){
        p_prt_str_func("inf");
        return;
    }
    else if(value > 4294967040.0 || value < -4294967040.0){
        p_prt_str_func("ovf");
        return;
    }

    if(value < 0){
        p_prt_chr_func('-');
        value = -value;
    }

    /* Round if need */
    tmp = digits;
    round = 0.5;
    while(tmp--){
        round *= 0.1;
    }

    value += round;

    /* Display integer part first */
    ipart = (uint32_t)value;
    value -= (float)ipart;
    UartStrm_PrintSigned(p_prt_str_func, ipart);

    if(digits)
        p_prt_chr_func('.');

    /* Display fraction part */
    while(digits--){
        value *= 10.0;
        ipart = (uint32_t)value;
        UartStrm_PrintSigned(p_prt_str_func, ipart);
        value -= ipart;
    }
}


/*
 *******************************************************************************
 * Private functions
 *******************************************************************************
 */

/**
 * UartStrm_PrintFormat - Private function which is created for printing formatted
 *                        data to UART.
 *
 * @param   [in]    *p_fmt      Display format string.
 *                              Please notice this function only accept string
 *                              which is allocated in PROGRAM memory (FLASH).
 *
 *                              For example: Uart0_Printf(PSTR("test"), ...).
 *
 *                              Current support format:
 *                                  %hu, %hd, %hx, %hX,
 *                                  %hhu, %hhd, %hhx, %hhX,
 *                                  %u, %d, %h, %x, %X, %o, %f, %%
 *
 *
 * @param   [in]    args        Additional arguments.
 *
 * @return  [none]
 *
 */
static void UartStrm_PrintFormat(UART_PRT_CHR_FUNC *p_prt_chr_func,
                                 UART_PRT_STR_FUNC *p_prt_str_func,
                                 const char *p_fmt, va_list args)
{
    int8_t ch;

    while((ch = pgm_read_byte(p_fmt++)) != 0){

        if(ch != '%'){
            p_prt_chr_func(ch);
        }
        else{
            ch = pgm_read_byte(p_fmt++);

            switch(ch){
                /* Null */
                case 0:
                    break;
                /* 16 Bits, %hu, %hd, %hx, %hX */
                case 'h':
                    ch = pgm_read_byte(p_fmt++);
                    if(ch == 0)
                        break;
                    else if(ch == 'u')
                        UartStrm_PrintUnsigned(p_prt_str_func, (uint16_t)va_arg(args, uint16_t), 10);
                    else if(ch == 'd')
                        UartStrm_PrintSigned(p_prt_str_func, (int16_t)va_arg(args, int16_t));
                    else if(ch == 'X' || ch == 'x')
                        UartStrm_PrintUnsigned(p_prt_str_func, (uint16_t)va_arg(args, uint16_t), 16);
                    /* 8 Bits, %hhu, %hhd, %hhx, %hhX */
                    else if(ch == 'h'){
                        ch = pgm_read_byte(p_fmt++);
                        if(ch == 0)
                            break;
                        else if(ch == 'u')
                            UartStrm_PrintUnsigned(p_prt_str_func, (uint8_t)va_arg(args, uint16_t), 10);
                        else if(ch == 'd')
                            UartStrm_PrintSigned(p_prt_str_func, (int8_t)va_arg(args, int16_t));
                        else if(ch == 'X' || ch == 'x')
                            UartStrm_PrintUnsigned(p_prt_str_func, (uint8_t)va_arg(args, uint16_t), 16);
                        else
                            p_prt_chr_func('?');
                    }
                    else
                        p_prt_chr_func('?');

                    break;
                /* Float, %f */
                case 'f':
                    UartStrm_PrintFloat(p_prt_chr_func, p_prt_str_func, va_arg(args, double), 2);
                    break;
                /* 32 Bits, Unsigned Dec, %u */
                case 'u':
                    UartStrm_PrintUnsigned(p_prt_str_func, va_arg(args, uint32_t), 10);
                    break;
                /* 32 Bits, Signed Dec, %d */
                case 'd':
                    UartStrm_PrintSigned(p_prt_str_func, va_arg(args, int32_t));
                    break;
                /* Character, %c */
                case 'c':
                    p_prt_chr_func((char)va_arg(args, int16_t));
                    break;
                /* String, %s */
                case 's':
                    p_prt_str_func(va_arg(args, char *));
                    break;
                /* 32 Bits, Hex,  %X, %x */
                case 'X':
                case 'x':
                    UartStrm_PrintUnsigned(p_prt_str_func, va_arg(args, uint32_t), 16);
                    break;
                /* 32 Bits, Octal, %o */
                case 'o':
                    UartStrm_PrintUnsigned(p_prt_str_func, va_arg(args, uint32_t), 8);
                    break;
                /* % symbol, %% */
                case '%':
                    p_prt_chr_func('%');
                    break;
                /* Unknown */
                default:
                    p_prt_chr_func('?');
            }
        }
    }
}
