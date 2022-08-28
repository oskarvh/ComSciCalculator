/*
 * comSciCalc.c
 *
 *  Created on: 28 aug. 2022
 *      Author: oskar
 */

/*
 * Brief:
 * This is the source file for all function related to the modern computer science calculator.
 *
 * */

// Include this files header file
#include <comSciCalc.h>



// Function to set the TFT display backlight
// Input is in percent, 0 is all off, 100 is full light.
void setBacklight(PWM_Handle pwm0, uint8_t duty){
    if(duty > 100){
        duty = 100;
    }
    PWM_setDuty(pwm0, (PWM_PERIOD*duty)/100);
}

// Pin assignment:
// PWM (backlight) : PB5
// SPI MOSI : PA5
// SPI MISO : PA4
// SPI CLK : PA2
// SPI CS (SPI hardware CS) : PA3
// GPIO CS (Software controlled CS) : PA7
// SPI DC (data/command pin for screen) : PA6

void taskFxn(UArg arg0, UArg arg1)
{
    char uartInputBuf;
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


void uartFxn(UArg arg0, UArg arg1)
{
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
