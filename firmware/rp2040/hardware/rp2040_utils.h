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
#include <stdbool.h>

// RP2040 stdlib
#include "pico/stdlib.h"

/**
 * @brief Init the RP2040 HW
 */
bool mcuInit(void);

/**
 * @brief Init the UART(s) on the RP2040
 */
bool initUart(void);

/**
 * @brief Init the offboard SPI.
 * @note The FT81x SPI interface is initialized in the EVE driver.
 */
bool initSpi(void);

/**
 * @brief Init the HW timer used for cursor blinking etc.
 */
bool initTimer(void);

/**
 * @brief Start the HW timer used for cursor blinking etc.
 */
void startTimer(void);

/**
 * @brief printf function used for UART logging with the RP2040
 * @param pcString Pointer string, must be null terminated.
 * @param vaArgP String formatting string
 */
void UARTvprintf(const char *pcString, va_list vaArgP);