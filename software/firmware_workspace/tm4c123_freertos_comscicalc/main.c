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
#include "utils/uartstdio.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "comscicalc.h"

#include "EVE.h"

#define MAX_CALC_CORE_INPUT_LEN 100
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

// Queue for handling UART input
QueueHandle_t uartReceiveQueue;
// Queue for handling the input going through the calculator core
QueueHandle_t calcCoreInputTransformedQueue;
// Queue for handling the result
QueueHandle_t calcCoreOutputQueue;

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
    IntPrioritySet(INT_UART0_TM4C123, configMAX_SYSCALL_INTERRUPT_PRIORITY+2);
#elif PART_TM4C129 //TODO

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
calcCoreState_t calcState;
void calcCoreTask(void *p){
    // Initialize the calculator core

    if(calc_coreInit(&calcState) != calc_funStatus_SUCCESS){
        // There is an error.
        while(1);
    }
    calcState.inputBase = inputBase_DEC;
    while(1){
        // Wait for UART input
        if(uartReceiveQueue != 0){
            char receiveChar = 0;
            if(xQueueReceive(uartReceiveQueue, &receiveChar, (TickType_t)5)){
                // Check if char is backspace, in which case delete.
                if(receiveChar == 127){
                    calc_removeInput(&calcState);
                }
                else{
                    calc_addInput(&calcState, receiveChar);
                }
                // Solve:
                uint8_t solveState = calc_solver(&calcState);
                // Print the output
                char *pOutputString[MAX_CALC_CORE_INPUT_LEN] = {0};
                calc_printBuffer(&calcState, pOutputString, MAX_CALC_CORE_INPUT_LEN);
                if(!xQueueSendToBack(calcCoreInputTransformedQueue, (void*)&pOutputString, (TickType_t)0)){
                    while(1);
                }
                // Get the result and print to buffer
                if(!xQueueSendToBack(calcCoreOutputQueue, (void*)&calcState.result, (TickType_t)0)){
                    while(1);
                }
            }
        }
    }
}
//*****************************************************************************
//
// Task to print the UART buffer to the screen
//
//*****************************************************************************
// Input text definition
#define FONT_SIZE 18
#define INPUT_TEXT_XC0 (EVE_HSIZE - 5)
#define INPUT_TEXT_YC0 (EVE_VSIZE/2 - FONT_SIZE -  5)

#define OUTPUT_DEC_XC0 (EVE_HSIZE - 5)
#define OUTPUT_DEC_YC0 (EVE_VSIZE - FONT_SIZE - 5)
#define OUTPUT_BIN_XC0 (EVE_HSIZE - 5)
#define OUTPUT_BIN_YC0 (EVE_VSIZE*3/4  - FONT_SIZE - 5)
#define OUTPUT_HEX_XC0 (EVE_HSIZE/2 - 5)
#define OUTPUT_HEX_YC0 (EVE_VSIZE - FONT_SIZE - 5)


#define INPUT_TEXT_OPTIONS EVE_OPT_RIGHTX
#define FONT 18
#define FONT_SIZE_OPTIONS 20

/*
 * --------------------------------------------------------
 *                                                  [input]
 *
 * --------------------------------------------------------
 *                                          [Binary output]
 * --------------------------------------------------------
 *              [Hex output] |             [Decimal output]
 * */
// Outline frame definitions
// Top line which parts the options from the input
#define OUTLINE_WIDTH 2*16
#define TOP_OUTLINE_X0 0
#define TOP_OUTLINE_X1 EVE_HSIZE
#define TOP_OUTLINE_Y (FONT_SIZE_OPTIONS+2)
// Middle horizontal line that parts the input from results
#define MID_TOP_OUTLINE_X0 0
#define MID_TOP_OUTLINE_X1 EVE_HSIZE
#define MID_TOP_OUTLINE_Y (EVE_VSIZE/2)
// Lower middle horizontal line that parts binary output from hex and dec
#define MID_LOW_OUTLINE_X0 0
#define MID_LOW_OUTLINE_X1 EVE_HSIZE
#define MID_LOW_OUTLINE_Y (EVE_VSIZE*3/4)
// Vertical line parting the hex and dec results
#define VERT_LOW_OUTLINE_X (EVE_HSIZE/2)
#define VERT_LOW_OUTLINE_Y0 (EVE_VSIZE*3/4)
#define VERT_LOW_OUTLINE_Y1 (EVE_VSIZE)

void displayOutline(void){
    // Write the top line. This parts the options from the input
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X0*16,TOP_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X1*16,TOP_OUTLINE_Y*16));
    // Write middle line parting input and binary
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X0*16,MID_TOP_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X1*16,MID_TOP_OUTLINE_Y*16));
    // Lower middle outline parting binary and hex/dec
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X0*16,MID_LOW_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X1*16,MID_LOW_OUTLINE_Y*16));
    // Vertical line parting hex and dec results
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X*16,VERT_LOW_OUTLINE_Y0*16));
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X*16,VERT_LOW_OUTLINE_Y1*16));
}

