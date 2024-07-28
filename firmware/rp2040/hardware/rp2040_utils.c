/*
MIT License

Copyright (c) 2024 Oskar von Heideken

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
 * This file includes function mappings used by firmware_common, 
 * which are specific to the RP2040 comSciCalc rev 1 board.
 */

// Standard library
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Header
#include "rp2040_utils.h"

// Hardware specific
#include "pin_map.h"
#include "hardware/uart.h"

// FW common, used for callback linking.
#include "firmware_common.h"

bool mcuInit(void){
    stdio_init_all();
}

bool initUart(void){
    uart_init(UART0, UART0_BR);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
}

bool initSpi(void){
    // Note: This the is the SPI for the offboard functions.
}

// Globally defined timer handlers.
repeating_timer_t rtA;
repeating_timer_t rtB;

// Wrapped timer callback
bool Timer1HzIntHandlerWrapper(repeating_timer_t *rt){
    Timer1HzIntHandler();
    return true; // Keep the timer going
}

// Wrapped timer callback
bool Timer60HzIntHandlerWrapper(repeating_timer_t *rt){
    Timer60HzIntHandler();
    return true; // Keep the timer going
}

bool initTimer(void) {
    // Init one 1 Hz timer and one 60 Hz timer with callbacks
    // For the 1 Hz timer, Timer1HzIntHandler should be called
    // and for the 60 Hz timer, Timer60HzIntHandler should be called. 
    // These functions are found in FW common. 
    add_repeating_timer_ms(1000, Timer1HzIntHandlerWrapper, NULL, &rtA);
    add_repeating_timer_us(16666, Timer60HzIntHandlerWrapper, NULL, &rtB);
}

void startTimer(void) {
}

void UARTvprintf(const char *pcString, va_list vaArgP) {
    int stringLen = vsnprintf(NULL, 0, pcString, vaArgP);
    if (stringLen > 0) {
        char *strBuf = malloc(stringLen * sizeof(char));
        if (strBuf != NULL) {
            vsprintf(strBuf, pcString, vaArgP);
            uart_puts(UART0, strBuf);
        }
        free(strBuf);
    }
}