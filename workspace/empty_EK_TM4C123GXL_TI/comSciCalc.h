/*
 * comSciCalc.h
 *
 *  Created on: 28 aug. 2022
 *      Author: Oskar von Heideken
 */

/*
 * Brief:
 * This is the header file for all function related to the modern computer science calculator.
 *
 * */
#ifndef COMSCICALC_H_
#define COMSCICALC_H_

// XDCtools Header files
#include <xdc/std.h>
#include <xdc/runtime/System.h>
#include <stdbool.h>
#include <stdio.h>
#include <xdc/runtime/Memory.h>

// BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/knl/Mailbox.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Clock.h>


// POSIX header files
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

// TI-RTOS Header files
#include <ti/drivers/GPIO.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>

// TI driverlib functions
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>

// Board Header file
#include "Board.h"

// Display driver:
#include "ADAFRUIT_2050.h"

// TI GRLIB
#include <grlib/grlib.h>

// TI-RTOS related defines
#define EVENT_TIMER_MODULE Event_Id_00
#define EVENT_USER_INPUT   Event_Id_01

// General settings
#define PWM_PERIOD 255
#define MAX_INPUT_CHARS 100
#define CURSOR_PERIOD_MS 1000

// Screen layout defines:
// TEXT BOXES
#define PTR_TEXT_BOX_FONT &g_sFontCmtt22
#define TEXT_BOX_WIDTH 55
#define TEXT_BOX_Y_START 1
#define TEXT_BOX_HEIGHT (CUTLINE_Y-2)
#define TEXT_BOX_1_X_START 1
#define TEXT_BOX_2_X_START (TEXT_BOX_1_X_START + TEXT_BOX_WIDTH + 2)
#define TEXT_BOX_3_X_START (TEXT_BOX_2_X_START + TEXT_BOX_WIDTH + 2)
// CUTLINE
#define CUTLINE_Y 19
// BATTERY INDICATOR
#define BATTERY_BOX_RECT_Y_START 1
#define BATTERY_BOX_RECT_Y_END (CUTLINE_Y - 2)
#define BATTERY_IND_X_WIDTH 10
#define BATTERY_BOX_RECT_X_START (HX8357_TFTWIDTH-BATTERY_IND_X_WIDTH*3 - 10)
#define BATTERY_BOX_RECT_X_END (BATTERY_BOX_RECT_X_START + BATTERY_IND_X_WIDTH*3+4)
#define BATTERY_STUB_Y_START (BATTERY_BOX_RECT_Y_START + 4)
#define BATTERY_STUB_Y_END (BATTERY_BOX_RECT_Y_END - 4)
#define BATTERY_STUB_X_START (BATTERY_BOX_RECT_X_END)
#define BATTERY_STUB_X_END (BATTERY_BOX_RECT_X_END + 3)
#define BATTERY_IND_Y_START (BATTERY_BOX_RECT_Y_START + 1)
#define BATTERY_IND_Y_END (BATTERY_BOX_RECT_Y_END - 1)
#define BATTERY_IND_ALMOST_EMPTY 2
#define BATTERY_IND_1_X_START (BATTERY_BOX_RECT_X_START + 1)
#define BATTERY_IND_2_X_START (BATTERY_IND_1_X_START + BATTERY_IND_X_WIDTH + 1)
#define BATTERY_IND_3_X_START (BATTERY_IND_2_X_START + BATTERY_IND_X_WIDTH + 1)

// Defines for the different keys.
// In order to be compatible with UART, as it's the first iteration,
// these defines are chars.
#define TOGGLE_MENU_BUTTON 'm'
#define LEFT_ARROW 'l'
#define RIGHT_ARROW 'r'
#define UP_ARROW 'u'
#define DOWN_ARROW 'w'
#define BACKSPACE 127
// Linked list for storing the input.
// We want a doubly linked list, so that characters can be inserted in between
// other characters.
typedef struct listElement{
    // Pointer to next element. If NULL then no next element is defined.
    void *pNextElem;
    // Pointer to previous element. NULL if this is the first element.
    void *pPrevElem;
    // Char currently in this entry of the list
    char currentChar;
}listElement_t;

// structure that works to keep track of the state of the list
typedef struct listState {
    listElement_t *pListEntry;
    listElement_t *pListEnd;
    uint8_t numEntries;
}listState_t;

// Different statuses that the linked list helper functions can return
enum listStatus {
    ALLOCATION_FAILED = -1, // new list element allocation failed
    LIST_END_ERROR = -2, // There is an error at the end of the list, e.g NULL pointer
    ENTRY_DONE = 0, // New list element entered at the chosen index
    REMOVE_DONE = 0, // Element removed successfully.
    INDEX_TOO_LARGE, // The index input was too large.
    LIST_EMPTY // Trying to remove from an empty list
};

typedef enum activeScreen {
    EDITOR_ACTIVE = 0,
    MENU_ACTIVE
} activeScreen_t;

// The different states for the editor
// This contains information regarding the editor
typedef struct editState {
    // Variable containing the current line is active. NOTE: IN PIXELS!
    uint16_t currentLine;
    // Variable containing the cursor location. NOTE: IN PIXELS!
    uint16_t cursorLocation;
    // Index of which to write to the buffer. This is 0 if adding or removing
    // characters to/from the end of the current edit, and non-zero
    // if not at the end.
    uint16_t index;
    // True if insert edit mode is active, false if not.
    // Insert meaning inserting the new char at the cursor location.
    // If insert is false, then it will overwrite the char at the cursor location.
    bool insert;
    // True if the cursor is now written to the screen. This variable toggles everytime the cursor is toggled
    // to keep track weather to write or erase the cursor.
    bool cursorWritten;
} editState_t;

// State of the menu.
// Used for e.g. which item is active now
typedef struct menuState {
    // Variable containing the currently active option.
    uint8_t currentlyActiveOption;
} menuState_t;


typedef struct screenState {
    activeScreen_t activeScreen;
    editState_t editState;
    menuState_t menuState;
    uint8_t batteryLevel; // 0,1,2,3 available
    tFont *screenFont;
} screenState_t;

// Global variables, shared by main.c
PWM_Handle backLightPwmHandle;
SPI_Handle spiHandle;
tDisplay display;
tDisplayData displayData;
tContext grlibContext;

// Mailboxes
// Mailbox between UART and screen threads
Mailbox_Handle uartMailBoxHandle;

// Events
// Event handler for waking up the display task
Event_Handle wakeDisplayEventHandle;

// Clocks
// Clock to trigger every CURSOR_PERIOD_MS ms
Clock_Handle cursorClkHandle;

// Define the first element of the linked list where the
// input is stored. This is the head of the linked list.
listState_t listState;

// Function prototypes:
void setBacklight(PWM_Handle pwm0, uint8_t duty);
void taskFxn(UArg arg0, UArg arg1);
void uartFxn(UArg arg0, UArg arg1);
void displayFxn(UArg arg0, UArg arg1);
void clkFxn(UArg arg0);

#endif /* COMSCICALC_H_ */
