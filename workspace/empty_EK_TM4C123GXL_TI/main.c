/*
 * Copyright (c) 2015, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  PIN ASSIGNMENT
 *
 * PWM (backlight) : PB5
 * SPI MOSI : PA5
 * SPI MISO : PA4
 * SPI CLK : PA2
 * SPI CS (SPI hardware CS) : PA3
 * GPIO CS (Software controlled CS) : PA7
 * SPI DC (data/command pin for screen) : PA6
 *
 *
 */

/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdbool.h>
#include <stdio.h>
#include <xdc/runtime/Memory.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>


/* POSIX header files */
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

/* TI-RTOS Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>

/* TI driverlib functions */
#include <driverlib/sysctl.h> // Used for PWM
#include <driverlib/gpio.h>    // used for PWM
#include <driverlib/pin_map.h> // used for PWM
#include <inc/hw_memmap.h>     // Used for PWM

/* Board Header file */
#include "Board.h"

// Display driver:
#include "ADAFRUIT_2050.h"

// Computer scientist calculator tool header file
#include "comSciCalc.h"
// TI GRLIB
#include <grlib/grlib.h>


// Task related items
#define MAINTASKSTACKSIZE   2048
#define INITTASKSTACKSIZE   800 // Try to keep this as small as possible.
#define UARTTASKSTACKSIZE   2048
Task_Struct initTaskStruct; // Init task
Task_Struct mainTaskStruct; // Main task
Task_Struct uartTaskStruct; // UART task
char initTaskStack[INITTASKSTACKSIZE];


// This function opens the SPI, and returns the handle.
void initSpi(SPI_Handle *masterSpi){
    SPI_Params spiParams;
    SPI_Params_init(&spiParams);
    spiParams.bitRate = 20000000;
    spiParams.mode = SPI_MASTER;
    // Initialize SPI handle as default master
    *masterSpi = SPI_open(Board_SPI0, &spiParams);
    if (*masterSpi == NULL) {
        System_abort("Error initializing SPI\n");
    }
    else {
        System_printf("SPI initialized\n");
    }
}

// Function to start PWM module, muxed to pin PB5
void initPWM(PWM_Handle *pPwmHandle){
    // This is done instead of Board_initPWM:
    // Enable PWM peripherals
    // NOTE: Need to change the PWM_config table in the board file EK_TM4C123GXL.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    // Enable PWM on PA7
    GPIOPinConfigure(GPIO_PB5_M0PWM3);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_5);
    // Init the PWM module
    PWM_init();

    PWM_Params params;
    PWM_Params_init(&params);
    params.period = PWM_PERIOD;// Period and duty in microseconds
    params.polarity = PWM_POL_ACTIVE_HIGH; // Set active low as we pull the backlight down.
    *pPwmHandle = PWM_open(Board_PWM3, &params);
    if (*pPwmHandle == NULL) {
        System_abort("Board_PWM0 did not open");
    }
}



