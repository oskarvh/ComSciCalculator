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

// C
#include <stdlib.h>
#include <stdio.h>

// Freertos:
#include "FreeRTOS.h"
#include "task.h"

// Comscicalc entry points
#include "firmware_common.h"

#if defined(RP2040)
// RP2040
#include "rp2040_utils.h"
#endif

/**
 * @brief This hook is called by FreeRTOS when an stack overflow error is detected.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    // UARTprintf("\n\n======================WARNING======================\n");
    // UARTprintf("Task %s had a stack overflow :(", pcTaskName);
    // UARTprintf("\n\n===================================================\n");
    while (1) {}
}

/**
 * @brief This hook is called by FreeRTOS when an stack overflow error is detected.
 */
void vApplicationMallocFailedHook(void) {
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    // UARTprintf("\n\n======================WARNING======================\n");
    // UARTprintf("Task %s had a stack overflow :(", pcTaskName);
    // UARTprintf("\n\n===================================================\n");
    while (1) {}
}
/*
 * RUNTIME START
 */
int main() {
    // Init MCU hardware
    mcuInit();
    
//#define TEST_DISPLAY
#ifdef TEST_DISPLAY
    // Start the display test task
    TaskHandle_t displayTestTask = NULL;
    xTaskCreate(displayTestThread, "DISPLAY_TEST_THREAD", 1000, (void *)1,
                tskIDLE_PRIORITY, &displayTestTask);

#else
    // Start the main thread
    TaskHandle_t mainThreadHandle = NULL;
    xTaskCreate(mainThread, "MAIN_TASK", 200, (void *)1, tskIDLE_PRIORITY,
                &mainThreadHandle);
#endif
    // Start scheduler
    vTaskStartScheduler();
}