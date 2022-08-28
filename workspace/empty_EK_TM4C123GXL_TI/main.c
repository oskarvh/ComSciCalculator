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
 *  ======== empty.c ========
 */
/* XDCtools Header files */
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdbool.h>
#include <stdio.h>

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
#include <driverlib/pin_map.h> // used for PWm
#include <inc/hw_memmap.h>     // Used for PWM

/* Board Header file */
#include "Board.h"

// Display driver:
#include "ADAFRUIT_2050.h"

// TI GRLIB
#include <grlib/grlib.h>

// Task related items
#define MAINTASKSTACKSIZE   4096
#define INITTASKSTACKSIZE   8192
#define CONSTRUCTTASKSTACKSIZE   2048
#define UARTTASKSTACKSIZE   2048
Task_Struct initTaskStruct; // Init task
Task_Struct constructTaskStruct; // Init task
Task_Struct mainTaskStruct; // Main task
Task_Struct uartTaskStruct; // UART task
Char initTaskStack[INITTASKSTACKSIZE];
Char mainTaskStack[MAINTASKSTACKSIZE];
Char uartTaskStack[UARTTASKSTACKSIZE];
Char constructTaskStack[CONSTRUCTTASKSTACKSIZE];

// Semaphores
// Semaphores for letting the UART task know that there is data waiting
Semaphore_Handle uartReadSemHandle;
// Semaphore to let other tasks wait for init done.
Semaphore_Handle initDoneSemHandle;

// Mailboxes
// Mailbox between UART and screen threads
Mailbox_Handle uartMailBoxHandle;

#define PWM_PERIOD 255
// Initialize the PWM module. Pin is PB5
PWM_Handle backLightPwmHandle;
SPI_Handle spiHandle;
tDisplay display;
tDisplayData displayData;
tContext grlibContext;


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
// function to set the PWM module.
// Input is in percent
void setBacklight(PWM_Handle pwm0, uint8_t duty){
    if(duty > 100){
        duty = 100;
    }
    PWM_setDuty(pwm0, (PWM_PERIOD*duty)/100);
}


// This function opens the SPI, and returns the handle.
// PA2 is CLK, PA3 is CS, PA4 is MISO, PA5 is MOSI
SPI_Handle initSpi(void){
    SPI_Handle masterSpi;
    SPI_Params spiParams;
    SPI_Params_init(&spiParams);
    spiParams.bitRate = 20000000;
    spiParams.mode = SPI_MASTER;
    // Initialize SPI handle as default master
    masterSpi = SPI_open(Board_SPI0, &spiParams);
    if (masterSpi == NULL) {
        System_abort("Error initializing SPI\n");
        return NULL;
    }
    else {
        System_printf("SPI initialized\n");
    }
    return masterSpi;
}

// Function to transmit (and receive) as master.
// Inputs are SPI handle, pointers to rx and tx buffers, must be equal in size,
// and the length of those buffers
// Returns true if transmission was successful
// SPI must have been initialized and opened prior to calling this function.
bool masterSpiTransmit(SPI_Handle masterSpi, unsigned char* pTxBuf, unsigned char* pRxBuf, uint8_t bufLen){
    SPI_Transaction masterTransaction;
    masterTransaction.count = bufLen;
    masterTransaction.txBuf = (Ptr)pTxBuf;
    masterTransaction.rxBuf = (Ptr)pRxBuf;

    /* Initiate SPI transfer */
    return SPI_transfer(masterSpi, &masterTransaction);
}

/*
 *  ======== main ========
 */
// Pin assignment:
// PWM (backlight) : PB5
// SPI MOSI : PA5
// SPI MISO : PA4
// SPI CLK : PA2
// SPI CS (SPI hardware CS) : PA3
// GPIO CS (Software controlled CS) : PA7
// SPI DC (data/command pin for screen) : PA6

