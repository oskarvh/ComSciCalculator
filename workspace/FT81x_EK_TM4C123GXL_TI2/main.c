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
 * PDN pin: PB0
 * SPI MOSI : PA5
 * SPI MISO : PA4
 * SPI CLK : PA2
 * SPI CS (SPI hardware CS) : PA3
 * GPIO CS (Software controlled CS) : PA6
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

// Include RudolphRieder's FT81x driver:
#include "EVE.h"
//#include "EVE_commands.h"
//#include "FT800-FT813/EVE_config.h"
//#include "FT800-FT813/EVE_target.h"

// Include the comSciCalc core
#include "comscicalc.h"


// Task related items
#define MAINTASKSTACKSIZE   2048
#define INITTASKSTACKSIZE   2048 // Try to keep this as small as possible.
#define UARTTASKSTACKSIZE   2048
Task_Struct initTaskStruct; // Init task
Task_Struct mainTaskStruct; // Main task
Task_Struct uartTaskStruct; // UART task
char initTaskStack[INITTASKSTACKSIZE];

void initFxn(UArg arg0, UArg arg1){
    calc_coreInit(NULL);
    EVE_SPI_Init();
    EVE_init();

    EVE_start_cmd_burst();
    EVE_cmd_dl_burst(CMD_DLSTART);
    EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | 0);
    EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
    EVE_color_rgb_burst(0xFFFFFF);
    EVE_cmd_text_burst(5, 15, 28, 0, "Hello there!");
    EVE_cmd_dl_burst(DL_DISPLAY);
    EVE_cmd_dl_burst(CMD_SWAP);
    EVE_end_cmd_burst();
    while (EVE_busy());

    while(1);
}
// Main function
// Starts the necessary HAL's,
// constructs init task, and start BIOS.
int main(void)
{
    // Call board init functions
    Board_initGeneral();
    Board_initGPIO();
    Board_initSPI();
    Board_initUART();


    Task_Params initTaskParams;
    Task_Params_init(&initTaskParams);
    initTaskParams.stackSize = INITTASKSTACKSIZE;
    initTaskParams.stack = &initTaskStack;
    initTaskParams.priority = 1;
    Task_construct(&initTaskStruct, (Task_FuncPtr)initFxn, &initTaskParams, NULL);


    System_printf("Starting the example\nSystem provider is set to SysMin. "
                  "Halt the target to view any SysMin contents in ROV.\n");
    /* SysMin will only print to the console when you call flush or exit */
    System_flush();

    /* Start BIOS */
    BIOS_start();

    return (0);
}
