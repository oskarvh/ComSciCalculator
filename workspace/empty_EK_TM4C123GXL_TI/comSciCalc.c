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
#include <driverlib/uart.h>
// Global variables:
const uint32_t depthColor[NUM_DEPTH_COLORS] = {
   ClrWhite,
   ClrAquamarine,
   ClrMediumSeaGreen,
   ClrYellow,
   ClrHotPink
};


// Function to set the TFT display backlight
// Input is in percent, 0 is all off, 100 is full light.
void setBacklight(PWM_Handle pwm0, uint8_t duty){
    if(duty > 100){
        duty = 100;
    }
    PWM_setDuty(pwm0, (PWM_PERIOD*duty)/100);
}

// Function that is called every 500ms, triggers the
// event that the cursor shall be updated.
void clkFxn(UArg arg0){
    Event_post(wakeDisplayEventHandle, EVENT_TIMER_MODULE);
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
        // Check if the first character is escape.
        // In which case, the next two characters shall be read, as it's
        // most probably a special char. Although, just the ESC key will cause this to halt.
        if(readBuf == 0x1b){
            // Escape char, read the two next ones.
            // NOTE: if this is just an ESC key, it will block until two more characters have been sent.
            // If more data is available, then it's an extension of the escape.
            // If not, then it's just an escape key, do nothing.

            // Read the next character
            UART_read(uart, &readBuf, 1);
            if(readBuf == 91){
                // Read the last char, check if arrow key has been pressed.
                UART_read(uart, &readBuf, 1);
                if(readBuf == 68){
                    // Left key
                    readBuf = LEFT_ARROW;
                }
                else if(readBuf == 65){
                    // Up key
                    readBuf = UP_ARROW;
                }
                else if(readBuf == 66){
                    // Down key
                    readBuf = DOWN_ARROW;
                }
                else if(readBuf == 67){
                    // Right key
                    readBuf = RIGHT_ARROW;
                }
                else {
                    // Something else.
                }
            }
            // If the next character is 91 (dec) then it might be an arrow key

        }
        // If this is the case,
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

// Padding function to get length of one character.
// The normal GrFontMaxWidthGet doesn't really do the trick.
uint32_t getCharWidth(void){
    char tmp = 'A';
    return GrStringWidthGet(&grlibContext, &tmp, 1);
}

// Padding function to get the height of one character.
uint32_t getCharHeight(void){
    return GrFontHeightGet(grlibContext.psFont);
}

// Function to set the screen layout, which
// writes the top rows of the screen with functions such
// as settings and battery information
// The sizes of the widgets are set by preprocessor defines
void updateScreenLayout(screenState_t screenState){

    // Draw the battery indicator:
    tRectangle rect;
    // Battery indicator: bounding box
    rect.i16XMin = BATTERY_BOX_RECT_X_START;
    rect.i16XMax = BATTERY_BOX_RECT_X_END;
    rect.i16YMin = BATTERY_BOX_RECT_Y_START;
    rect.i16YMax = BATTERY_BOX_RECT_Y_END;
    RectFill(display.pvDisplayData, &rect, ClrWhite);
    // Battery indicator: stub
    rect.i16XMin = BATTERY_STUB_X_START;
    rect.i16XMax = BATTERY_STUB_X_END;
    rect.i16YMin = BATTERY_STUB_Y_START;
    rect.i16YMax = BATTERY_STUB_Y_END;
    RectFill(display.pvDisplayData, &rect, ClrWhite);
    // Battery indicator: battery level indicators
    rect.i16YMin = BATTERY_IND_Y_START;
    rect.i16YMax = BATTERY_IND_Y_END;
    if(screenState.batteryLevel == 0){
        // Battery almost empty indicator
        rect.i16XMin = BATTERY_IND_1_X_START;
        rect.i16XMax = BATTERY_IND_1_X_START + BATTERY_IND_ALMOST_EMPTY;
        RectFill(display.pvDisplayData, &rect, ClrBlack);
    }
    else {
        if(screenState.batteryLevel > 0){
            // Write the first box
            rect.i16XMin = BATTERY_IND_1_X_START;
            rect.i16XMax = BATTERY_IND_1_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, ClrBlack);
        }
        if(screenState.batteryLevel > 1){
            // Write the second box
            rect.i16XMin = BATTERY_IND_2_X_START;
            rect.i16XMax = BATTERY_IND_2_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, ClrBlack);
        }
        if(screenState.batteryLevel > 2){
            // Write the third box
            rect.i16XMin = BATTERY_IND_3_X_START;
            rect.i16XMax = BATTERY_IND_3_X_START + BATTERY_IND_X_WIDTH;
            RectFill(display.pvDisplayData, &rect, ClrBlack);
        }
    }

    // Write the text(s). Tests for now. This will be replaced by actual statuses.
    const char testBuf[4] = {'T', 'S', 'T', '1'};
    GrContextFontSet(&grlibContext, PTR_TEXT_BOX_FONT);

    // Write the first text box:
    // Clear the box before overwriting it.
    rect.i16XMin = TEXT_BOX_1_X_START;
    rect.i16XMax = TEXT_BOX_1_X_START + 4*getCharWidth();
    // The height is determined by the font.
    rect.i16YMin = TEXT_BOX_Y_START;
    rect.i16YMax = TEXT_BOX_Y_START + getCharHeight();
    if(screenState.editState.insert){
        RectFill(display.pvDisplayData, &rect, ClrBlack);
        const char insertBuf[4] = {'I','N','S',' '};
        GrStringDraw(&grlibContext, insertBuf, 4, TEXT_BOX_1_X_START, TEXT_BOX_Y_START, true);
    }
    else {
        RectFill(display.pvDisplayData, &rect, ClrBlack);
        const char insertBuf[4] = {'O','V','R','W'};
        GrStringDraw(&grlibContext, insertBuf, 4, TEXT_BOX_1_X_START, TEXT_BOX_Y_START, true);
    }
    //GrStringDraw(&grlibContext, testBuf, 4, TEXT_BOX_1_X_START, TEXT_BOX_Y_START, false);
    GrStringDraw(&grlibContext, testBuf, 4, TEXT_BOX_2_X_START, TEXT_BOX_Y_START, false);
    GrStringDraw(&grlibContext, testBuf, 4, TEXT_BOX_3_X_START, TEXT_BOX_Y_START, false);
    // Restore the screen font to default:
    GrContextFontSet(&grlibContext, screenState.screenFont);

    // Draw the cutline, top of screen.
    LineDrawH(display.pvDisplayData, 0, HX8357_TFTWIDTH, CUTLINE_Y, HX8357_WHITE);

}