Void taskFxn(UArg arg0, UArg arg1)
{
    Semaphore_pend(initDoneSemHandle, BIOS_WAIT_FOREVER);
    char uartInputBuf = 'd';
    uint16_t x,y;
    x = y = 0;
    uint16_t charCounter = 0; // Character counter

#define Y_INCREASE 40
#define X_INCREASE 20
    while(1){
        Mailbox_pend(uartMailBoxHandle, &uartInputBuf, BIOS_WAIT_FOREVER);
        if(uartInputBuf != 0x7F){
            GrStringDraw(&grlibContext, &uartInputBuf, 1, x, y, false);
            charCounter++;
            x += X_INCREASE;
            if(x >= 480-X_INCREASE*2){
                y += Y_INCREASE;
                x = 0;
            }
            if(y >= 320-Y_INCREASE-1){
                y = 0;
                x = 0;
            }
        }
        else{
            // Handle backspace
            // Calculate where the backspace should be done:
            if(charCounter > 0){
                tRectangle rect;
                if(x < X_INCREASE){
                    if(y < Y_INCREASE){
                        y = 320 - Y_INCREASE*2;
                    }
                    else{
                        y -= Y_INCREASE;
                    }

                    x = 480 - X_INCREASE*3;
                }
                else{
                    x -= X_INCREASE;
                }
                rect.i16XMin = x;
                rect.i16XMax = x + X_INCREASE;
                rect.i16YMin = y;
                rect.i16YMax = y + Y_INCREASE;
                RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
                charCounter--;
            }
        }
        tRectangle rect;
        rect.i16XMin = 0;
        rect.i16XMax = 4*X_INCREASE;
        rect.i16YMin = 200;
        rect.i16YMax = 200 + 2*Y_INCREASE;
        RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
        char numChars[3];
        sprintf(numChars, "%i", charCounter);
        GrStringDraw(&grlibContext, numChars, 3, 0, 200, false);

    }

}

char uartTmpBuf[50];
void uartFxn(UArg arg0, UArg arg1)
{
    Semaphore_pend(initDoneSemHandle, BIOS_WAIT_FOREVER);
    UART_Handle uart;
    UART_Params uartParams;
    uint16_t tmpCount = 0;
    /* Create a UART with data processing off. */
    UART_Params_init(&uartParams);
    uartParams.writeDataMode = UART_DATA_BINARY;
    uartParams.readDataMode = UART_DATA_BINARY;
    uartParams.readReturnMode = UART_RETURN_NEWLINE;
    uartParams.readEcho = UART_ECHO_OFF;
    uartParams.baudRate = arg0;
    uartParams.readMode = UART_MODE_BLOCKING;//UART_MODE_CALLBACK;
    uartParams.readCallback = NULL; //&uartReadCallback;
    uart = UART_open(Board_UART0, &uartParams);

    if (uart == NULL) {
        System_abort("Error opening the UART");
    }

    char readBuf;
    while(1){
        // Read one character at a time
        UART_read(uart, &readBuf, 1);
        if(readBuf != 0x7F){
            tmpCount++;
        }
        else {
            tmpCount--;
        }
        // Send the read character to the screen task in a mailbox event:
        Mailbox_post(uartMailBoxHandle, &readBuf, BIOS_WAIT_FOREVER);

    }

}


// Function to initialize hardware and software packages
void initFxn(UArg arg0, UArg arg1){

    // Init SPI and PWM (needs to be set in a task)
    initPWM(&backLightPwmHandle);

    // Set the backlight strength:
    setBacklight(backLightPwmHandle, 100); // sets backlight to 0%

    // Driver the reset pin high to get out of reset.
    // NOTE: RESET MUST HAVE BEEN SET TO 0 IN MAIN TASK!
    GPIO_write(GPIO_SCREEN_RESET, 1);

    // Init the SPI driver, get the spiHandle
    spiHandle = initSpi();

    // Sleep for a small amount of time in order for the display to boot:
    usleep(100000); // Sleep for 10 ms

    // Init the screen
    HX8357_init(spiHandle);

    // Put the SPI handle in the displayData in order to let GRLIB use SPI.
    displayData.spiHandle = spiHandle;
    // Populate the GRLIB tDisplay variable
    display.i32Size = 0; // The size of this structure
    display.pvDisplayData = &displayData; // A pointer to display driver-specific data.
    display.ui16Width = HX8357_TFTHEIGHT; // 480 pixels wide
    display.ui16Height = HX8357_TFTWIDTH; // 320 pixels high
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
    rect.i16XMax = HX8357_TFTHEIGHT;
    rect.i16YMin = 0;
    rect.i16YMax = HX8357_TFTWIDTH-100;
    RectFill(display.pvDisplayData, &rect, HX8357_BLACK);

    // Sleep for 10 ms
    usleep(10000);
    setBacklight(backLightPwmHandle, 100); // sets backlight to 100%

    // Post the semaphore to let other threads know that the init is done:
    Semaphore_post(initDoneSemHandle);
    Semaphore_post(initDoneSemHandle);

    // Exit the task.
    Task_exit();
}

