/*
 * comSciCalc.c
 *
 *  Created on: 28 aug. 2022
 *      Author: Oskar von Heideken
 */

/*
 * Brief:
 * This is the source file for all function related to the modern computer science calculator.
 *
 * */

// Include this files header file
#include <comSciCalc.h>

// Global variables:



// Function to set the TFT display backlight
// Input is in percent, 0 is all off, 100 is full light.
void setBacklight(PWM_Handle pwm0, uint8_t duty){
    if(duty > 100){
        duty = 100;
    }
    PWM_setDuty(pwm0, (PWM_PERIOD*duty)/100);
}

// Function to display uart buffer on screen
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

// Function to read from UART and send the result onwards in a mailbox
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
        // Send the read character to the screen task in a mailbox event.
        // This will post the wakeDisplayEventHandle event as well.
        Mailbox_post(uartMailBoxHandle, &readBuf, BIOS_WAIT_FOREVER);
        // Post the event to wake the screen task as well
        //Event_Post(wakeDisplayEventHandle, EVENT_USER_INPUT);
    }
}

// Function to set the screen layout, which
// writes the top rows of the screen with functions such
// as settings and battery information
// The sizes of the widgets are set by preprocessor defines
void updateScreenLayout(screenState_t screenState){

    // Draw the cutline, top of screen.
    LineDrawH(display.pvDisplayData, 0, HX8357_TFTWIDTH, CUTLINE_Y, HX8357_WHITE);

    // Draw the battery indicator:
    tRectangle rect;
    // Battery indicator: bounding box
    rect.i16XMin = BATTERY_BOX_RECT_X_START;
    rect.i16XMax = BATTERY_BOX_RECT_X_END;
    rect.i16YMin = BATTERY_BOX_RECT_Y_START;
    rect.i16YMax = BATTERY_BOX_RECT_Y_END;
    RectFill(display.pvDisplayData, &rect, HX8357_WHITE);
    // Battery indicator: stub
    rect.i16XMin = BATTERY_STUB_X_START;
    rect.i16XMax = BATTERY_STUB_X_END;
    rect.i16YMin = BATTERY_STUB_Y_START;
    rect.i16YMax = BATTERY_STUB_Y_END;
    RectFill(display.pvDisplayData, &rect, HX8357_WHITE);
    // Battery indicator: battery level indicators
    rect.i16YMin = BATTERY_IND_Y_START;
    rect.i16YMax = BATTERY_IND_Y_END;
    if(screenState.batteryLevel == 0){
        // Battery almost empty indicator
        rect.i16XMin = BATTERY_IND_1_X_START;
        rect.i16XMax = BATTERY_IND_1_X_START + BATTERY_IND_ALMOST_EMPTY;
        RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
    }
    else {
        if(screenState.batteryLevel > 0){
            // Write the first box
            rect.i16XMin = BATTERY_IND_1_X_START;
            rect.i16XMax = BATTERY_IND_1_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
        }
        if(screenState.batteryLevel > 1){
            // Write the second box
            rect.i16XMin = BATTERY_IND_2_X_START;
            rect.i16XMax = BATTERY_IND_2_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
        }
        if(screenState.batteryLevel > 2){
            // Write the third box
            rect.i16XMin = BATTERY_IND_3_X_START;
            rect.i16XMax = BATTERY_IND_3_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, HX8357_BLACK);
        }
    }

    // Write the text(s). Tests for now.
    char testBuf[4] = {'T', 'S', 'T', '1'};
    GrContextFontSet(&grlibContext, PTR_TEXT_BOX_FONT);
    GrStringDraw(&grlibContext, &testBuf, 4, TEXT_BOX_1_X_START, TEXT_BOX_Y_START, false);
    GrStringDraw(&grlibContext, &testBuf, 4, TEXT_BOX_2_X_START, TEXT_BOX_Y_START, false);
    GrStringDraw(&grlibContext, &testBuf, 4, TEXT_BOX_3_X_START, TEXT_BOX_Y_START, false);
    // Restore the screen font to default:
    GrContextFontSet(&grlibContext, screenState.screenFont);

}

