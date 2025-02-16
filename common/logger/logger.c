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
 * This file implements a UART logger if TIVAWARE is defined,
 * otherwise it uses the normal stdout to print.
 *
 * It's also implemented to be thread safe when using FreeRTOS, using
 * a critical section. There are pros and cons to this approach, with
 * the alternative being using a semaphore instead. However, the semaphore
 * needs to be initialized and given, and a critical section can just be local
 * to this file, which makes it nice.
 */

// Standard library
#include <stdarg.h>

// Logger header
#include "logger.h"

#if defined(RP2040)
#include "pico/stdio_usb.h"
#include "pico/printf.h"
#include "pico/stdio.h"
#include "pico/stdio/driver.h"
void out_char_driver(char c, void *arg) {
    ((stdio_driver_t *)arg)->out_chars(&c, 1);
}

void UARTvprintf(const char *pcString, va_list vaArgP) {
    void *pDriver = &stdio_usb;
    // printf("IN LOGGER");

    vfctprintf(out_char_driver, pDriver, pcString, vaArgP);

    // int stringLen = vsnprintf(NULL, 0, pcString, vaArgP);
    // if (stringLen > 0) {
    //     char *strBuf = malloc(stringLen * sizeof(char));
    //     if (strBuf != NULL) {
    //         vsprintf(strBuf, pcString, vaArgP);
    //         usb_puts(UART0, strBuf);
    //     }
    //     free(strBuf);
    // }
}
#else
#include <stdio.h>
#endif

void logger(int8_t log_level, char *msg, ...) {
#if defined(LOG_LEVEL)
    // 0 is the highest prio log level.
    // Only print statements that are at or under the
    // LOG_LEVEL, given by the build.
#else
    // LOG_LEVEL has not been defined, always ignore then.
#define LOG_LEVEL LOGGER_LEVEL_NONE
#warning "WARNING: LOGGER NOT USED"
#endif // defined(LOG_LEVEL)
    if (log_level > LOG_LEVEL) {
        return;
    }
#if defined(RP2040)
    // Print using UART instead
    va_list vaArgP;
    va_start(vaArgP, msg);
    // Print the message, UARTvprintf is defined in rp2040_utils
    UARTvprintf(msg, vaArgP);
    va_end(vaArgP);
#else
    va_list argp;
    va_start(argp, msg);
    vprintf(msg, argp);
    va_end(argp);
#endif 
}
