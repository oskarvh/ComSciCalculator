/***********************************************************************************************************************
 * Copyright [2020-2022] Renesas Electronics Corporation and/or its affiliates.  All Rights Reserved.
 *
 * This software and documentation are supplied by Renesas Electronics America Inc. and may only be used with products
 * of Renesas Electronics Corp. and its affiliates ("Renesas").  No other uses are authorized.  Renesas products are
 * sold pursuant to Renesas terms and conditions of sale.  Purchasers are solely responsible for the selection and use
 * of Renesas products and Renesas assumes no liability.  No license, express or implied, to any intellectual property
 * right is granted by Renesas. This software is protected under all applicable laws, including copyright laws. Renesas
 * reserves the right to change or discontinue this software and/or this documentation. THE SOFTWARE AND DOCUMENTATION
 * IS DELIVERED TO YOU "AS IS," AND RENESAS MAKES NO REPRESENTATIONS OR WARRANTIES, AND TO THE FULLEST EXTENT
 * PERMISSIBLE UNDER APPLICABLE LAW, DISCLAIMS ALL WARRANTIES, WHETHER EXPLICITLY OR IMPLICITLY, INCLUDING WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT, WITH RESPECT TO THE SOFTWARE OR
 * DOCUMENTATION.  RENESAS SHALL HAVE NO LIABILITY ARISING OUT OF ANY SECURITY VULNERABILITY OR BREACH.  TO THE MAXIMUM
 * EXTENT PERMITTED BY LAW, IN NO EVENT WILL RENESAS BE LIABLE TO YOU IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION
 * (OR ANY PERSON OR ENTITY CLAIMING RIGHTS DERIVED FROM YOU) FOR ANY LOSS, DAMAGES, OR CLAIMS WHATSOEVER, INCLUDING,
 * WITHOUT LIMITATION, ANY DIRECT, CONSEQUENTIAL, SPECIAL, INDIRECT, PUNITIVE, OR INCIDENTAL DAMAGES; ANY LOST PROFITS,
 * OTHER ECONOMIC DAMAGE, PROPERTY DAMAGE, OR PERSONAL INJURY; AND EVEN IF RENESAS HAS BEEN ADVISED OF THE POSSIBILITY
 * OF SUCH LOSS, DAMAGES, CLAIMS OR COSTS.
 **********************************************************************************************************************/

#include "hal_data.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h" 

#include "comscicalc.h"

#include "EVE.h"
#include "display.h"
#include "uart_logger.h"

//FSP specific includes:
#include <stdint.h>
#include "r_sci_spi.h"
#include "r_sci_uart.h"


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
    //UARTprintf("\n\n======================WARNING======================\n");
    //UARTprintf("Task %s had a stack overflow :(", pcTaskName);
    //UARTprintf("\n\n===================================================\n");
    while(1)
    {

    }
}

/*-------------------------MCU SPECIFIC DRIVERS-------------------------------*/
// TODO
void sci_spi_callback(spi_callback_args_t *p_args){

}
char uartRxChar = 0; 
void uartRxIntHandler(uart_callback_args_t *p_args){
    

    // Read the FIFO and put in a queue.
    uartRxChar = 0; 

    if(UART_EVENT_RX_CHAR == p_args->event)
    {
        uartRxChar = (uint8_t ) p_args->data;
    
        
        // Handle escape char sequence.
        // Ideally, this should be handled by something else than the ISR,
        // since we don't want to wait in the ISR.yy
        if(uartRxChar != 255 && uartRxChar != 27){
            // hooray, there is a character in the rx buffer
            // which is now read!
            // Push that to the queue.
            if(!xQueueSendToBackFromISR (uartReceiveQueue, (void*)&uartRxChar, (TickType_t)0)){
                while(1);
            }
        }
    }
}
void Timer0BIntHandler(void){

}
void Timer0AIntHandler(void){

}

void ConfigureUART(void){
    // Initialize UART channel with baud rate 115200 
    fsp_err_t err = R_SCI_UART_Open (&g_uart0_ctrl, &g_uart0_cfg);
    if (FSP_SUCCESS != err)
    {
        while(1);
    }
}

void initTimer(){

}

void initDisplay(void){
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
                        if(receiveChar == 'm' || receiveChar == 'M'){
                            // Update the input format (int, float, fixed)
                            uint8_t inputFormat = calcState.numberFormat.inputFormat;
                            inputFormat += 1;
                            if(inputFormat >= INPUT_FMT_RESERVED){
                                inputFormat = 0;
                            }
                            calc_updateInputFormat(&calcState, inputFormat);
                        }
                        if(receiveChar == 'o' || receiveChar == 'O'){
                            // Update the output format (int, float, fixed)
                            uint8_t outputFormat = calcState.numberFormat.outputFormat;
                            outputFormat += 1;
                            if(outputFormat >= INPUT_FMT_RESERVED){
                                outputFormat = 0;
                            }
                            calc_updateOutputFormat(&calcState, outputFormat);
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
                if(displayState.printStatus == calc_funStatus_INPUT_LIST_NULL){
                    // If there is no result due to the input list being 0,
                    // that means that there wasn't any input chars. So set the
                    // result to 0
                    displayState.result = 0;
                }
                displayState.cursorLoc = calc_getCursorLocation(&calcState);
                memcpy(&(displayState.inputOptions), &(calcState.numberFormat), sizeof(numberFormat_t));
                // Give the semaphore back
                xSemaphoreGive(displayStateSemaphore);

            }
            xEventGroupSetBits(displayTriggerEvent, DISPLAY_EVENT_NEW_DATA);
        }
    }
}

