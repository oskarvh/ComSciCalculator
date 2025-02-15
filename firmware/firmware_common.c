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
 * This file contains all the common firmware behaviour,
 * such as threading etc. Note that this should only contain
 * functions common to all MCU platforms!
 */

#include "firmware_common.h"
#include "EVE.h"

// Comscicalc includes
#include "comscicalc.h"
#include "display.h"
#include "logger.h"

// C includes
#include <string.h>

// Hardware dependent includes
#if defined(RP2040)
#include "rp2040_utils.h"
#endif

// FreeRTOS includes
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

//! Display state - holding shared variables between calc core thread and
//! display thread
displayState_t displayState;
//! Queue for handling UART input
QueueHandle_t uartReceiveQueue;
//! Semaphore protecting the display state
xSemaphoreHandle displayStateSemaphore;
//! Event group which triggers a display update.
EventGroupHandle_t displayTriggerEvent;

// Timer ISR
void Timer1HzIntHandler(void) {
    // Set the cursor event.
    xEventGroupSetBitsFromISR(displayTriggerEvent, DISPLAY_EVENT_CURSOR, NULL);
}

// Timer ISR
void Timer60HzIntHandler(void) {
    // For now, empty ISR
}

/**
 * @brief Task that handles the calculator core functions.
 * the other threads.
 * @param p Pointer to task arguments
 * @return Nothing
 */
