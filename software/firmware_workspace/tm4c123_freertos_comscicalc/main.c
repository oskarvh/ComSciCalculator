/*
 * main.c
 *
 *  Created on: 16 maj 2023
 *      Author: oskar
 */


//*****************************************************************************
//
// freertos_demo.c - Simple FreeRTOS example.
//
// Copyright (c) 2012-2017 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
//
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
//
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
//
// This is part of revision 2.1.4.178 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdbool.h>
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/timer.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#include "comscicalc.h"

#include "EVE.h"
#include "display.h"
#include "uart_logger.h"

//*****************************************************************************
// Pin allocation:
// TM4C123GXL board:
// PA2: SCK0
// PA3: CS0 (Hardware CS)
// PA4: MISO0
// PA5: MOSI0
// PA6: Software CS
// PB0: PDN
// TO ADD: TM4C129 calculator board.
//*****************************************************************************

// TODO:
// There is an issue with the buffer being cleared when multiple
// UART inputs are being made
// Therefore, a timer shall trigger the display update, and
// the calculator core shall only be triggered on
// input. Thereby, the update event is not used, instead
// the current state shall be updated on the screen, no matter what.

// Moreover, a timer checks the UART input. I think it's probably better to
// limit this to not support escape sequences for now.


//! Display state - holding shared variables between calc core thread and display thread
displayState_t displayState;
//! Semaphore protecting the display state
xSemaphoreHandle displayStateSemaphore;
//! Event group which triggers a display update.
EventGroupHandle_t displayTriggerEvent;

//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************

// Queue for handling UART input
QueueHandle_t uartReceiveQueue;


//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName)
{
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    UARTprintf("\n\n======================WARNING======================\n");
    UARTprintf("Task %s had a stack overflow :(", pcTaskName);
    UARTprintf("\n\n===================================================\n");
    while(1)
    {

    }
}

//*****************************************************************************
//
// UART receive interrupt handler
//
//*****************************************************************************
/**
 * @brief Struct holding one UART entry, to enable fast
 * reception of UART data.
 */
typedef struct uartListEntry{
    /*
     * @param pNext Pointer to the next entry
     */
    void *pNext;
    /*
     * @param pPrev Pointer to the previous entry
     */
    void *pPrev;
    /*
     * @param c Char for this entry
     */
    char c;
}uartListEntry_t;
void uartRxIntHandler(void){
    // Clear the interrupt
    UARTIntClear(UART0_BASE, UART_INT_RX);
    // Read the FIFO and put in a queue.
    char uartRxChar = UARTCharGetNonBlocking(UART0_BASE);
    // Handle escape char sequence.
    // Ideally, this should be handled by something else than the ISR,
    // since we don't want to wait in the ISR.yy
    if(uartRxChar != 255 && uartRxChar != 27){
        // hooray, there is a character in the rx buffer
        // which is now read!
        // Push that to the queue.
        if(!xQueueSendToBackFromISR(uartReceiveQueue, (void*)&uartRxChar, (TickType_t)0)){
            while(1);
        }
    }
}

// reset the cursor timer
void resetCursorTimer(void){
    //ROM_TimerDisable(TIMER0_BASE, TIMER_A);
    //HWREG(TIMER0_BASE+0x50)=0;
    //ROM_TimerEnable(TIMER0_BASE, TIMER_A);
}

// Timer to check if there are any messages in the UART buffer.
void Timer0BIntHandler(void){
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt
    if(UARTCharsAvail(UART0_BASE)){
        char uartRxChar = UARTCharGetNonBlocking(UART0_BASE);
        // Check if start of the escape sequence
        if(uartRxChar == 27){
            // Check if another char is available, max wait until next timer event
            while(!UARTCharsAvail(UART0_BASE)){
                if(TimerIntStatus(TIMER1_BASE, TIMER_A)){
                    break;
                }
            }
            uartRxChar = UARTCharGetNonBlocking(UART0_BASE);

            if(uartRxChar == 91){
                // Not the correct espace sequence, break and continue to process
                while(!UARTCharsAvail(UART0_BASE)){
                    if(TimerIntStatus(TIMER1_BASE, TIMER_A)){
                        break;
                    }
                }
                uartRxChar = UARTCharGetNonBlocking(UART0_BASE);
                // This is an escaped sequence, i.e. we're dealing with the
                // arrow keys.
                switch(uartRxChar){
                case 65:
                    // Up, replace with U
                    uartRxChar = 'U';
                    break;
                case 66:
                    // dOwn, replace with O
                    uartRxChar = 'O';
                    break;
                case 67:
                    // Right, replace with R
                    uartRxChar = 'R';
                    break;
                case 68:
                    // Left, replace with L
                    uartRxChar = 'L';
                    break;
                default:
                    //uartRxChar = 255; // Ignore this
                    break;
                }
            }

        }
        if(uartRxChar != 255 && uartRxChar != 27){
            // hooray, there is a character in the rx buffer
            // which is now read!
            // Push that to the queue.
            if(!xQueueSendToBackFromISR(uartReceiveQueue, (void*)&uartRxChar, (TickType_t)0)){
                while(1);
            }
            resetCursorTimer();
        }
    }
}

