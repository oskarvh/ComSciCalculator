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
This file contains functions that are specific to the RP2040 MCU, but used throughout
the firmware. 
To use another MCU, create a new file with the same function names, and include it in the
build system instead of this file. 
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
#include "hardware/uart.h"
#include "pin_map.h"

// Globally defined timer handlers.
repeating_timer_t rtA;
repeating_timer_t rtB;

// Wrapped timer callback
bool Timer1HzIntHandlerWrapper(repeating_timer_t *rt) {
    void (*pCallbackFun)(void) = rt->user_data;
    pCallbackFun();
    return true; // Keep the timer going
}

// Wrapped timer callback
bool Timer60HzIntHandlerWrapper(repeating_timer_t *rt) {
    void (*pCallbackFun)(void) = rt->user_data;
    pCallbackFun();
    return true; // Keep the timer going
}

// Read USB data
int rp2040_read_usb(uint32_t timeout_us) {
    return getchar_timeout_us(timeout_us);
}

// Init the RP2040 HW
bool mcuInit(void) {
    stdio_init_all();
#if defined(DEBUG)
    // If the logger level is higher than error, then wait until
    // a valid USB connection is made.
    // TODO: Disable this in case just power is provided.
    // while(!stdio_usb_connected());
    sleep_ms(2000);
#endif
    return true;
}

// Init the UART(s) on the RP2040
bool initUart(void* pCallbackFun) {
    uart_init(UART0, UART0_BR);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);

    // Set the USB callback:
    stdio_set_chars_available_callback(pCallbackFun, NULL);
}

bool initSpi(void) {
    // Note: This the is the SPI for the offboard functions.
}

bool initTimer(void* p60HzCallback, void* p1HzCallback) {
    // Init one 1 Hz timer and one 60 Hz timer with callbacks
    // For the 1 Hz timer, Timer1HzIntHandler should be called
    // and for the 60 Hz timer, Timer60HzIntHandler should be called.
    // These functions are found in FW common.
    add_repeating_timer_ms(1000, Timer1HzIntHandlerWrapper, p1HzCallback, &rtA);
    add_repeating_timer_us(16666, Timer60HzIntHandlerWrapper, p60HzCallback, &rtB);
}

void startTimer(void) {}