static void calcCoreTask(void *p) {
    // Create the calculator state variable
    calcCoreState_t calcState;

    // Initialize the calculator core
    if (calc_coreInit(&calcState) != calc_funStatus_SUCCESS) {
        // There is an error with the calculator state.
        while (1)
            ;
    }

    // TODO: Initialize based on what was saved in flash
    // For now though, just initialize to decimal base.

    calcState.numberFormat.inputBase = inputBase_DEC;
    calcState.numberFormat.numBits = 64;
    calcState.numberFormat.sign = false;

    // Boolean to check if we should keep waiting for the menu to exit.
    bool inMenu = false;
    while (1) {
        // Wait for UART data to be available in the queue
        if (uartReceiveQueue != 0) {

            // Read out data from the UART queue while there is data in there
            // The do-while loop here is to enable the task to sleep while
            // it's waiting on the input.
            do {
                char receiveChar = 0;
                calc_funStatus_t addRemoveStatus;
                if (xQueueReceive(uartReceiveQueue, &receiveChar,
                                  (TickType_t)portMAX_DELAY)) {
                    // TODO: Check that we're in a state to add chars to the
                    // calc core. The option here could be if we're in
                    // the menu for example.

                    if (receiveChar == 127) {
                        addRemoveStatus = calc_removeInput(&calcState);
                    } else {
                        // Check if 91 was received, which for USB
                        // is an escape key for some reason?
                        // Also check if there's anything else in the queue

                        if (receiveChar == 27) {
                            // Escape char. We expect two more chars in there,
                            // but don't wait around for it
                            char escapeSeq[3] = {0};
                            for (int i = 0; i < 2; i++) {
                                xQueueReceive(uartReceiveQueue, &(escapeSeq[i]),
                                              (TickType_t)10);
                            }

                            escapeSeq[2] = '\0';
                            if (strcmp(escapeSeq, "[A") == 0) {
                                // Up
                            }
                            if (strcmp(escapeSeq, "[B") == 0) {
                                // Down
                            }
                            if (strcmp(escapeSeq, "[C") == 0) {
                                // Forward/right
                                if (calcState.cursorPosition > 0) {
                                    calcState.cursorPosition -= 1;
                                }
                            }
                            if (strcmp(escapeSeq, "[D") == 0) {
                                // Backward/left
                                if (calcState.pListEntrypoint != NULL) {
                                    // Only increase if we have an actual entry point
                                    calcState.cursorPosition += 1;    
                                }
                            }
                            // Here there's a USB espace char, and something
                            // else in the queue C (right), D(left), A (up) or ?
                            // (down)
                        }
                        if (receiveChar == 'i' || receiveChar == 'I') {
                            // Update the input base.
                            calcState.numberFormat.inputBase += 1;
                            if (calcState.numberFormat.inputBase > 2) {
                                calcState.numberFormat.inputBase = 0;
                            }
                            calc_updateBase(&calcState);
                        }
                        if (receiveChar == 'm' || receiveChar == 'M') {
                            // Update the input format (int, float, fixed)
                            uint8_t inputFormat =
                                calcState.numberFormat.inputFormat;
                            inputFormat += 1;
                            if (inputFormat >= INPUT_FMT_RESERVED) {
                                inputFormat = 0;
                            }
                            calc_updateInputFormat(&calcState, inputFormat);
                        }
                        if (receiveChar == 'o' || receiveChar == 'O') {
                            // Update the output format (int, float, fixed)
                            uint8_t outputFormat =
                                calcState.numberFormat.outputFormat;
                            outputFormat += 1;
                            if (outputFormat >= INPUT_FMT_RESERVED) {
                                outputFormat = 0;
                            }
                            calc_updateOutputFormat(&calcState, outputFormat);
                        }
                        if (receiveChar == 't' || receiveChar == 'T') {
                            // Placeholder menu state.
                            inMenu = true;
                        }

                        // The add input contains valuable checks.
                        addRemoveStatus =
                            calc_addInput(&calcState, receiveChar);
                    }
                    // Check if the input wasn't accepted
                    if (addRemoveStatus == calc_funStatus_UNKNOWN_INPUT) {
                        // For now, do nothing here. Might trigger something
                        // later on Could be kind of cool to blink the button
                        // red or something.
                    }
                    // Check if there was an issue with adding/removing input
                    else if (addRemoveStatus != calc_funStatus_SUCCESS) {
                        // There was an issue with adding or removing
                        // input.
                        // TODO: call the error collection instead.
                        // while(1);
                    }
                }
            } while (uxQueueMessagesWaiting(uartReceiveQueue) > 0);

            // Call the solver
            calc_funStatus_t solveStatus = calc_solver(&calcState);

            // Here, all the results needed for the display task is available
            // Therefore, obtain the semaphore and start writing to the
            // displayState struct

            // All input has been read, ready to solve the current state of the
            // input buffer. Reset the result to 0 to have a clean slate.
            // calcState.result = 0;
            if (xSemaphoreTake(displayStateSemaphore, portMAX_DELAY) ==
                pdTRUE) {
                if (inMenu) {
                    displayState.inMenu = true;
                }
                // Set the output buffer to all null terminators.
                memset(displayState.printedInputBuffer, 0,
                       MAX_PRINTED_BUFFER_LEN);

                displayState.syntaxIssueIndex = -1;
                displayState.printStatus = calc_printBuffer(
                    &calcState, displayState.printedInputBuffer,
                    MAX_PRINTED_BUFFER_LEN, &displayState.syntaxIssueIndex);
                displayState.solveStatus = solveStatus;
                if (solveStatus == calc_solveStatus_SUCCESS) {
                    displayState.result = calcState.result;
                }
                if (displayState.printStatus ==
                    calc_funStatus_INPUT_LIST_NULL) {
                    // If there is no result due to the input list being 0,
                    // that means that there wasn't any input chars. So set the
                    // result to 0
                    displayState.result = 0;
                }
                displayState.cursorLoc = calc_getCursorLocation(&calcState);
                memcpy(&(displayState.inputOptions), &(calcState.numberFormat),
                       sizeof(numberFormat_t));
                // Give the semaphore back
                xSemaphoreGive(displayStateSemaphore);
            }
            xEventGroupSetBits(displayTriggerEvent, DISPLAY_EVENT_NEW_DATA);

            while (inMenu) {
                // We have given the display trigger event.
                // We need to just hold tight here and wait for
                // the user to exit the menu.

                // Wait for the menu exit event:
                uint32_t eventbits =
                    xEventGroupWaitBits(displayTriggerEvent, DISPLAY_EXIT_MENU,
                                        pdTRUE, pdFALSE, portMAX_DELAY);
                // Check if we can unlock this task
                if (xSemaphoreTake(displayStateSemaphore, portMAX_DELAY)) {
                    // Copy the display state to the local state to get out of
                    // the menu
                    inMenu = displayState.inMenu;
                    // Copy over any changes made to the input state:
                    if (calcState.numberFormat.fixedPointDecimalPlace !=
                        displayState.inputOptions.fixedPointDecimalPlace) {
                        // TODO: Update the fixed point decimal place from the
                        // calcState POV
                    }
                    if (calcState.numberFormat.inputBase !=
                        displayState.inputOptions.inputBase) {
                        calcState.numberFormat.inputBase =
                            displayState.inputOptions.inputBase;
                        calc_updateBase(&calcState);
                    }
                    if (calcState.numberFormat.inputFormat !=
                        displayState.inputOptions.inputFormat) {
                        calc_updateInputFormat(
                            &calcState, displayState.inputOptions.inputFormat);
                    }
                    if (calcState.numberFormat.numBits !=
                        displayState.inputOptions.numBits) {
                        // TODO: Make an update function.
                    }
                    if (calcState.numberFormat.outputFormat !=
                        displayState.inputOptions.outputFormat) {
                        calc_updateOutputFormat(
                            &calcState, displayState.inputOptions.outputFormat);
                    }
                    if (calcState.numberFormat.sign !=
                        displayState.inputOptions.sign) {
                        // TODO: make update function.
                    }
                    // Just copy the number format just in case.
                    memcpy(&(calcState.numberFormat),
                           &(displayState.inputOptions),
                           sizeof(numberFormat_t));
                    xSemaphoreGive(displayStateSemaphore);
                }
            }
        }
    }
}