//
void Timer0AIntHandler(void)
{
    ROM_TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Clear the timer interrupt
    // Set the cursor event.
    xEventGroupSetBitsFromISR(displayTriggerEvent, DISPLAY_EVENT_CURSOR, NULL);
}

//*****************************************************************************
//
// Configure the UART and its pins.  This must be called before UARTprintf().
//
//*****************************************************************************
void ConfigureUART(void)
{
    // Enable the GPIO Peripheral used by the UART.
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

    // Enable UART0
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);

    // Configure GPIO Pins for UART mode.
    ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
    ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
    ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // Use the internal 16MHz oscillator as the UART clock source.
    UARTClockSourceSet(UART0_BASE, UART_CLOCK_PIOSC);

    // Initialize the UART for console I/O.
    UARTStdioConfig(0, 115200, 16000000);

#ifdef RX_ISR
    // Disable the UART FIFO to get an interrupt on each
    // char. Not recommended, but I couldn't find another way
    // quickly to get an interrupt on each char.
    UARTFIFODisable(UART0_BASE);

    // Register an interrupt for the UART receive
    UARTIntEnable(UART0_BASE, UART_INT_RX);
#ifdef PART_TM4C123GH6PM
    IntPrioritySet(INT_UART0_TM4C123, configMAX_SYSCALL_INTERRUPT_PRIORITY+3);
#elif PART_TM4C129 //TODO

#endif
    UARTIntRegister(UART0_BASE, &uartRxIntHandler);
#endif
}

void initTimer()
{
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    ROM_TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);   // 32 bits Timer
    IntPrioritySet(INT_TIMER0A_TM4C123, configMAX_SYSCALL_INTERRUPT_PRIORITY+4);
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AIntHandler);    // Registering  isr
    ROM_TimerEnable(TIMER0_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER0A);
    ROM_TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    uint8_t freqHz = 1;   // frequency in Hz
    uint32_t period = (SysCtlClockGet() / freqHz)/ 2;
    ROM_TimerLoadSet(TIMER0_BASE, TIMER_A, period -1);

    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    ROM_TimerConfigure(TIMER1_BASE, TIMER_CFG_PERIODIC);   // 32 bits Timer
    // Set timer 0B to 30 Hz to check if there is any UART chars in the FIFO
    IntPrioritySet(INT_TIMER1A_TM4C123, configMAX_SYSCALL_INTERRUPT_PRIORITY+3);
    TimerIntRegister(TIMER1_BASE, TIMER_A, Timer0BIntHandler);    // Registering  isr
    ROM_TimerEnable(TIMER1_BASE, TIMER_A);
    ROM_IntEnable(INT_TIMER1A);
    ROM_TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    freqHz = 60;   // frequency in Hz
    period = (SysCtlClockGet() / freqHz)/ 2;
    ROM_TimerLoadSet(TIMER1_BASE, TIMER_A, period -1);

}



//*****************************************************************************
//
// Configure display and peripherals used by the display.
//
//*****************************************************************************
void initDisplay(void){
    // Enable the GPIO peripherals for PDN and software CSn
    ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
    GPIOPinTypeGPIOOutput(GPIO_PORTB_BASE, GPIO_PIN_0);
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_6);

    // Initialize the SPI and subsequently the display
    EVE_SPI_Init();
    EVE_init();
    while (EVE_busy());

}

