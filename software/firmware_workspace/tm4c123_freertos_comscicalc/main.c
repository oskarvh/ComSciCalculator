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
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/rom.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "comscicalc.h"

#include "EVE.h"
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


//*****************************************************************************
//
// The mutex that protects concurrent access of UART from multiple tasks.
//
//*****************************************************************************
xSemaphoreHandle g_pUARTSemaphore;

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
void uartRxIntHandler(void){
    // Clear the interrupt
    UARTIntClear(UART0_BASE, UART_INT_RX);
    // Read the FIFO and put in a queue.
    char uartRxChar = UARTCharGetNonBlocking(UART0_BASE);
    if(uartRxChar != -1){
        // hooray, there is a character in the rx buffer
        // which is now read!
        // Push that to the queue.
        if(!xQueueSendToBackFromISR(uartReceiveQueue, (void*)&uartRxChar, (TickType_t)0)){
            while(1);
        }
    }
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

    // Disable the UART FIFO to get an interrupt on each
    // char. Not recommended, but I couldn't find another way
    // quickly to get an interrupt on each char.
    UARTFIFODisable(UART0_BASE);

    // Register an interrupt for the UART receive
    UARTIntEnable(UART0_BASE, UART_INT_RX);
#ifdef PART_TM4C123GH6PM
    IntPrioritySet(INT_UART0_TM4C123, 200);
#else if PART_TM4C129 //TODO

#endif
    UARTIntRegister(UART0_BASE, &uartRxIntHandler);
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

    // Initialize the calculator core (Should be done elsewhere?)
    calc_coreInit(NULL);

    // Initialize the SPI and subsequently the display
    EVE_SPI_Init();
    EVE_init();
    EVE_start_cmd_burst();
    EVE_cmd_dl_burst(CMD_DLSTART);
    EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | 0);
    EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    EVE_color_rgb_burst(0xFFFFFF);
    //EVE_cmd_text_burst(5, 15, 28, 0, rxBuf);
    EVE_cmd_dl_burst(DL_DISPLAY);
    EVE_cmd_dl_burst(CMD_SWAP);
    EVE_end_cmd_burst();
    while (EVE_busy());

}

//*****************************************************************************
//
// Task to print the UART buffer to the screen
//
//*****************************************************************************

void displayUartOnScreenTask(void *p){
    char rxBuf[100] = {0};
    uint8_t charCounter = 0;
    while(1){
        // Check if data is available
        if(uartReceiveQueue != 0){
            char receiveChar = 0;
            if(xQueueReceive(uartReceiveQueue, &receiveChar, (TickType_t)5)){
                // Check if received byte is newline, in which case
                // clear
                if(receiveChar == 13){
                    memset(rxBuf, 0, 100);
                    charCounter = 0;
                }
                // If received byte is backspace, remove the last character
                else if(receiveChar == 127){
                    if(charCounter > 0){
                        charCounter -= 1;
                    }
                    rxBuf[charCounter] = 0;
                }
                // Otherwise glue it on to the end, if there is room
                else if(charCounter < 100){
                    rxBuf[charCounter++] = receiveChar;
                }
            }
            // Update the screen:
            // Send an initial message here

            EVE_start_cmd_burst();
            EVE_cmd_dl_burst(CMD_DLSTART);
            EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | 0);
            EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
            EVE_color_rgb_burst(0xFFFFFF);
            EVE_cmd_text_burst(5, 15, 28, 0, rxBuf);
            EVE_cmd_dl_burst(DL_DISPLAY);
            EVE_cmd_dl_burst(CMD_SWAP);
            EVE_end_cmd_burst();
            while (EVE_busy());

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

    // Print demo introduction.
    UARTprintf("\n\nWelcome to the EK-TM4C123GXL FreeRTOS Demo!\n");

    // Create a mutex to guard the UART.
    g_pUARTSemaphore = xSemaphoreCreateMutex();

    // Create the queue used by the display and UART
    uartReceiveQueue = xQueueCreate(10, sizeof(char)); // Ten elements of chars
    BaseType_t xReturned;
    TaskHandle_t xHandle = NULL;

    // Create the task, storing the handle.
    xReturned = xTaskCreate(
            displayUartOnScreenTask,               // Function that implements the task.
                    "DISPLAY",             // Text name for the task.
                    1000,               // Stack size in words, not bytes.
                    ( void * ) 1,       // Parameter passed into the task.
                    tskIDLE_PRIORITY,   // Priority at which the task is created.
                    &xHandle );         // Used to pass out the created task's handle.

    if( xReturned == pdPASS )
    {
        // The task was created.  Use the task's handle to delete the task.
        //vTaskDelete( xHandle );
    }

    UARTprintf("\n\nInit task complete!\n");
    //
    // Start the scheduler.  This should not return.
    //
    vTaskStartScheduler();

    //
    // In case the scheduler returns for some reason, print an error and loop
    // forever.
    //

    while(1)
    {
    }
}
