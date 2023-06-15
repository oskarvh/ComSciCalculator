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

#ifndef UART_LOGGER_H_
#define UART_LOGGER_H_

/**
 * @brief Prints logger statements to stdout, or UART if TIVAWARE is defined.
 *
 * @note The UART printing uses a FreeRTOS critical section protection.
 *
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return None.
 * @warning This uses critical section to protect the shared resources,
 * hence it should not be used in project with critical interrupt timings.
 * For that, the critical section should be replaced with a semaphore.
 */
void logger(char *msg, ...);

#endif /* UART_LOGGER_H_ */
