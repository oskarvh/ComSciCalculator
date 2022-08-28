/*
 * comSciCalc.h
 *
 *  Created on: 28 aug. 2022
 *      Author: oskar
 */

/*
 * Brief:
 * This is the header file for all function related to the modern computer science calculator.
 *
 * */
#ifndef COMSCICALC_H_
#define COMSCICALC_H_

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

// TI GRLIB
#include <grlib/grlib.h>

#define PWM_PERIOD 255

// Global variables
PWM_Handle backLightPwmHandle;
SPI_Handle spiHandle;
tDisplay display;
tDisplayData displayData;
tContext grlibContext;

// Mailboxes
// Mailbox between UART and screen threads
Mailbox_Handle uartMailBoxHandle;

// Function prototypes:
void setBacklight(PWM_Handle pwm0, uint8_t duty);
void taskFxn(UArg arg0, UArg arg1);
void uartFxn(UArg arg0, UArg arg1);

#endif /* COMSCICALC_H_ */
