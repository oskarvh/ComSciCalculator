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

#include "EVE.h"
#include "display.h"
#include "firmware_common.h"
#include "menu.h"
#include "uart_logger.h"

menuOption_t bitSizesMenuList[] = {
    [0] =
        {
            .pOptionString = "Update number of bits",
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = true,
                    .pUpdateFun = &updateBitWidth,
                },
            .pDisplayFun = &getBitSize,
        },
    [1] =
        {
            .pOptionString = "Update fixed point fractional bits",
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = NULL, // TODO: Update this
                },
            .pDisplayFun = &getFractionalBits,
        },
    // All submenus should have this option. It just makes sense
    [2] =
        {
            .pOptionString = "Back",
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = &goUpOneMenu, // TODO: Insert function
                },
            .pDisplayFun = NULL, // TODO: Insert function
        },
    // END OF LIST, all NULL
    [3] =
        {
            .pOptionString = NULL,
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = NULL,
                },
            .pDisplayFun = NULL,
        },
};

menuOption_t topLevelMenuList[] = {
    [0] =
        {
            .pOptionString = "Update bit size",
            .pSubMenu = &bitSizesMenu,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = NULL,
                },
            .pDisplayFun = NULL,
        },
    [1] =
        {
            .pOptionString = "Change font",
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = &changeFont,
                },
            .pDisplayFun = &getCurrentFont,
        },
    // Top level menu needs an exit.
    [2] =
        {
            .pOptionString = "Exit",
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = &exitMenu,
                },
            .pDisplayFun = NULL, // TODO: Insert function
        },
    // END OF LIST, all NULL
    [3] =
        {
            .pOptionString = NULL,
            .pSubMenu = NULL,
            .menuUpdateFun =
                {
                    .interactiveUpdateFun = false,
                    .pUpdateFun = NULL,
                },
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
void getCurrentFont(displayState_t *pDisplayState, char *pString) {
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pSmallFont;
    if (strlen(pCurrentFont->font_name) > MAX_MENU_DISPLAY_FUN_STRING) {
        // TODO: The name is too long. Cut it
    }
    strcpy(pString, pCurrentFont->font_name);
}

// Function to return the current bit size/width
void getBitSize(displayState_t *pDisplayState, char *pString) {
    uint8_t numBits = pDisplayState->inputOptions.numBits;
    sprintf(pString, "Current:\n%i bits", numBits);
}

// Function to display the fractional bits
void getFractionalBits(displayState_t *pDisplayState, char *pString) {
    uint8_t numBits = pDisplayState->inputOptions.numBits;
    uint8_t decimalBits =
        getEffectiveFixedPointDecimalPlace(&(pDisplayState->inputOptions));
    // Work out the Q notation:
    uint8_t integerBits = numBits - decimalBits;
    sprintf(pString, "Current:\n%u.%u\n[int.dec]", integerBits, decimalBits);
}

// Function to update the number of bits:
// NOTE: THIS HIJACKS THE DISPLAY AND RECEIVE QUEUE!
void updateBitWidth(displayState_t *pDisplayState,
                    QueueHandle_t *pUartReceiveQueue) {

// This should show a new screen, with the option to enter a number between
// 0-64.
#define MAX_BIT_WIDTH_LEN 2
#define MAX_NUM_MENU_ITEMS 2
    // Buffer to hold the entered chars. Currently only supporting up to 64
    // bits, so 2 chars + null char is enough to hold the entered number.
    char pEnteredChars[MAX_BIT_WIDTH_LEN + 1] = {"\0"};
    // Iterator to track how many chars have been written to the buffer
    uint8_t charIter = 0;
    // Iterator to track the current menu selection
    uint8_t selectionIter = 0;
    // Get the current font:
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
    // Help string for entering the number
    char *pHelpString =
        "Enter number of bits\nMax: 64\nMin: 1\nPress enter to accept";
    char *pAbortString = "Abort";

    // Use the same parameters as for the menu here
    uint16_t frameStart_x =
        MENU_FRAME_OFFSET_X + MENU_FRAME_TO_OPTION_FRAME_OFFSET_X;
    uint16_t frameStart_y =
        MENU_FRAME_OFFSET_Y + MENU_FRAME_TO_OPTION_FRAME_OFFSET_Y;
    uint16_t frameEnd_x = EVE_HSIZE - frameStart_x;
    uint16_t start_x = frameStart_x + MENU_OPTION_FRAME_TEXT_PADDING_X;
    uint16_t start_y = frameStart_y + MENU_OPTION_FRAME_TEXT_PADDING_Y;
    uint16_t end_x = EVE_HSIZE - start_x;
    uint16_t textPadding_x = MENU_OPTION_FRAME_TEXT_PADDING_X;
    uint16_t textPadding_y = MENU_OPTION_FRAME_TEXT_PADDING_Y;

    // Precompute how manu lines the help string would be
    int numLinesHelpString = printMenuOptionString(
        pDisplayState, pHelpString, start_x, start_y,
        (end_x - start_x) / 2 - MENU_OPTION_FRAME_TEXT_PADDING_X / 2,
        MENU_OPTION_LINE_TEXT_PADDING,
        false, // left justified
        false  // Don't print the string to screen
    );
    while (true) {
        // Print the screen.
        startDisplaylist();
        // Print the menu outline to keep things consistent.
        printMenuOutline(MENU_FRAME_OFFSET_X, MENU_FRAME_OFFSET_Y);

        // Print the helpString frame:
        // Write the rectangle encompassing the option.
        uint16_t menuOptionFrameHeight =
            2 * textPadding_y +
            numLinesHelpString * pCurrentFont->font_caps_height +
            (numLinesHelpString - 1) * MENU_OPTION_LINE_TEXT_PADDING;
        if (selectionIter == 0) {
            EVE_color_rgb_burst(WHITE);
        } else {
            EVE_color_rgb_burst(GRAY);
        }
        // Write the rectangle encompassing the option.
        EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
        EVE_cmd_dl(VERTEX2F(frameStart_x * 16, frameStart_y * 16));
        EVE_cmd_dl(VERTEX2F(frameEnd_x * 16,
                            (frameStart_y + menuOptionFrameHeight) * 16));
        if (selectionIter == 0) {
            EVE_color_rgb_burst(BLACK);
        } else {
            EVE_color_rgb_burst(WHITE);
        }
        numLinesHelpString = printMenuOptionString(
            pDisplayState, pHelpString, start_x, start_y,
            (end_x - start_x) / 2 - MENU_OPTION_FRAME_TEXT_PADDING_X / 2,
            MENU_OPTION_LINE_TEXT_PADDING,
            false, // left justified
            true);
        if (selectionIter == 0) {
            // Here, there's an option to blink red or something.
            EVE_color_rgb_burst(BLACK);
        }
        int numLinesNumBits = printMenuOptionString(
            pDisplayState, pEnteredChars,
            (end_x - start_x) / 2 - MENU_OPTION_FRAME_TEXT_PADDING_X / 2,
            start_y, end_x - textPadding_x, MENU_OPTION_LINE_TEXT_PADDING,
            true, // right justified
            true);

        // Print the abort option frame:
        if (selectionIter == 1) {
            EVE_color_rgb_burst(WHITE);
        } else {
            EVE_color_rgb_burst(GRAY);
        }
        EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
        uint16_t frameStart_y_abort_option =
            frameStart_y + menuOptionFrameHeight + MENU_OPTION_FRAME_SPACING_Y;
        uint16_t menuOptionFrameHeight_abort_option =
            2 * textPadding_y + pCurrentFont->font_caps_height;
        EVE_cmd_dl(VERTEX2F(frameStart_x * 16, frameStart_y_abort_option * 16));
        EVE_cmd_dl(VERTEX2F(
            frameEnd_x * 16,
            (frameStart_y_abort_option + menuOptionFrameHeight_abort_option) *
                16));
        if (selectionIter == 1) {
            EVE_color_rgb_burst(BLACK);
        } else {
            EVE_color_rgb_burst(WHITE);
        }
        int numLinesAbortString = printMenuOptionString(
            pDisplayState, pAbortString, start_x,
            frameStart_y_abort_option + MENU_OPTION_FRAME_TEXT_PADDING_Y,
            (end_x - start_x) / 2 - MENU_OPTION_FRAME_TEXT_PADDING_X / 2,
            MENU_OPTION_LINE_TEXT_PADDING,
            false, // left justified
            true);
        endDisplayList();

        // Wait for a character to be received.
        char receiveChar = 0;
        if (xQueueReceive(*pUartReceiveQueue, &receiveChar,
                          (TickType_t)portMAX_DELAY)) {
            if (receiveChar == 27) {
                // Escape char. read them out of the queue if there are any,
                // Only up or down is available, to select writing or aborting.
                char escapeSeq[3] = {0};
                for (int i = 0; i < 2; i++) {
                    xQueueReceive(*pUartReceiveQueue, &(escapeSeq[i]),
                                  (TickType_t)10);
                }
                escapeSeq[2] = '\0';
                if (strcmp(escapeSeq, "[A") == 0) {
                    // Up
                    if (selectionIter > 0) {
                        selectionIter--;
                    }
                }
                if (strcmp(escapeSeq, "[B") == 0) {
                    // Down
                    if (selectionIter < MAX_NUM_MENU_ITEMS - 1) {
                        selectionIter++;
                    }
                }
            } else if (receiveChar == '\r') {
                // Done, check if the number of bits are OK, in which case
                // change it and exit this function.
                if (selectionIter == 1) {
                    // This is the abort option, abort.
                    return;
                }
                int newBitSize = strtol(pEnteredChars, NULL, 10);
                if (newBitSize <= 64 && newBitSize > 0) {
                    pDisplayState->inputOptions.numBits = newBitSize;
                    return;
                }
            } else if (receiveChar == '\b' || receiveChar == 127) {
                // Backspace, remove the latest added char.
                if (charIter != 0) {
                    pEnteredChars[--charIter] = '\0';
                } else {
                    // Buffer is full, inform.
                    logger(LOGGER_LEVEL_INFO,
                           "updateBitWidth: Could not remove digit, buffer is "
                           "empty\r\n");
                }
            } else if (receiveChar >= '0' && receiveChar <= '9') {
                // Received a number, add it to the bit width.
                if (charIter == 0 && receiveChar == '0') {
                    // Cannot have a leading 0, ignore it.
                    logger(LOGGER_LEVEL_INFO, "updateBitWidth: Leading zeros "
                                              "is not a valid input\r\n");
                } else if (charIter < 2) {
                    pEnteredChars[charIter++] = receiveChar;
                } else {
                    // Buffer is full, inform.
                    logger(LOGGER_LEVEL_INFO,
                           "updateBitWidth: %c could not be added since %s is "
                           "already entered\r\n",
                           receiveChar, pEnteredChars);
                }
            } else {
                logger(LOGGER_LEVEL_INFO,
                       "updateBitWidth: Unrecognized/unhandled input: %c\r\n",
                       receiveChar);
            }
        }
    }
}

void changeFont(displayState_t *pDisplayState, char *pString) {
    // Get the current font index
    uint8_t fontIdx = pDisplayState->fontIdx;
    // Do we have any more fonts?
    if (fontIdx + 1 < MAX_LEN_FONT_LIBRARY_TABLE) {
        // If we increase the font index, will the font table point to 0?
        if (pFontLibraryTable[fontIdx + 1] == NULL) {
            // No more fonts have been programmed, roll around to 0
            pDisplayState->fontIdx = 0;
        } else {
            pDisplayState->fontIdx += 1;
        }
    } else {
        // Maximum font index is reached roll around to 0
        pDisplayState->fontIdx = 0;
    }
}

void goUpOneMenu(displayState_t *pDisplayState, char *pString) {
    // Get the current display option:
    menuState_t *pMenuState = pDisplayState->pMenuState;
    // Move the currentMenuOption to the upper menu option.
    if (pMenuState->pUpperMenu != NULL) {
        pDisplayState->pMenuState = (menuState_t *)pMenuState->pUpperMenu;
    }
}

void exitMenu(displayState_t *pDisplayState, char *pString) {
    pDisplayState->inMenu = false;
}

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
            }
            if (receiveChar == '\r') {
                // This is carriage return.
                // If there is an associated function, run that.
                // else if there is a submenu, enter that
                menuState_t *pMenuState = pLocalDisplayState->pMenuState;
                menuOption_t *pCurrentMenuOption =
                    pMenuState->pCurrentMenuOption;
                menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
                if (pCurrentMenuOption->menuUpdateFun.pUpdateFun != NULL) {
                    // Run the update function
                    // This is a void function with args:
                    // displayState_t *pDisplayState, char *pString,
                    // Where in this case the pString is NULL
                    if (pCurrentMenuOption->menuUpdateFun
                            .interactiveUpdateFun) {
                        // NOTE: This will hijack the display and serial
                        // interface, and return when it's done
                        (*((interactive_menu_function *)(pCurrentMenuOption
                                                             ->menuUpdateFun
                                                             .pUpdateFun)))(
                            pLocalDisplayState, &uartReceiveQueue);
                    } else {
                        (*((non_interactive_menu_function
                                *)(pCurrentMenuOption->menuUpdateFun
                                       .pUpdateFun)))(pLocalDisplayState, NULL);
                    }
                } else if (pCurrentMenuOption->pSubMenu != NULL) {
                    // Enter the sub menu
                    pLocalDisplayState->pMenuState =
                        (menuState_t *)(pCurrentMenuOption->pSubMenu);
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