// Function to print a number as binary, since
// there is no printf function for this in C
void printToBinary(char* pBuf, uint32_t num){
    for(uint8_t i = 0 ; i < 32 ; i++){
        pBuf[i] = ((num>>i)&0x01) + '0';
    }
}

void displayUartOnScreenTask(void *p){
    char pRxBuf[MAX_CALC_CORE_INPUT_LEN] = {0};
    char pBinRes[MAX_CALC_CORE_INPUT_LEN] = {0};
    char pHexRes[MAX_CALC_CORE_INPUT_LEN] = {0};
    char pDecRes[MAX_CALC_CORE_INPUT_LEN] = {0};
    uint32_t calcResult = 0;
    // Write the outlines:
    EVE_start_cmd_burst();
    EVE_cmd_dl_burst(CMD_DLSTART);
    EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | 0);
    EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    EVE_color_rgb_burst(0xFFFFFF);
    displayOutline();
    EVE_cmd_dl_burst(DL_DISPLAY);
    EVE_cmd_dl_burst(CMD_SWAP);
    EVE_end_cmd_burst();
    while (EVE_busy());

    while(1){
        // Check if data is available
        bool updateScreen = false;
        if(calcCoreInputTransformedQueue != 0){
            if(xQueueReceive(calcCoreInputTransformedQueue, pRxBuf, (TickType_t)5)){
                updateScreen = true;
            }
        }
        if(calcCoreOutputQueue != 0){
            if(xQueueReceive(calcCoreOutputQueue, &calcResult, (TickType_t)5)){
                // The result is now in calcResult, print to the different types
                sprintf(pDecRes, "%1i", calcResult);
                printToBinary(pBinRes, calcResult);
                sprintf(pHexRes, "0x%1X", calcResult);
                updateScreen = true;
            }
        }
        if(updateScreen){
            // Update the screen:
            EVE_start_cmd_burst();
            EVE_cmd_dl_burst(CMD_DLSTART);
            EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | 0);
            EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
            EVE_color_rgb_burst(0xFFFFFF);
            displayOutline();
            // Write the input text
            EVE_cmd_text_burst(INPUT_TEXT_XC0, INPUT_TEXT_YC0, FONT, INPUT_TEXT_OPTIONS, pRxBuf);
            // Write the results
            EVE_cmd_text_burst(OUTPUT_DEC_XC0, OUTPUT_DEC_YC0, FONT, INPUT_TEXT_OPTIONS, pDecRes);
            EVE_cmd_text_burst(OUTPUT_BIN_XC0, OUTPUT_BIN_YC0, FONT, INPUT_TEXT_OPTIONS, pBinRes);
            EVE_cmd_text_burst(OUTPUT_HEX_XC0, OUTPUT_HEX_YC0, FONT, INPUT_TEXT_OPTIONS, pHexRes);
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
    //UARTprintf("\n\nWelcome to the EK-TM4C123GXL FreeRTOS Demo!\n");

    // Create a mutex to guard the UART.
    g_pUARTSemaphore = xSemaphoreCreateMutex();

    // Create the queue used by the display and UART
    // Ten elements of chars. Should be enough to handle basic input
    uartReceiveQueue = xQueueCreate(10, sizeof(char));

    // Create queue for input buffer between calculator core and display task
    calcCoreInputTransformedQueue = xQueueCreate(2, sizeof(char)*MAX_CALC_CORE_INPUT_LEN);

    // Create queue for output between calculator core and display task
    calcCoreOutputQueue = xQueueCreate(2, sizeof(uint32_t));

    BaseType_t xReturned;
    TaskHandle_t screenTaskHandle = NULL;
    TaskHandle_t calcCoreTaskHandle = NULL;

    // Create the task, storing the handle.
    xTaskCreate(
            displayUartOnScreenTask, // Function that implements the task.
            "DISPLAY",               // Text name for the task.
            500,                    // Stack size in words, not bytes.
            ( void * ) 1,            // Parameter passed into the task.
            tskIDLE_PRIORITY,        // Priority at which the task is created.
            &screenTaskHandle );              // Used to pass out the created task's handle.
    xTaskCreate(
            calcCoreTask, // Function that implements the task.
            "CALCCORE",               // Text name for the task.
            500,                    // Stack size in words, not bytes.
            ( void * ) 1,            // Parameter passed into the task.
            tskIDLE_PRIORITY,        // Priority at which the task is created.
            &calcCoreTaskHandle );              // Used to pass out the created task's handle.



    //UARTprintf("\n\nInit task complete! Write in console to write something to the screen :) \n");
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