// Function to print the menu
void printMenuLayout(screenState_t screenState){

}

// Function to update/toggle the cursor
void updateCursor(screenState_t *pScreenState, listState_t * pListState){
    if(pScreenState->activeScreen == EDITOR_ACTIVE){
        if(pScreenState->editState.insert){
            // "Normal" edit mode, insert a new character at the location
            // This is a normal blinking vertical line.
            // Use the rectangle draw function in case we want to change the
            // width at some point in the future
            tRectangle rect;
            // The x-location is determined by the cursor location variable.
            rect.i16XMin = pScreenState->editState.cursorLocation;
            rect.i16XMax = pScreenState->editState.cursorLocation+1;
            // The height is determined by the font.
            // Remember: (0,0) is the top left of the screen
            rect.i16YMin = pScreenState->editState.currentLine;
            rect.i16YMax = rect.i16YMin + getCharHeight() - 1;
            uint32_t color;
            if(pScreenState->editState.cursorWritten){
                color = ClrBlack;
            }
            else{
                color = ClrWhite;
            }
            // Write the cursor
            RectFill(display.pvDisplayData, &rect, color);
            // Toggle the internal state
            pScreenState->editState.cursorWritten = !pScreenState->editState.cursorWritten;
        }
        else {
            // Toggle the inversion of the colors of the current
            // character to indicate that the current character will
            // be overwritten.
            // If index is zero, and/or if the list is empty, then simply
            // toggle a rectangle at the end.
            uint32_t color;
            if(pScreenState->editState.cursorWritten){
                color = ClrBlack;
            }
            else{
                color = ClrWhite;
            }
            // Toggle an empty rectangle:
            tRectangle rect;
            // The x-location is determined by the cursor location variable.
            rect.i16XMin = pScreenState->editState.cursorLocation;
            rect.i16XMax = pScreenState->editState.cursorLocation + getCharWidth();
            // The height is determined by the font.
            // Remember: (0,0) is the top left of the screen
            rect.i16YMin = pScreenState->editState.currentLine;
            rect.i16YMax = rect.i16YMin + getCharHeight() - 1;
            // Write the rectangle:
            RectFill(display.pvDisplayData, &rect, color);
            // Now that the character has been cleared, toggle the character.
            if(pScreenState->editState.index > 0){
                // Toggle the next character (at the index -1)
                if(pScreenState->editState.cursorWritten){
                    // Cursor was written last time, i.e. we should set it back
                    // This means that the background and foreground should be reset
                    GrContextForegroundSet(&grlibContext, ClrWhite); // white foreground
                    GrContextBackgroundSet(&grlibContext, ClrBlack); // black background
                }
                else{
                    GrContextForegroundSet(&grlibContext, ClrBlack); // black foreground
                    GrContextBackgroundSet(&grlibContext, ClrWhite); // white background
                }
                // Write the character at index -1:
                char toggleChar = 0;
                getCharFromList(&listState, &toggleChar, pScreenState->editState.index-1);
                GrStringDraw(&grlibContext,
                             &toggleChar,
                             1,
                             pScreenState->editState.cursorLocation,
                             pScreenState->editState.currentLine,
                             true);

            }
            // Toggle the internal state
            pScreenState->editState.cursorWritten = !pScreenState->editState.cursorWritten;
        }
    }
}