// Test a construct task:
void constructFxn(UArg arg0, UArg arg1){
    Task_Params uartTaskParams;
    Task_Params mainTaskParams;

    // Wait on the init task to complete.
    Semaphore_pend(initDoneSemHandle, BIOS_WAIT_FOREVER);

    // Construct the UART task:
    Task_Params_init(&uartTaskParams);
    uartTaskParams.arg0 = 115200; // Baud rate
    uartTaskParams.stackSize = UARTTASKSTACKSIZE;
    uartTaskParams.stack = &uartTaskStack;
    uartTaskParams.priority = 3;
    Task_construct(&uartTaskStruct, (Task_FuncPtr)uartFxn, &uartTaskParams, NULL);

    // Construct the main task
    Task_Params_init(&mainTaskParams);
    mainTaskParams.arg0 = 10000;
    mainTaskParams.stackSize = MAINTASKSTACKSIZE;
    mainTaskParams.stack = &mainTaskStack;
    mainTaskParams.priority = 2;
    Task_construct(&mainTaskStruct, (Task_FuncPtr)taskFxn, &mainTaskParams, NULL);

    // Exit the task.
    Task_exit();
}

int main(void)
{
    Task_Params initTaskParams;
    Task_Params uartTaskParams;
    Task_Params mainTaskParams;
    Task_Params constructTaskParams;
    Semaphore_Params initDoneSemParams;
    Semaphore_Struct uartReadSemStruct;
    Semaphore_Struct initDoneSemStruct;
    Mailbox_Params mailboxParams;
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
/*
    // Constructor task:
    Task_Params_init(&constructTaskParams);
    constructTaskParams.stackSize = CONSTRUCTTASKSTACKSIZE;
    constructTaskParams.stack = &constructTaskStack;
    constructTaskParams.priority = 2;
    Task_construct(&constructTaskStruct, (Task_FuncPtr)constructFxn, &constructTaskParams, NULL);
*/

    // Construct the UART task:
    Task_Params_init(&uartTaskParams);
    uartTaskParams.arg0 = 115200; // Baud rate
    uartTaskParams.stackSize = UARTTASKSTACKSIZE;
    uartTaskParams.stack = &uartTaskStack;
    uartTaskParams.priority = 3;
    Task_construct(&uartTaskStruct, (Task_FuncPtr)uartFxn, &uartTaskParams, NULL);

    // Construct the main task
    Task_Params_init(&mainTaskParams);
    mainTaskParams.arg0 = 10000;
    mainTaskParams.stackSize = MAINTASKSTACKSIZE;
    mainTaskParams.stack = &mainTaskStack;
    mainTaskParams.priority = 2;
    Task_construct(&mainTaskStruct, (Task_FuncPtr)taskFxn, &mainTaskParams, NULL);

    // Construct semaphores
    Semaphore_Params_init(&initDoneSemParams);
    initDoneSemHandle = Semaphore_create(0, &initDoneSemParams, NULL);
    if(initDoneSemHandle == NULL){
        System_abort("Semaphore could not be created");
    }

    // Construct mailbox between UART thread and screen thread
    Mailbox_Params_init(&mailboxParams);
    // Create a mailbox of size 1 byte, 1 buffer.
    uartMailBoxHandle =  Mailbox_create(1, 1, &mailboxParams, NULL);

    //Semaphore_construct(&initDoneSemStruct, 0, &initDoneSemParams);
    //initDoneSemHandle = Semaphore_handle(&initDoneSemStruct);

/*
    // Construct the main task
    Task_Params_init(&mainTaskParams);
    mainTaskParams.arg0 = 10000;
    mainTaskParams.stackSize = MAINTASKSTACKSIZE;
    mainTaskParams.stack = &task0Stack;
    Task_construct(&task0Struct, (Task_FuncPtr)taskFxn, &mainTaskParams, NULL);

    // Construct the UART task:
    Task_Params_init(&uartTaskParams);
    uartTaskParams.arg0 = 115200; // Baud rate
    uartTaskParams.stackSize = UARTTASKSTACKSIZE;
    uartTaskParams.stack = &task1Stack;
    Task_construct(&task1Struct, (Task_FuncPtr)uartFxn, &uartTaskParams, NULL);

    // Construct semaphores
    Semaphore_Params_init(&uartSemParams);
    Semaphore_construct(&uartReadSemStruct, 1, &uartSemParams);
    uartReadSemHandle = Semaphore_handle(&uartReadSemStruct);


    // Construct mailbox between UART thread and screen thread
    Mailbox_Params_init(&mailboxParams);
    // Create a mailbox of size 1 byte, 1 buffer.
    uartMailBoxHandle =  Mailbox_create(1, 1, &mailboxParams, NULL);
*/
    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
