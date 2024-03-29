/*
MIT License

Copyright (c) 2023 Oskar von Heideken

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#ifndef RENESAS_UTILS_H_
#define RENESAS_UTILS_H_
#include "FreeRTOS.h"
#include "queue.h"
#include "r_gpt.h"
#include "r_sci_spi.h"
#include "r_uart_api.h"
#include <stdarg.h>

extern QueueHandle_t uartReceiveQueue;

/**
 * @brief Initializes UART.
 * @return True if successful, otherwise false
 */
bool initUart(void);

/**
 * @brief Initializes MCU functions outside of RTOS context
 * @return None
 */
void mcuInit(void);
bool initSpi(void);
bool initTimer(void);

void UARTvprintf(const char *pcString, va_list vaArgP);
void uartRxIntHandler(uart_callback_args_t *p_args);
void spiSendReceive(spi_ctrl_t *const p_api_ctrl, void const *p_src,
                    void *p_dest, uint32_t const length,
                    spi_bit_width_t const bit_width);
void spiReceive(spi_ctrl_t *const p_api_ctrl, void *p_dest,
                uint32_t const length, spi_bit_width_t const bit_width);
void spiSend(spi_ctrl_t *const p_api_ctrl, void const *p_src,
             uint32_t const length, spi_bit_width_t const bit_width);
void sci_spi_callback(spi_callback_args_t *p_args);

void Timer1IntHandler(timer_callback_args_t *p_args);
void Timer0IntHandler(timer_callback_args_t *p_args);

#endif // RENESAS_UTILS_H_