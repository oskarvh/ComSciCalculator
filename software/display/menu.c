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

// Standard library
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "display.h"
#include "firmware_common.h"
#include "menu.h"

menuOption_t bitSizesMenuList[] = {
    [0] =
        {
            .pOptionString = "Update number of bits",
            .pSubMenu = NULL,
            .pUpdateFun = NULL,  // TODO: Insert function to call update bits.
            .pDisplayFun = NULL, // TODO: Insert function
        },
    [1] =
        {
            .pOptionString = "Update fixed point fractional bits",
            .pSubMenu = NULL,
            .pUpdateFun = NULL,  // TODO: Insert function
            .pDisplayFun = NULL, // TODO: Insert function
        },
    // All submenus should have this option. It just makes sense
    [2] =
        {
            .pOptionString = "Back",
            .pSubMenu = NULL,
            .pUpdateFun = &goUpOneMenu, // TODO: Insert function
            .pDisplayFun = NULL,        // TODO: Insert function
        },
    // END OF LIST, all NULL
    [3] =
        {
            .pOptionString = NULL,
            .pSubMenu = NULL,
            .pUpdateFun = NULL,
            .pDisplayFun = NULL,
        },
};

menuOption_t topLevelMenuList[] = {
    [0] =
        {
            .pOptionString = "Bit sizes",
            .pSubMenu = &bitSizesMenu,
            .pUpdateFun = NULL,
            .pDisplayFun = NULL,
        },
    [1] =
        {
            .pOptionString = "Change font",
            .pSubMenu = NULL,
            .pUpdateFun = &changeFont,
            .pDisplayFun = &getCurrentFont,
        },
    // All submenus should have this option. It just makes sense
    [2] =
        {
            .pOptionString = "Exit",
            .pSubMenu = NULL,
            .pUpdateFun = &goUpOneMenu, // TODO: Insert function
            .pDisplayFun = NULL,        // TODO: Insert function
        },
    // END OF LIST, all NULL
    [3] =
        {
            .pOptionString = NULL,
            .pSubMenu = NULL,
            .pUpdateFun = NULL,
            .pDisplayFun = NULL,
        },
};

//! Top level menu
menuState_t topMenu = {
    .pMenuOptionList = topLevelMenuList,
    .pCurrentMenuOption = &(topLevelMenuList[0]),
    .pUpperMenu = NULL,
};

//! Menu for bit sizes
menuState_t bitSizesMenu = {
    .pMenuOptionList = bitSizesMenuList,
    .pCurrentMenuOption = &(bitSizesMenuList[0]),
    .pUpperMenu = &topMenu,
};

// Functions for doing stuff in the menus
void getCurrentFont(char *pString) {}
void changeFont() {}
void goUpOneMenu() {}

int findCurrentMenuOption(menuState_t *pMenuState) {
    menuOption_t *pCurrentMenuOption = pMenuState->pCurrentMenuOption;
    menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
    int i = 0;
    while (pMenuOption->pOptionString != NULL) {
        // The pMenuOption points to a list of menu options.
        // Go through and print.
        bool selected_item = false;
        if (pCurrentMenuOption == pMenuOption) {
            return i;
        }
        i++;
        pMenuOption++;
    }
}

void updateMenuState(displayState_t *pLocalDisplayState,
                     displayState_t *pGlobalDisplayState) {
    do {
        char receiveChar = 0;
        calc_funStatus_t addRemoveStatus;
        if (xQueueReceive(uartReceiveQueue, &receiveChar,
                          (TickType_t)portMAX_DELAY)) {
            if (receiveChar == 27) {
                // Escape char. We expect two more chars in there, but don't
                // wait around for it
                char escapeSeq[3] = {0};
                for (int i = 0; i < 2; i++) {
                    xQueueReceive(uartReceiveQueue, &(escapeSeq[i]),
                                  (TickType_t)10);
                }
                escapeSeq[2] = '\0';
                if (strcmp(escapeSeq, "[A") == 0) {
                    // Up
                    menuState_t *pMenuState = pLocalDisplayState->pMenuState;
                    menuOption_t *pCurrentMenuOption =
                        pMenuState->pCurrentMenuOption;
                    menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
                    // Check if the current menu option points to the
                    // first entry to the list:
                    if (pCurrentMenuOption !=
                        &(pMenuState->pMenuOptionList[0])) {
                        // We're not at the top, meaning we can go down.
                        int idx = findCurrentMenuOption(pMenuState) - 1;
                        pMenuState->pCurrentMenuOption =
                            &(pMenuState->pMenuOptionList[idx]);
                    }
                }
                if (strcmp(escapeSeq, "[B") == 0) {
                    // Down
                    menuState_t *pMenuState = pLocalDisplayState->pMenuState;
                    menuOption_t *pCurrentMenuOption =
                        pMenuState->pCurrentMenuOption;
                    menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
                    // Check if the current menu option points to the
                    // first entry to the list:
                    if ((++pCurrentMenuOption)->pOptionString == NULL) {
                        // We're at the bottom, revert the change we made.
                        pCurrentMenuOption--;
                    } else {
                        int idx = findCurrentMenuOption(pMenuState) + 1;
                        pMenuState->pCurrentMenuOption =
                            &(pMenuState->pMenuOptionList[idx]);
                    }
                }
                if (strcmp(escapeSeq, "[C") == 0) {
                }
                if (strcmp(escapeSeq, "[D") == 0) {
                    // Backward/left
                }
                // Here there's a USB espace char, and something else in the
                // queue C (right), D(left), A (up) or ? (down)
            }
            if (receiveChar == '\r') {
                // This is carriage return.
                // If there is an associated function, run that.
                // else if there is a submenu, enter that
                menuState_t *pMenuState = pLocalDisplayState->pMenuState;
                menuOption_t *pCurrentMenuOption =
                    pMenuState->pCurrentMenuOption;
                menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
                if (pCurrentMenuOption->pUpdateFun != NULL) {
                    // Run the update function
                    // TODO: Figure out how this should be run.
                }
                if (pCurrentMenuOption->pSubMenu != NULL) {
                    // Enter the sub menu
                    pMenuState->pCurrentMenuOption =
                        &(((menuState_t *)(pCurrentMenuOption->pSubMenu))
                              ->pMenuOptionList[0]);
                    pMenuState->pMenuOptionList =
                        ((menuState_t *)(pCurrentMenuOption->pSubMenu))
                            ->pMenuOptionList;
                }
            }
            if (receiveChar == 't' || receiveChar == 'T') {
                pLocalDisplayState->inMenu = false;
            }
            // Copy over the local state to the global state.
            if (xSemaphoreTake(displayStateSemaphore, portMAX_DELAY)) {
                // To save time, copy the display state. Sort of waste of
                // space. Not sure if this is the best approach..
                memcpy(pGlobalDisplayState, pLocalDisplayState,
                       sizeof(displayState_t));
                // Release the semaphore, since we're done with the display
                // state global variable
                xSemaphoreGive(displayStateSemaphore);
                // Give the event to the main loop to give back control
                xEventGroupSetBits(displayTriggerEvent, DISPLAY_EXIT_MENU);
                if (pLocalDisplayState->inMenu == false) {
                    // Return if we're exiting the menu, since we don't
                    // want to read from the receive queue.
                    return;
                }
            }
        }
    } while (uxQueueMessagesWaiting(uartReceiveQueue) > 0);
}