/**
 * @brief Initialize the FT810 EVE driver and SPI interface
 * @return Nothing
 */
static void initDisplay(void) {
    // Initialize the SPI and subsequently the display
    EVE_init_spi();
    EVE_init();
    while (EVE_busy())
        ;
}

void displayTestThread(void *p) {
    // Init UART to enable the logger
    if (!initUart()) {
        // Something went wrong with initializing UART.
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_DEBUG, "DISPLAY TEST: UART STARTED\r\n");
    // Init the SPI
    if (!initSpi()) {
        // Something went wrong with initializing UART.
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_DEBUG, "DISPLAY TEST: SPI STARTED\r\n");
    // Initialize the display driver
    initDisplay();
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: DISPLAYED INITIALIZED\r\n");
    // Call the display test function.
    testDisplay();
}

// Main thread.
// This is a simple thread that is responsible
// to start other threads, and init the hardware
void mainThread(void *p) {
    // Create the queue for the UART receive queue:
    uartReceiveQueue = xQueueCreate(100, sizeof(char));
    if (uartReceiveQueue == NULL) {
        while (1)
            ;
    }

    // Create the binary semaphore to protect the display state
    displayStateSemaphore = xSemaphoreCreateBinary();
    if (displayStateSemaphore == NULL) {
        while (1)
            ;
    }

    // Create the synchronization event between the calculator task
    // and the display task
    displayTriggerEvent = xEventGroupCreate();

    // Initialize UART.
    // NOTE: This is MCU specific, so the initUart function must be
    // linked in based on which MCU is built.
    if (!initUart()) {
        // Something went wrong with initializing UART.
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: UART INIT'D\r\n");

    // Initialize SPI.
    // NOTE: This is MCU specific, so the initSpi function must be
    // linked in based on which MCU is built.
    if (!initSpi()) {
        // Something went wrong with initializing offboard SPI.
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: SPI INIT'D\r\n");
    // Initialize timers.
    // NOTE: This is MCU specific, so the initTimer function must be
    // linked in based on which MCU is built.
    if (!initTimer()) {
        // Something went wrong with initializing UART.
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: TIMER INIT'D\r\n");
    // Initialize the FT810 EVE display driver.
    // Note, this is using the FT810 driver functions,
    // which will initialize SPI. Ensure the implementation
    // does not do this twice.
    initDisplay();
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: DISPLAYED INIT'D\r\n");
    // Initialize the displaystate variable
    initDisplayState(&displayState);

    TaskHandle_t screenTaskHandle = NULL;
    TaskHandle_t calcCoreTaskHandle = NULL;
    // Create the task that handles the display
    xTaskCreate(displayTask,          // Function that implements the task.
                "DISPLAY",            // Text name for the task.
                700,                  // Stack size in words, not bytes.
                (void *)1,            // Parameter passed into the task.
                tskIDLE_PRIORITY + 1, // Priority at which the task is created.
                &screenTaskHandle // Used to pass out the created task's handle.
    );
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: DISPLAY TASK CREATED\r\n");
    // Create the task that handles the calculator core.
    xTaskCreate(
        calcCoreTask,       // Function that implements the task.
        "CALCCORE",         // Text name for the task.
        2000,               // Stack size in words, not bytes.
        (void *)1,          // Parameter passed into the task.
        tskIDLE_PRIORITY,   // Slightly higher priority than display task
        &calcCoreTaskHandle // Used to pass out the created task's handle.
    );
    logger(LOGGER_LEVEL_DEBUG, "DEBUG: CALC CORE TASK CREATED\r\n");

    // Start by giving the semaphore, as the semaphore needs to initialize once
    xSemaphoreGive(displayStateSemaphore);

    // Wait 1 second before starting the timers.
    vTaskDelay(100 / portTICK_PERIOD_MS);

    // Now that the tasks have been created, start the timers.
    startTimer();

    // Suspend this thread
    vTaskSuspend(NULL);
}