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

#include "uart_logger.h"

// FreeRTOS, if tivaware is used
#ifdef TIVAWARE
#include "FreeRTOS.h"
#include "task.h"
#else
#include <stdio.h>
#endif

void logger(char *msg, ...) {
#ifdef VERBOSE
#ifdef TIVAWARE
    // Enter critical section
    taskENTER_CRITICAL();

    // Print using UART instead
    va_list vaArgP;
    va_start(vaArgP, msg);
    UARTvprintf(msg, vaArgP);
    va_end(vaArgP);

    // Exit critical section.
    taskEXIT_CRITICAL();
#else
    va_list argp;
    va_start(argp, msg);
    vprintf(msg, argp);
    va_end(argp);
#endif
#endif
}