// Function to update the screen and print the entire list.
// This should be called every time a button has been pressed, except the menu button.
// NOTE: if the printing is longer than the screen, some scrolling is needed.
// If it's too long, then the cursor should decide what part is printed.
// TODO: handle functions with inputs longer than 1 character width.
void printListElements(listState_t *pListState, screenState_t *pScreenState, bool bUpdateScreen){

    if(bUpdateScreen){
        // Clear the screen segment before writing the text to it.
        tRectangle rect;
        // Start at the current index, as nothing will change before that
        // (unless insert == false)
        // We have the number of characters on the screen last time, which gives us the
        // maximum width we need to clear:
        rect.i16XMax = getCharWidth()*(pScreenState->editState.numCharsOnScreen);
        // As to how far we need to clear, that depends on the number of entries in the
        // list, and what the index is.
        // If there are more entries in the list than are shown on the screen,
        // then get the minimum from the number of character on screen.
        if(pListState->numEntries > pScreenState->editState.numCharsOnScreen){
            rect.i16XMin = getCharWidth()*(pScreenState->editState.numCharsOnScreen
                                           - pScreenState->editState.index);
        }
        else{
            // If the same amount of entries, then get the minimum from the number of entries and index
            rect.i16XMin = getCharWidth()*(pListState->numEntries - pScreenState->editState.index);
        }
        // If index is not zero, and insert is false, that means that we need to
        // render the character that was just overwritten as well.
        if((!pScreenState->editState.insert)){
            rect.i16XMin -= getCharWidth();
            // Do a zero check as well.
            if(rect.i16XMin < 0){
                rect.i16XMin = 0;
            }
        }
        // If the number of characters extend beyond the screen, we need to scroll.
        // Therefore, we need to blackout everything
        if(rect.i16XMax > HX8357_TFTWIDTH){
            rect.i16XMin = 0;
            rect.i16XMax = HX8357_TFTWIDTH;
        }
        // The height is determined by the font.
        // Remember: (0,0) is the top left of the screen
        rect.i16YMin = pScreenState->editState.currentLine;
        rect.i16YMax = rect.i16YMin + pScreenState->screenFont->ui8Height;
        // Write the blackout rectangle
        //RectFill(display.pvDisplayData, &rect, ClrBlack);


        // Get the list entry:
        listElement_t *pListEntry = pListState->pListEntry;
        // The first element is the first element entered into the list.
        // If the entry is NULL, then nothing to print.
        if(pListEntry != NULL){
            // Allocate a char buffer for the elements in the list
            char* pTmpBuf = (char*)malloc(pListState->numEntries);

            // Entry is not zero, loop through the list and print everything.
            listElement_t *pCurrentElement = pListEntry;
            if(pTmpBuf == NULL){
                // If the local buffer could not be allocated, simply print each character.
                // Loop until the next element is NULL, i.e. end of list.
                // Should be slower than writing directly from char array
                uint16_t cursorLocation = 0;
                // Blackout the screen before writing data
                RectFill(display.pvDisplayData, &rect, ClrBlack);
                while(pCurrentElement != NULL){
                    GrStringDraw(&grlibContext,
                                 &(pCurrentElement->currentEntry.currentChar),
                                 1,
                                 cursorLocation,
                                 pScreenState->editState.currentLine,
                                 false);
                    cursorLocation += getCharWidth();
                    pCurrentElement = pCurrentElement->pNextElem;
                }
            }
            else {
                // Copy the elements from the list to a temporary buffer,
                // making it possible to print all characters at the same time.
                // Time gained from this is unknown.
                int count = 0;
                while(pCurrentElement != NULL){
                    pTmpBuf[count++] = pCurrentElement->currentEntry.currentChar;
                    pCurrentElement = pCurrentElement->pNextElem;
                }
                // Blackout the screen before writing data
                RectFill(display.pvDisplayData, &rect, ClrBlack);
                GrStringDraw(&grlibContext,
                             pTmpBuf,
                             pListState->numEntries,
                             0,
                             pScreenState->editState.currentLine,
                             false);
                free(pTmpBuf);
            }
        }
        else{
            // Blackout the screen even if there are no entries.
            // could be a case where there is no entries because everything was
            // deleted by the user.
            RectFill(display.pvDisplayData, &rect, ClrBlack);
        }
    }
    // Calculate the cursor location based on the index.
    pScreenState->editState.cursorLocation =
            (pListState->numEntries - pScreenState->editState.index) *
            getCharWidth();
    // Set the number of characters on the screen to the
    // number of entries in the list.
    pScreenState->editState.numCharsOnScreen = pListState->numEntries;
}