// Function to add an entry to the linked list.
// takes in a pointer to the list state,
// the character that should be entered in the list,
// and the index FROM THE LAST ENTRY(!).
// For example, if index = 0, then the entry will be placed at the last
// spot on the list. If index = 1, then the new entry will be placed between
// the last and second to last element.
// If index is larger than the list is long, then the entry will be allocated
// as the first entry of the list.
int addListEntry(listState_t *pListState, char entry, uint8_t index){
    // Get the pointer to the list entry
    listElement_t *pListEntry = pListState->pListEntry;

    // Check if the list is un-initialized
    if(pListEntry == NULL){
        // Allocate the first list entry.
        pListEntry = (listElement_t*)malloc(sizeof(listElement_t));
        if(pListEntry == NULL){
            // Allocation failed, return with failure warning
            return ALLOCATION_FAILED;
        }
        // Enter the character
        pListEntry->currentChar = entry;

        // Since this is the only entry to the list so far, set all pointers to NULL
        pListEntry->pNextElem = NULL;
        pListEntry->pPrevElem = NULL;

        // Set the state variables. The end and start list entries are the same.
        pListState->pListEntry = pListEntry;
        pListState->pListEnd = pListEntry;
        pListState->numEntries += 1;

        // Check if the index was anything else than 0
        if(index > 0){
            return INDEX_TOO_LARGE;
        }
        else{
            return ENTRY_DONE;
        }
    }
    else{
        // A list already exists.
        // Therefore, get the tail end of the list
        listElement_t *pListEnd = pListState->pListEnd;

        // Do a NULL check, and if true there is an error
        if(pListEnd == NULL){
            return LIST_END_ERROR;
        }

        // Iterate through the list index amount of times to find
        // the entry into which this character shall be placed,
        // starting at the end of the list.
        listElement_t *pCurrentElement = pListEnd;
        int i;
        for(i = 0 ; i < index ; i++){
            // Set the current entry to the previous entry
            pCurrentElement = pCurrentElement->pPrevElem;
            if(pCurrentElement == NULL){
                break;
            }
        }

        // Then, insert the new element here.
        // Start by allocating a new element
        listElement_t *pNewElement = (listElement_t*)malloc(sizeof(listElement_t));
        if(pNewElement == NULL){
            // Allocation failed, return with failure warning
            return ALLOCATION_FAILED;
        }
        // Populate the new elements parameters:
        // Enter the character
        pNewElement->currentChar = entry;
        // Squeeze the new element into the list
        if(pCurrentElement == NULL){
            // If the current entry is NULL, that means that
            // we're at the beginning of the list.
            // Therefore, replace the list entry with this entry:

            // Set the next element of the new element to the list entry
            pNewElement->pNextElem = pListState->pListEntry;

            // Set the previous element to NULL to indicate that this
            // is the first entry
            pNewElement->pPrevElem = NULL;

            // Set the list entries previous entry to the new element
            pListState->pListEntry->pPrevElem = pNewElement;

            // Overwrite the first entry in the list state
           pListState->pListEntry = pNewElement;
           if(i == index){
               return ENTRY_DONE;
           }
           else{
               return INDEX_TOO_LARGE;
           }
        }
        else {
            // The current entry is not NULL, which means that we're somewhere else in the list.
            // Insert an element in the list OR insert an element at the end of the list.
            // If it's the end of the list, then update the state.
            // Ensure that the list isn't broken when inserting.

            // The current element points to what will be the new element previous element,
            // therefore set the previous element of the new element to the current element
            // (I know, it's confusing..)
            pNewElement->pPrevElem = pCurrentElement;
            // Get the next element in the list, pointed to by the current entry, and
            // set that as the new element next element:
            pNewElement->pNextElem = pCurrentElement->pNextElem;
            // Set the next elements previous element to the new element.
            // If this was the last entry, then set the previous element to the entry,
            // and change the last entry of the state.
            if(pNewElement->pNextElem != NULL){
                ((listElement_t*)(pNewElement->pNextElem))->pPrevElem = pNewElement;
            }
            else{
                pListState->pListEnd = pNewElement;
            }

            // And finally, set the current elements next pointer to this new one:
            pCurrentElement->pNextElem = pNewElement;
        }
        return ENTRY_DONE;

    }
}

// Function to remove a list entry
int removeListEntry(listState_t *pListState, uint8_t index){

}

// Function to receive content from input mailbox and
// display it on the screen.
// It also is responsible for placing the data in the interpreters buffer
// and display the result on the screen.
// It shall also keep track of the state of the screen: edit mode or menu mode
//
// This task should be called every 500 ms to update the cursor.
// However, there are several events that can trigger this task to get out of sleep:
// 1. User input
// 2. Timer module (to update the cursor)
void displayFxn(UArg arg0, UArg arg1){
    // Initialize the screen state:
    screenState_t screenState;
    screenState.activeScreen = EDITOR_ACTIVE; // Start in editor mode
    screenState.editState.currentLine = 0; // Start at the top of the screen
    screenState.editState.cursorLocation = 0; // Start at cursor location 0
    screenState.editState.insert = true; // default is using the insert edit mode
    screenState.batteryLevel = 3; // HACK: set full battery indicator
    screenState.screenFont = &g_sFontCmtt38;
    // Initialize the screen layout
    updateScreenLayout(screenState);
    char uartInputBuf;
    // Loop forever.
    // This task shall never end. All waiting is done inside of the loop
    int x, y; //DEBUG
    x = 0; // DEBUG
    y = CUTLINE_Y +3; //DEBUG
    while(1){
        // Pend on the wake up event. Only pend on OR mask.
        unsigned int events = Event_pend(wakeDisplayEventHandle,
                                         Event_Id_NONE,
                                         (EVENT_TIMER_MODULE + EVENT_USER_INPUT),
                                         BIOS_WAIT_FOREVER);
        // 1. Check what woke up this task.
        if(events & EVENT_USER_INPUT){
            // Read the mailbox
            Mailbox_pend(uartMailBoxHandle, &uartInputBuf, BIOS_NO_WAIT);
            // At the character to the linked list, at the end
            addListEntry(&listState, uartInputBuf, 0);
            // Depending on the input, either write to the screen, change a state
            // or invoke the interpreter for calculation.
            char testBuf[4] = {'T','E','S','T'};

            GrStringDraw(&grlibContext, &uartInputBuf, 1, x, y, true);
            //x += screenState.screenFont->ui8MaxWidth;

        }
        else if(events & EVENT_TIMER_MODULE){
            // Timer module kicked in.
            // If in editor mode, toggle the cursor
            if(screenState.activeScreen == EDITOR_ACTIVE){
                // In the editor.
                // Toggle the cursor
                // TODO: Toggle the cursor
            }
        }


        if(screenState.activeScreen == EDITOR_ACTIVE){
            // In the editor.
            // Check what the
        }
    }
}