//*****************************************************************************
//
// Task that handles the calculator core
//
//*****************************************************************************
void calcCoreTask(void *p){
    // Create the calculator state variable
    calcCoreState_t calcState;

    // Initialize the calculator core
    if(calc_coreInit(&calcState) != calc_funStatus_SUCCESS){
        // There is an error with the calculator state.
        while(1);
    }

    // TODO: Initialize based on what was saved in flash
    // For now though, just initialize to decimal base.

    calcState.numberFormat.inputBase = inputBase_DEC;
    calcState.numberFormat.numBits = 64;
    calcState.numberFormat.sign = false;
    calcState.numberFormat.formatBase = INPUT_FMT_INT;
    while(1){
        // Wait for UART data to be available in the queue
        if(uartReceiveQueue != 0){

            // Read out data from the UART queue while there is data in there
            // The do-while loop here is to enable the task to sleep while
            // it's waiting on the input.
            do {
                char receiveChar = 0;
                calc_funStatus_t addRemoveStatus;
                if(xQueueReceive(uartReceiveQueue, &receiveChar, (TickType_t)portMAX_DELAY )){
                    // TODO: Check that we're in a state to add chars to the
                    // calc core. The option here could be if we're in
                    // the menu for example.
                    if(receiveChar == 127){
                        addRemoveStatus = calc_removeInput(&calcState);
                    } else {
                        if(receiveChar == 'U'){
                            // TODO
                        }
                        if(receiveChar == 'D'){
                            // TODO
                        }
                        if(receiveChar == 'L'){
                            calcState.cursorPosition += 1;
                        }
                        if(receiveChar == 'R'){
                            if(calcState.cursorPosition > 0){
                                calcState.cursorPosition -= 1;
                            }
                        }
                        if(receiveChar == 'i' || receiveChar == 'I'){
                            // Update the input base.
                            calcState.numberFormat.inputBase += 1;
                            if(calcState.numberFormat.inputBase > 2){
                                calcState.numberFormat.inputBase = 0;
                            }
                            calc_updateBase(&calcState);
                        }
                        // The add input contains valuable checks.
                        addRemoveStatus = calc_addInput(&calcState, receiveChar);
                    }
                    // Check if the input wasn't accepted
                    if(addRemoveStatus == calc_funStatus_UNKNOWN_INPUT){
                        // For now, do nothing here. Might trigger something later on
                        // Could be kind of cool to blink the button red or something.
                    }
                    // Check if there was an issue with adding/removing input
                    else if(addRemoveStatus != calc_funStatus_SUCCESS){
                        // There was an issue with adding or removing
                        // input.
                        // TODO: call the error collection instead.
                        //while(1);
                    }
                }
            } while(uxQueueMessagesWaiting(uartReceiveQueue) > 0);

            // Call the solver
            calc_funStatus_t solveStatus = calc_solver(&calcState);

            // Here, all the results needed for the display task is available
            // Therefore, obtain the semaphore and start writing to the displayState struct

            // All input has been read, ready to solve the current state of the input buffer.
            // Reset the result to 0 to have a clean slate.
            //calcState.result = 0;
            if( xSemaphoreTake( displayStateSemaphore, portMAX_DELAY) == pdTRUE ){
                // Set the output buffer to all null terminators.
                memset(displayState.printedInputBuffer, 0, MAX_PRINTED_BUFFER_LEN);


                displayState.syntaxIssueIndex = -1;
                displayState.printStatus = calc_printBuffer(&calcState,
                                                            displayState.printedInputBuffer,
                                                            MAX_PRINTED_BUFFER_LEN,
                                                            &displayState.syntaxIssueIndex);
                displayState.solveStatus = solveStatus;
                if(solveStatus == calc_solveStatus_SUCCESS){
                    displayState.result = calcState.result;
                }
                displayState.cursorLoc = calc_getCursorLocation(&calcState);
                // Give the semaphore back
                xSemaphoreGive(displayStateSemaphore);
            }
            xEventGroupSetBits(displayTriggerEvent, DISPLAY_EVENT_NEW_DATA);
        }
    }
}

//*****************************************************************************
//
// Initialize FreeRTOS, initialize the hardware and kick off all tasks
//
//*****************************************************************************
int
main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    // NOTE: The SYSCTL_XTAL_ value must be changed to match the value of the
    // crystal on your board.
#if defined(TARGET_IS_TM4C129_RA0) ||                                         \
    defined(TARGET_IS_TM4C129_RA1) ||                                         \
    defined(TARGET_IS_TM4C129_RA2)
    ui32SysClock = SysCtlClockFreqSet((SYSCTL_XTAL_25MHZ |
                                       SYSCTL_OSC_MAIN |
                                       SYSCTL_USE_OSC), 25000000);
#else
    //SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
    //               SYSCTL_XTAL_16MHZ);

    // Set the clocking to run at 50 MHz from the PLL.
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ |
                       SYSCTL_OSC_MAIN);
#endif

    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    ConfigureUART();
    initDisplay();
    initDisplayState(&displayState);

    // Create the binary semaphore to protect the display state
    displayStateSemaphore = xSemaphoreCreateBinary();
    if( displayStateSemaphore == NULL )
    {
        while(1);
    }

    // Create the synchronization event between the calculator task
    // and the display task
    displayTriggerEvent = xEventGroupCreate();

    // Create the queue used by the display and UART
    // 100 elements of chars is way more than needed for
    // human input, but for HIL test execution, this might be
    // necessary
    uartReceiveQueue = xQueueCreate(100, sizeof(char));

    TaskHandle_t screenTaskHandle = NULL;
    TaskHandle_t calcCoreTaskHandle = NULL;

    // Create the task, storing the handle.
    xTaskCreate(
            displayTask, // Function that implements the task.
            "DISPLAY",               // Text name for the task.
            700,                    // Stack size in words, not bytes.
            ( void * ) 1,            // Parameter passed into the task.
            tskIDLE_PRIORITY,        // Priority at which the task is created.
            &screenTaskHandle );              // Used to pass out the created task's handle.
    xTaskCreate(
            calcCoreTask, // Function that implements the task.
            "CALCCORE",               // Text name for the task.
            700,                    // Stack size in words, not bytes.
            ( void * ) 1,            // Parameter passed into the task.
            tskIDLE_PRIORITY,        // Slightly higher priority than display task
            &calcCoreTaskHandle );              // Used to pass out the created task's handle.

    // Start by giving the semaphore, as the semaphore needs to initialize once
    xSemaphoreGive(displayStateSemaphore);

    // Initialize the timer after the tasks have been created to minimize the
    // risk of setting the off the timer before the task are run.
    initTimer();

    // Start the scheduler.  This should not return.
    vTaskStartScheduler();

    // If the scheduler returns for some reason, just loop here.
    while(1)
    {
    }
}
