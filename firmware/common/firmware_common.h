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
#ifndef FIRMWARE_COMMON_H_
#define FIRMWARE_COMMON_H_
// Standard libraries
#include <stdint.h>


/**
 * @brief Main thread. Calls init functions and starts
 * the other threads. 
 * @param p Pointer to arguments
 * @return Nothing
 */
void mainThread(void *p);

/**
 * @brief Task for testing the display.
 * @param p Pointer to arguments
 * @return Nothing
 */
void displayTestThread(void *p);

/**
 * @brief ISR for 1 Hz timer
 * @return Nothing
 */
void Timer1HzIntHandler(void);

/**
 * @brief ISR for 60 Hz timer
 * @return Nothing
 */
void Timer60HzIntHandler(void);

#endif //FIRMWARE_COMMON_H_