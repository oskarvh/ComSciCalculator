/*
MIT License

Copyright (c) 2024 Oskar von Heideken

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
 * This file includes function mappings used by firmware_common, 
 * which are specific to the RP2040 comSciCalc rev 1 board.
 */

// Standard library
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Header
#include "rp2040_utils.h"

// Hardware specific
#include "pin_map.h"
#include "hardware/uart.h"

// RP2040:
#include "pico/printf.h"
#include "pico/stdio.h"
//#include "pico/stdio_uart.h"
#include "pico/stdio_usb.h"
#include "pico/stdio/driver.h"
//#include "pico/stdio_usb.h"

// comscicalc
#include "uart_logger.h"

// FW common, used for callback linking.
#include "firmware_common.h"

// FreeRTOS:
#include "FreeRTOS.h"
#include "queue.h"

// Globally defined timer handlers.
repeating_timer_t rtA;
repeating_timer_t rtB;

// STDIO event and defined
EventGroupHandle_t usbReadEvent;
#define USB_NEW_DATA_IN 1 << 0

// Wrapped timer callback
bool Timer1HzIntHandlerWrapper(repeating_timer_t *rt){
    Timer1HzIntHandler();
    return true; // Keep the timer going
}

// Wrapped timer callback
bool Timer60HzIntHandlerWrapper(repeating_timer_t *rt){
    Timer60HzIntHandler();
    return true; // Keep the timer going
}

void usbReadTask(void *p) {

    // Read the buffer until it's empty:
    while(getchar_timeout_us(10) != PICO_ERROR_TIMEOUT);

    // Loop forever
    while (1) {
        // Pend on the uartReadEvent
        uint32_t eventbits =
            xEventGroupWaitBits(usbReadEvent, USB_NEW_DATA_IN, pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventbits & USB_NEW_DATA_IN) {
            // read the buffer and add to the queue
            int rxChar = PICO_ERROR_TIMEOUT;
            do{
                rxChar = getchar_timeout_us(100); // Read the input character
            
                // Put it into the uartReceiveQueue
                if (rxChar != 255 && rxChar != 27 && rxChar != PICO_ERROR_TIMEOUT) {
                    // hooray, there is a character in the rx buffer
                    // which is now read!
                    // Push that to the queue.
                    // But first, convert it to an 8 bit char
                    char c = (char)rxChar;
                    if(c == '\b'){
                        // Hack: If a backspace is sent, then replace it with 127
                        // which is backspace here.
                        c = 127;
                    }
                    if (!xQueueSendToBack(uartReceiveQueue, (void *)&c,(TickType_t)0)) {
                        while (1);
                    }
                }
            } while(rxChar != PICO_ERROR_TIMEOUT);
        }
    }
}

bool stdio_callback(void){
    // RP2040 cannot read the char in the 
    // callback, which is a lazy implementation, but fair enough
    // To work around this, set an event to read the USB input
    // such that it's done outside of IRQ context
    xEventGroupSetBits(usbReadEvent, USB_NEW_DATA_IN);
}

bool mcuInit(void){
    stdio_init_all();
#if defined(DEBUG)
    // If the logger level is higher than error, then wait until
    // a valid USB connection is made. 
    // TODO: Disable this in case just power is provided. 
    //while(!stdio_usb_connected());
    sleep_ms(2000);
#endif
    return true;
}

bool initUart(void){
    uart_init(UART0, UART0_BR);
    gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    
    // Create the event group
    usbReadEvent = xEventGroupCreate();

    // Start the UART read task:
    TaskHandle_t usbReadTaskHandle = NULL;
    xTaskCreate(
        usbReadTask, // Function that implements the task.
        "USB_READ_TASK",               // Text name for the task.
        300,                    // Stack size in words, not bytes.
        ( void * ) 1,            // Parameter passed into the task.
        tskIDLE_PRIORITY + 1,        // Priority at which the task is created.
        &usbReadTaskHandle // Used to pass out the created task's handle.
    );

    // Set the USB callback:
    stdio_set_chars_available_callback((void*)stdio_callback, NULL);
}

bool initSpi(void){
    // Note: This the is the SPI for the offboard functions.
}

bool initTimer(void) {
    // Init one 1 Hz timer and one 60 Hz timer with callbacks
    // For the 1 Hz timer, Timer1HzIntHandler should be called
    // and for the 60 Hz timer, Timer60HzIntHandler should be called. 
    // These functions are found in FW common. 
    add_repeating_timer_ms(1000, Timer1HzIntHandlerWrapper, NULL, &rtA);
    add_repeating_timer_us(16666, Timer60HzIntHandlerWrapper, NULL, &rtB);
}

void startTimer(void) {
}

void out_char_driver(char c, void *arg) {
    ((stdio_driver_t *)arg)->out_chars(&c, 1);
}

int pprintf(const stdio_driver_t *driver, const char *format, ...) {
  va_list va;
  va_start(va, format);
  int ret = vfctprintf(out_char_driver, (void *)driver, format, va);
  va_end(va);

  return ret;
}


void UARTvprintf(const char *pcString, va_list vaArgP) {
    void *pDriver = &stdio_usb;
    //printf("IN LOGGER");

    vfctprintf(out_char_driver, pDriver, pcString, vaArgP);
    
    // int stringLen = vsnprintf(NULL, 0, pcString, vaArgP);
    // if (stringLen > 0) {
    //     char *strBuf = malloc(stringLen * sizeof(char));
    //     if (strBuf != NULL) {
    //         vsprintf(strBuf, pcString, vaArgP);
    //         usb_puts(UART0, strBuf);
    //     }
    //     free(strBuf);
    // }
}