// Function to receive content from input mailbox and
// display it on the screen.
// It also is responsible for placing the data in the interpreters buffer
// and display the result on the screen.
// It shall also keep track of the state of the screen: edit mode or menu mode
//
// This task should be called every CURSOR_PERIOD_MS ms to update the cursor.
// However, there are several events that can trigger this task to get out of sleep:
// 1. User input
// 2. Timer module (to update the cursor)
void displayFxn(UArg arg0, UArg arg1){
    // Initialize the screen state:
    screenState_t screenState;
    screenState.activeScreen = EDITOR_ACTIVE; // Start in editor mode
    screenState.editState.currentLine = CUTLINE_Y +3; // Start at the top of the screen, just below the cutline
    screenState.editState.cursorLocation = 0; // Start at cursor location 0
    screenState.editState.insert = true; // default is using the insert edit mode
    screenState.batteryLevel = 3; // HACK: set full battery indicator
    screenState.editState.cursorWritten = false; // Start with no cursor written.
    screenState.editState.index = 0; // Start writing at the back of the list.
    screenState.editState.numCharsOnScreen = 0; // Start with no characters on the screen.
    screenState.screenFont = g_psFontCmtt38; // A bit of a hack with the type casting..

    // Initialize the screen layout
    updateScreenLayout(screenState);
    char uartInputBuf;

    // Loop forever.
    // This task shall never end. All waiting is done inside of the loop
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

            // Declare boolean to say if the operation should update the screen or not
            bool bUpdateScreen = false;
            // Check if in menu or edit mode:
            if(screenState.activeScreen == EDITOR_ACTIVE){
                // In edit state
                // Act on the input
                // The cursor should blink on each input, therefore remove the cursor first
                if(screenState.editState.cursorWritten){
                    updateCursor(&screenState, &listState);
                }

                if(uartInputBuf == TOGGLE_MENU_BUTTON){
                    // Write the menu
                    printMenuLayout(screenState);
                    // set the screen state to menu mode
                    screenState.activeScreen = MENU_ACTIVE;
                    // No need to update the screen, therefore break
                    break;
                }
                else if(uartInputBuf == LEFT_ARROW){
                    // Move the cursor left
                    if(screenState.editState.index < listState.numEntries){
                        // Move the index, if not at the end of the input buffer.
                        screenState.editState.index += 1;
                    }
                }
                else if(uartInputBuf == RIGHT_ARROW){
                    // Move the cursor left
                    if(screenState.editState.index > 0){
                        // Move the index, if not at the end of the input buffer.
                        screenState.editState.index -= 1;
                    }
                }
                else if(uartInputBuf == UP_ARROW){
                    // TODO: Fetch the last calculation from memory.
                    // If enter/= is pressed after this, copy it to the
                    // current buffer.
                }
                else if(uartInputBuf == DOWN_ARROW){
                    // TODO: Navigate the previous calculations, fetching them
                    // from non-volatile memory.
                }
                else if(uartInputBuf == BACKSPACE){
                    // Backspace, remove the character before the cursor.
                    removeListEntry(&listState, screenState.editState.index);
                    // This should update the screen:
                    bUpdateScreen = true;
                }
                else if(uartInputBuf == TOGGLE_INSERT){
                    // Toggle the insert:
                    screenState.editState.insert = !screenState.editState.insert;
                    // Update the screen layout to reflect it.
                    updateScreenLayout(screenState);
                }
                else {
                    // Write whatever else input to the screen and input buffer
                    // At the character to the linked list.
                    // Check if inserting or overwriting, and act accordingly.
                    if(screenState.editState.insert){
                        addListEntry(&listState, uartInputBuf, screenState.editState.index);
                    } else {
                        overwriteListEntry(&listState, uartInputBuf, screenState.editState.index);
                        // For the overwrite to make sense, we also want to move the cursor/index
                        // to the right
                        if(screenState.editState.index > 0){
                            // Move the index, if not at the end of the input buffer.
                            screenState.editState.index -= 1;
                        }
                    }
                    // This should update the screen:
                    bUpdateScreen = true;
                }
                // Update the screen.
                // Print the entire buffer.
                printListElements(&listState, &screenState, bUpdateScreen);

                // Print the cursor again
                updateCursor(&screenState, &listState);
            }
            else {
                // In menu state
            }
        }
        else if(events & EVENT_TIMER_MODULE){
            // Timer module kicked in.
            // If in editor mode, toggle the cursor
            updateCursor(&screenState, &listState);
        }
    }
}
