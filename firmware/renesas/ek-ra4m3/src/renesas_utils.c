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

/*
 * This file implements some of the functions unique to the
 * Renesas FSP which are included in TIVAWARE, such as UART printf
 */

// Standard library
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// Renesas files
#include "hal_data.h"
#include "r_sci_uart.h"
#include "renesas_utils.h"

// Comscicalc files
#include "firmware_common.h"

void Timer0IntHandler(timer_callback_args_t *p_args) {
    // Check that the event was as expected, then call
    // the common ISR function.
    Timer1HzIntHandler();
}
void Timer1IntHandler(timer_callback_args_t *p_args) {
    // Check that the event was as expected, then call
    // the common ISR function.
    Timer60HzIntHandler();
}

void mcuInit(void) {
    // Renesas MCU is initialized outside of the main,
    // hence this is left empty.
    return;
}

bool initUart(void) {
    fsp_err_t err = R_SCI_UART_Open(&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err) {
        return false;
    }
    return true;
}

bool initSpi(void) {
    // HW SPI init handled by EVE FT81x driver
    return true;
}

bool initTimer(void) {
    if (FSP_SUCCESS != R_GPT_Open(&g_timer0_ctrl, &g_timer0_cfg)) {
        return false;
    }

    if (FSP_SUCCESS != R_GPT_Open(&g_timer1_ctrl, &g_timer1_cfg)) {
        return false;
    }
    return true;
}

void startTimer(void) {
    if (FSP_SUCCESS != R_GPT_Start(&g_timer0_ctrl)) {
        while (1)
            ;
    }
    if (FSP_SUCCESS != R_GPT_Start(&g_timer1_ctrl)) {
        while (1)
            ;
    }
}

static volatile spi_event_t
    g_master_event_flag; // Master Transfer Event completion flag
bool spiTxComplete = false;
// Nasty hack for the SPI as well.
void sci_spi_callback(spi_callback_args_t *p_args) {
    if (SPI_EVENT_TRANSFER_COMPLETE == p_args->event) {
        g_master_event_flag = SPI_EVENT_TRANSFER_COMPLETE;
        spiTxComplete = true;
    } else {
        /* Updating the flag here to capture and handle all other error events
         */
        g_master_event_flag = SPI_EVENT_TRANSFER_ABORTED;
        spiTxComplete = false;
    }
}
static volatile int32_t g_wait_count = INT32_MAX;
static void sci_spi_event_check(void) {
    while (!spiTxComplete) {
        g_wait_count--;
        if (0 >= g_wait_count) {
            return;
        }
    }
}

void spiSend(spi_ctrl_t *const p_api_ctrl, void const *p_src,
             uint32_t const length, spi_bit_width_t const bit_width) {
    // Enter critical section as to not overwrite a variable set
    // in an interrupt.
    sci_spi_instance_ctrl_t *p_ctrl = (sci_spi_instance_ctrl_t *)p_api_ctrl;
    R_BSP_IrqDisable(p_ctrl->p_cfg->tei_irq);
    spiTxComplete = false;
    R_BSP_IrqEnable(p_ctrl->p_cfg->tei_irq);
    if (FSP_SUCCESS != R_SCI_SPI_Write(p_api_ctrl, p_src, length, bit_width)) {
        while (1)
            ;
    }

    // Wait until TX is complete.
    sci_spi_event_check();
}

void spiReceive(spi_ctrl_t *const p_api_ctrl, void *p_dest,
                uint32_t const length, spi_bit_width_t const bit_width) {
    // Enter critical section as to not overwrite a variable set
    // in an interrupt.
    sci_spi_instance_ctrl_t *p_ctrl = (sci_spi_instance_ctrl_t *)p_api_ctrl;
    R_BSP_IrqDisable(p_ctrl->p_cfg->tei_irq);
    spiTxComplete = false;
    R_BSP_IrqEnable(p_ctrl->p_cfg->tei_irq);
    if (FSP_SUCCESS != R_SCI_SPI_Read(p_api_ctrl, p_dest, length, bit_width)) {
        while (1)
            ;
    }

    // Wait until RX is complete.
    sci_spi_event_check();
}

void spiSendReceive(spi_ctrl_t *const p_api_ctrl, void const *p_src,
                    void *p_dest, uint32_t const length,
                    spi_bit_width_t const bit_width) {
    sci_spi_instance_ctrl_t *p_ctrl = (sci_spi_instance_ctrl_t *)p_api_ctrl;
    R_BSP_IrqDisable(p_ctrl->p_cfg->tei_irq);
    spiTxComplete = false;
    R_BSP_IrqEnable(p_ctrl->p_cfg->tei_irq);
    R_SCI_SPI_WriteRead(p_api_ctrl, p_src, p_dest, length, bit_width);

    // Wait until RX is complete.
    sci_spi_event_check();
}

// This is a nasty hack but I'm too lazy.
// Have a global variable to pend on until the
// UART TX has been completed. Would be nice
// to have that in hardware, but Renesas doesn't do that.
bool uartTxComplete = false;

void uartRxIntHandler(uart_callback_args_t *p_args) {
    // Read the FIFO and put in a queue.
    if (UART_EVENT_RX_CHAR == p_args->event) {
        char uartRxChar = (uint8_t)p_args->data;

        // Handle escape char sequence.
        // Ideally, this should be handled by something else than the ISR,
        // since we don't want to wait in the ISR.yy
        if (uartRxChar != 255 && uartRxChar != 27) {
            // hooray, there is a character in the rx buffer
            // which is now read!
            // Push that to the queue.
            if (!xQueueSendToBackFromISR(uartReceiveQueue, (void *)&uartRxChar,
                                         (TickType_t)0)) {
                while (1)
                    ;
            }
        }
    }
    if (UART_EVENT_TX_COMPLETE == p_args->event) {
        uartTxComplete = true;
    }
}

static void uartSend(uart_ctrl_t *const p_api_ctrl, uint8_t const *const p_src,
                     uint32_t const bytes) {
    // Enter critical section as to not overwrite a variable set
    // in an interrupt.
    sci_uart_instance_ctrl_t *p_ctrl = (sci_uart_instance_ctrl_t *)p_api_ctrl;
    // taskENTER_CRITICAL();
    R_BSP_IrqDisable(p_ctrl->p_cfg->tei_irq);
    uartTxComplete = false;
    R_BSP_IrqEnableNoClear(p_ctrl->p_cfg->tei_irq);
    // taskEXIT_CRITICAL();

    // Use the FSP function to transmit the UART message:
    R_SCI_UART_Write(p_api_ctrl, p_src, bytes);

    // Wait until TX is complete.
    while (!uartTxComplete)
        ;
}

static const char *const g_pcHex = "0123456789abcdef";

void UARTvprintf(const char *pcString, va_list vaArgP) {
    int stringLen = vsnprintf(NULL, 0, pcString, vaArgP);
    if (stringLen > 0) {
        char *strBuf = malloc(stringLen * sizeof(char));
        if (strBuf != NULL) {
            vsprintf(strBuf, pcString, vaArgP);
            uartSend(&g_uart0_ctrl, strBuf, stringLen);
        }
        free(strBuf);
    }
}