// Function to initialize hardware and software packages
// Also starts the rest of the tasks dynamically
Task_Params uartTaskParams;
Task_Params mainTaskParams;
Task_Handle uartTaskHandle;
Task_Handle mainTaskHandle;
void initFxn(UArg arg0, UArg arg1){

    // Init SPI and PWM (needs to be set in a task)
    initPWM(&backLightPwmHandle);

    // Set the backlight strength:
    setBacklight(backLightPwmHandle, 0); // sets backlight to 0%

    // Driver the reset pin high to get out of reset.
    // NOTE: RESET MUST HAVE BEEN SET TO 0 IN MAIN TASK!
    GPIO_write(GPIO_SCREEN_RESET, 1);

    // Init the SPI driver, get the spiHandle
    initSpi(&spiHandle);

    // Sleep for a small amount of time in order for the display to boot:
    usleep(100000); // Sleep for 10 ms

    // Init the screen
    HX8357_init(spiHandle);

    // Put the SPI handle in the displayData in order to let GRLIB use SPI.
    displayData.spiHandle = spiHandle;
    // Populate the GRLIB tDisplay variable
    display.i32Size = 0; // The size of this structure
    display.pvDisplayData = &displayData; // A pointer to display driver-specific data.
    display.ui16Width = HX8357_TFTWIDTH; // 480 pixels wide
    display.ui16Height = HX8357_TFTHEIGHT; // 320 pixels high
    display.pfnPixelDraw = &PixelDraw; // A pointer to the function to draw a pixel on this display
    display.pfnPixelDrawMultiple = &PixelDrawMultiple; //A pointer to the function to draw multiple pixels on this display.
    display.pfnLineDrawV = &LineDrawV; // A pointer to the function to draw a vertical line on this display
    display.pfnLineDrawH = &LineDrawH; // A pointer to the function to draw a horizontal line on this display.
    display.pfnRectFill = &RectFill; // A pointer to the function to draw a filled rectangle on this display
    display.pfnColorTranslate = &ColorTranslate; // A pointer to the function to translate 24-bit RGB colors to display-specific colors
    display.pfnFlush = &Flush; // A pointer to the function to flush any cached drawing operations on this display.

    // Initialize GRLIB:
    GrContextInit(&grlibContext, &display);
    GrContextForegroundSet(&grlibContext, ClrWhite); // white foreground
    GrContextBackgroundSet(&grlibContext, ClrBlack); // black background
    GrContextFontSet(&grlibContext, &g_sFontCmtt38);
    GrStringCodepageSet(&grlibContext, CODEPAGE_ISO8859_1);

    tGrLibDefaults grlibDefaults;
    GrLibInit(&grlibDefaults);

    // Fill the entire screen with black:
    tRectangle rect;
    rect.i16XMin = 0;
    rect.i16XMax = HX8357_TFTWIDTH;
    rect.i16YMin = 0;
    rect.i16YMax = HX8357_TFTHEIGHT;
    RectFill(display.pvDisplayData, &rect, HX8357_BLACK);

    // Sleep for 10 ms
    usleep(10000);
    setBacklight(backLightPwmHandle, 100); // sets backlight to 100%

    // Initialize the input buffer list state to empty state
    listState.pListEnd = NULL;
    listState.pListEntry = NULL;
    listState.numEntries = 0;


    // Stacks do not need to be pre-allocated for dynamic stack creation.
    // NOTE: THE STACKS WILL BE ALLOCATED ON THE HEAP
    // IF ALLOCATED DYNAMICALLY! THEREFORE A LARGE HEAP IS NEEDED!
    // Construct the main task
    Task_Params_init(&mainTaskParams);
    mainTaskParams.stackSize = MAINTASKSTACKSIZE;

    mainTaskParams.priority = 2;
    mainTaskHandle = Task_create((Task_FuncPtr)displayFxn/*taskFxn*/, &mainTaskParams, NULL);
    if(mainTaskHandle == NULL){
        System_abort("Main task could not be created");
    }

    // Construct the UART task:
    Task_Params_init(&uartTaskParams);
    uartTaskParams.arg0 = 115200; // Baud rate
    uartTaskParams.stackSize = UARTTASKSTACKSIZE;
    uartTaskParams.priority = 3;
    uartTaskHandle = Task_create((Task_FuncPtr)uartFxn, &uartTaskParams, NULL);
    if(uartTaskHandle == NULL){
        System_abort("UART task could not be created");
    }
    // Exit this task.
    Task_exit();
}

// Main function
// Starts the necessary hardware HAL's,
// constructs init task, and start BIOS.
int main(void)
{
    Task_Params initTaskParams;
    Mailbox_Params mailboxParams;
    Event_Params eventParams;
    /* Call board init functions */
    Board_initGeneral();
    Board_initGPIO();
    // Drive the reset pin low to reset
    GPIO_write(GPIO_SCREEN_RESET, 0);


    // Board_initI2C();
    // Board_initSDSPI();
    Board_initSPI();


    Board_initUART();
    // Board_initUSB(Board_USBDEVICE);
    // Board_initWatchdog();
    // Board_initWiFi();
    // Board_initPWM();

    // Construct the init task
    Task_Params_init(&initTaskParams);
    initTaskParams.stackSize = INITTASKSTACKSIZE;
    initTaskParams.stack = &initTaskStack;
    initTaskParams.priority = 1;
    Task_construct(&initTaskStruct, (Task_FuncPtr)initFxn, &initTaskParams, NULL);

    // Construct the event module to wake up the display task.
    Event_Params_init(&eventParams);
    // Events can either be user input, or timer module
    wakeDisplayEventHandle = Event_create(NULL, NULL);
    if (wakeDisplayEventHandle == NULL) {
        System_abort("Event create failed");
    }

    // Construct mailbox between UART thread and screen thread
    Mailbox_Params_init(&mailboxParams);
    mailboxParams.readerEvent = wakeDisplayEventHandle;
    mailboxParams.readerEventId = EVENT_USER_INPUT;
    // Create a mailbox of size 1 byte, 1 buffer.
    uartMailBoxHandle =  Mailbox_create(1, 1, &mailboxParams, NULL);
    if (uartMailBoxHandle == NULL) {
        System_abort("Mailbox create failed");
    }


    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
