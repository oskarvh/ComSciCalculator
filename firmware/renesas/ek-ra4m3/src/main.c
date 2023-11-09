/***********************************************************************************************************************
 * Copyright [2020-2022] Renesas Electronics Corporation and/or its affiliates.
 *All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America
 *Inc. and may only be used with products of Renesas Electronics Corp. and its
 *affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely
 *responsible for the selection and use of Renesas products and Renesas assumes
 *no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable
 *laws, including copyright laws. Renesas reserves the right to change or
 *discontinue this software and/or this documentation. THE SOFTWARE AND
 *DOCUMENTATION IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO
 *REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT PERMISSIBLE UNDER
 *APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY,
 *INCLUDING WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND
 *NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR DOCUMENTATION.  RENESAS SHALL
 *HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE
 *MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN
 *CONNECTION WITH THE SOFTWARE OR DOCUMENTATION (OR ANY PERSON OR ENTITY
 *CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER,
 *INCLUDING, WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT,
 *PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS, OTHER ECONOMIC DAMAGE,
 *PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF
 *THE POSSIBILITY OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

#include <stdlib.h>

// Freertos:
#include "FreeRTOS.h"
#include "task.h"

// Comscicalc entry points
#include "firmware_common.h"

// Renesas PSF:
#include "hal_data.h"

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void __error__(char *pcFilename, uint32_t ui32Line) {}

#endif

//*****************************************************************************
//
// This hook is called by FreeRTOS when an stack overflow error is detected.
//
//*****************************************************************************
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    //
    // This function can not return, so loop forever.  Interrupts are disabled
    // on entry to this function, so no processor interrupts will interrupt
    // this loop.
    //
    // UARTprintf("\n\n======================WARNING======================\n");
    // UARTprintf("Task %s had a stack overflow :(", pcTaskName);
    // UARTprintf("\n\n===================================================\n");
    while (1) {
    }
}

/*******************************************************************************************************************/ /**
                                                                                                                       * This function is called at various points during the startup process.  This implementation uses the event that is
                                                                                                                       * called right before main() to set up the pins.
                                                                                                                       *
                                                                                                                       * @param[in]  event    Where at in the start up process the code is currently at
                                                                                                                       **********************************************************************************************************************/
void R_BSP_WarmStart(bsp_warm_start_event_t event) {
    if (BSP_WARM_START_RESET == event) {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery.
         * Placing the enable here, before clock and C runtime initialization,
         * should negate the need for a delay since the initialization will
         * typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event) {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    }
}

/**
 * @brief ComSciCalc main function
 * Only initializes a main thread, which later starts all other
 * threads. This is to ensure init is done in a threading context.
 */
int main(void) {
    // MCU specfic functions to be initialized out of thread context
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

    // The scheduler should never return. If this is the case,
    // we should reboot or do something.
    // For now though, just loop forever.
    while (1)
        ;
}