int
main(void)
{
    // Set the clocking to run directly from the external crystal/oscillator.
    // NOTE: The SYSCTL_XTAL_ value must be changed to match the value of the
    // crystal on your board.
#if defined(TARGET_IS_EK_RA4M3) 
// TODO: Init the MCU here! 

#endif
    //vPortDefineHeapRegions();
    // Create the queue used by the display and UART
    // 100 elements of chars is way more than needed for
    // human input, but for HIL test execution, this might be
    // necessary
    uartReceiveQueue = xQueueCreate(100, sizeof(char));
    if( uartReceiveQueue == NULL )
    {
        while(1);
    }
    // Initialize the UART and configure it for 115,200, 8-N-1 operation.
    ConfigureUART();

    //R_SCI_UART_Write(&g_uart0_ctrl, "test/r/n", 6);
    logger("UART init'd\r\n"); 

    //initDisplay();
    //initDisplayState(&displayState);

    // Create the binary semaphore to protect the display state
    displayStateSemaphore = xSemaphoreCreateBinary();
    if( displayStateSemaphore == NULL )
    {
        while(1);
    }

    // Create the synchronization event between the calculator task
    // and the display task
    displayTriggerEvent = xEventGroupCreate();

    

    TaskHandle_t screenTaskHandle = NULL;
    TaskHandle_t calcCoreTaskHandle = NULL;
    // Create the task, storing the handle.
    /*
    xTaskCreate(
            displayTask, // Function that implements the task.
            "DISPLAY",               // Text name for the task.
            700,                    // Stack size in words, not bytes.
            ( void * ) 1,            // Parameter passed into the task.
            tskIDLE_PRIORITY,        // Priority at which the task is created.
            &screenTaskHandle );              // Used to pass out the created task's handle.
            
    */
    xTaskCreate(
            calcCoreTask, // Function that implements the task.
            "CALCCORE",               // Text name for the task.
            2000,                    // Stack size in words, not bytes.
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

/*-------------------------EXAMPLE CODE-------------------------------*/


void R_BSP_WarmStart(bsp_warm_start_event_t event);

extern bsp_leds_t g_bsp_leds;

/*******************************************************************************************************************//**
 * @brief  Blinky example application
 *
 * Blinks all leds at a rate of 1 second using the software delay function provided by the BSP.
 *
 **********************************************************************************************************************/
int old_main(void)
{
#if BSP_TZ_SECURE_BUILD

    /* Enter non-secure code */
    R_BSP_NonSecureEnter();
#endif

    /* Define the units to be used with the software delay function */
    const bsp_delay_units_t bsp_delay_units = BSP_DELAY_UNITS_MILLISECONDS;

    /* Set the blink frequency (must be <= bsp_delay_units */
    const uint32_t freq_in_hz = 2;

    /* Calculate the delay in terms of bsp_delay_units */
    const uint32_t delay = bsp_delay_units / freq_in_hz;

    /* LED type structure */
    bsp_leds_t leds = g_bsp_leds;

    /* If this board has no LEDs then trap here */
    if (0 == leds.led_count)
    {
        while (1)
        {
            ;                          // There are no LEDs on this board
        }
    }

    /* Holds level to set for pins */
    bsp_io_level_t pin_level = BSP_IO_LEVEL_LOW;

    while (1)
    {
        /* Enable access to the PFS registers. If using r_ioport module then register protection is automatically
         * handled. This code uses BSP IO functions to show how it is used.
         */
        R_BSP_PinAccessEnable();

        /* Update all board LEDs */
        for (uint32_t i = 0; i < leds.led_count; i++)
        {
            /* Get pin to toggle */
            uint32_t pin = leds.p_leds[i];

            /* Write to this pin */
            R_BSP_PinWrite((bsp_io_port_pin_t) pin, pin_level);
        }

        /* Protect PFS registers */
        R_BSP_PinAccessDisable();

        /* Toggle level for next write */
        if (BSP_IO_LEVEL_LOW == pin_level)
        {
            pin_level = BSP_IO_LEVEL_HIGH;
        }
        else
        {
            pin_level = BSP_IO_LEVEL_LOW;
        }

        /* Delay */
        R_BSP_SoftwareDelay(delay, bsp_delay_units);
    }
}

/*******************************************************************************************************************//**
 * This function is called at various points during the startup process.  This implementation uses the event that is
 * called right before main() to set up the pins.
 *
 * @param[in]  event    Where at in the start up process the code is currently at
 **********************************************************************************************************************/
void R_BSP_WarmStart (bsp_warm_start_event_t event)
{
    if (BSP_WARM_START_RESET == event)
    {
#if BSP_FEATURE_FLASH_LP_VERSION != 0

        /* Enable reading from data flash. */
        R_FACI_LP->DFLCTL = 1U;

        /* Would normally have to wait tDSTOP(6us) for data flash recovery. Placing the enable here, before clock and
         * C runtime initialization, should negate the need for a delay since the initialization will typically take more than 6us. */
#endif
    }

    if (BSP_WARM_START_POST_C == event)
    {
        /* C runtime environment and system clocks are setup. */

        /* Configure pins. */
        R_IOPORT_Open(&g_ioport_ctrl, g_ioport.p_cfg);
    }
}
