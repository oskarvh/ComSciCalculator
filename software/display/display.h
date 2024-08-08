/*
MIT License

Copyright (c) 2023 Oskar von Heideken

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

#ifndef DISPLAY_H_
#define DISPLAY_H_

// Operator functions
#include "comscicalc.h"

// Font library
#include "fonts/font.h"
#include "fonts/font_library.h"

// FreeRTOS
#include "FreeRTOS.h"
#include "event_groups.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Standard library
#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------
 * ----------- DEFINES AND MACROS ------------
 * -------------------------------------------*/
// Screen outline:
/* opt1 opt2 opt3
 * --------------------------------------------------------
 *                                                  [input]
 *
 * --------------------------------------------------------
 *                                          [Binary output]
 * --------------------------------------------------------
 *              [Hex output] |             [Decimal output]
 * */

#define DISPLAY_EVENT_NEW_DATA 1
#define DISPLAY_EVENT_CURSOR (1 << 1)

//! Top line which parts the options from the input
#define OUTLINE_WIDTH 2 * 16
/**
 * @defgroup outline Definitions for writing the outline of the screen.
 * @{
 */
// Top outline X start coordinate.
#define TOP_OUTLINE_X0 0
#define TOP_OUTLINE_X1 EVE_HSIZE
#define TOP_OUTLINE_Y (FONT_SIZE_OPTIONS + 2)
// Middle horizontal line that parts the input from results
#define MID_TOP_OUTLINE_X0 0
#define MID_TOP_OUTLINE_X1 EVE_HSIZE
#define MID_TOP_OUTLINE_Y (EVE_VSIZE / 2)
// Lower middle horizontal line that parts binary output from hex and dec
#define MID_LOW_OUTLINE_X0 0
#define MID_LOW_OUTLINE_X1 EVE_HSIZE
#define MID_LOW_OUTLINE_Y (EVE_VSIZE * 3 / 4)
// Vertical line parting the hex and dec results
#define VERT_LOW_OUTLINE_X (EVE_HSIZE / 2)
#define VERT_LOW_OUTLINE_Y0 (EVE_VSIZE * 3 / 4)
#define VERT_LOW_OUTLINE_Y1 (EVE_VSIZE)
/**@}*/

//! Input text X start coordinate
#define INPUT_TEXT_XC0 (EVE_HSIZE - 5)
//! Input text Y coordinate. Will depend on font height
#define INPUT_TEXT_YC0(font_height)                                            \
    (EVE_VSIZE / 2 - font_height - font_height / 2)

//! Decimal output X start coordinate.
#define OUTPUT_DEC_XC0 5 //(EVE_HSIZE - 5)
//! Decimal output Y coordinate, depends on fond height
#define OUTPUT_DEC_YC0(font_height) (EVE_VSIZE * 3 / 4 + font_height / 2)
//! Decimal output maximum X length
#define OUTPUT_DEC_X_LEN (EVE_HSIZE / 2 - 10)

//! Binary output X start coordinate
#define OUTPUT_BIN_XC0 5 //(EVE_HSIZE - 5)
//! Binary Y coordinate, depends on font height
#define OUTPUT_BIN_YC0(font_height) (EVE_VSIZE / 2 + font_height / 2)
//! Binary output maximum X length
#define OUTPUT_BIN_X_LEN (EVE_HSIZE - 10)

//! Hexadecimal X start coordinate
#define OUTPUT_HEX_XC0 (EVE_HSIZE / 2 - 5)
//! Hexadecimal Y coordinate. Depends on font height
#define OUTPUT_HEX_YC0(font_height) (EVE_VSIZE * 3 / 4 + font_height / 2)
//! Hexadecimal output maximum X length
#define OUTPUT_HEX_X_LEN (EVE_HSIZE / 2 - 10)

//! Status bar X offset
#define OUTPUT_STATUS_X0 (5)
//! Status bar Y offset
#define OUTPUT_STATUS_YC0(font_height) (2)
//! Display text options for input buffer
#define INPUT_TEXT_OPTIONS EVE_OPT_RIGHTX
#define FONT 18
#define FONT_SIZE_X 8
#define FONT_SIZE_OPTIONS 20

/**
 * @defgroup ColorDefinitions Definitions for the colors used in the display
 * @{
 */
#define COLORWHEEL_LEN 8
#define RED 0xff0000
#define ORANGE 0xff7f00
#define GREEN 0x00ff00
#define BLUE 0x0000ff
#define BLUE_1 0x5dade2
#define BLUE_2 0x4c7efc
#define YELLOW 0xffff00
#define MAGENTA 0xff00ff
#define PURPLE 0x4b0082 // 0x800080
#define WHITE 0xffffff
#define BLACK 0x000000
#define GRAY 0x303030
#define TURQOISE 0x00fff7
/**@}*/

//! Maximum length of printed input buffer in decimal. Maximum is
//! -9,223,372,036,854,775,808 with a decimal (and \0)
#define MAX_PRINTED_BUFFER_LEN_DEC 21
//! Maximum length of printed input buffer in binary. Maximum is 64 bits with
//! decimal point, null pointer and spaces every 4 bits
#define MAX_PRINTED_BUFFER_LEN_BIN (64 + 2 + 16)
//! Maximum length of printed input buffer in hexadecimal. Maximum looks like 16
//! bits with decimal point but it's 2 extra with decimal (and \0)
#define MAX_PRINTED_BUFFER_LEN_HEX 20
//! Maximum length of results buffer
#define MAX_PRINTED_BUFFER_LEN 100

/**
 * @brief Type to handle the menu options
 */
typedef struct menuOption {
    /**
     * @param pOptionString Pointer to the string for this option
     */
    char *pOptionString;

} menuOption_t;

/**
 * @brief Struct holding the state of the options shown
 * at the top of the screen
 * This is populated by the calculator core thread.
 */
typedef struct inputState {
    /**
     * @param currentInputArith The current input arithmetic type,
     * i.e. float, fixed or none.
     * Inherits the types from the operators.
     */
    uint8_t currentInputArith;
    /**
     * @param currentInputBase The base in which the input
     * is given in, i.e. bin, dec or hex. Type inherited from
     * operators.
     */
    uint8_t currentInputBase;

} inputState_t;

/**
 * @brief Struct holding the menu state.
 */
typedef struct menuState {
    /**
     * @param pMenuOptionList Pointer to the list of menu options
     */
    const menuOption_t *pMenuOptionList;
    /**
     * @param currentItemIndex Index of the current menu item
     */
    uint8_t currentItemIndex;
} menuState_t;

/**
 * @brief Struct holding the display state, along with calculator
 * results and user input states which should be displayed.
 * This script and the underlying structs are populated
 * by the calculator core thread.
 */
typedef struct displayState {
    /**
     * @param result Copy of the calculated result
     */
    SUBRESULT_INT result;
    /**
     * @param inputOptions Contains information regarding the
     * options currently active for the input format.
     */
    numberFormat_t inputOptions;
    /**
     * @param solveStatus This variable is the output from the
     * calculator solver.
     */
    calc_funStatus_t solveStatus;
    /**
     * @param printStatus Status of the calculator core print
     * to string function, which is used to print the input buffer
     */
    calc_funStatus_t printStatus;
    /**
     * @param fontIdx Current font identifier
     */
    uint8_t fontIdx;
    /**
     * @param syntaxIssueIndex Index to the first char in the input
     * string that has a syntax issue
     */
    int16_t syntaxIssueIndex;
    /**
     * @param cursorLoc Cursor location, copied from the corestate
     */
    uint8_t cursorLoc;
    /**
     * @param inMenu Boolean to indicate if we're currently in the menu
     */
    bool inMenu;
    /**
     * @param pMenuState Pointer to menu state.
     */
    menuState_t *pMenuState;
    /**
     * @param printedInputBuffer Input buffer printed by calc core.
     */
    char printedInputBuffer[MAX_PRINTED_BUFFER_LEN];

} displayState_t;

/* -------------------------------------------
 * ----------- EXTERNAL VARIABLES ------------
 * -------------------------------------------*/
extern displayState_t displayState;
extern xSemaphoreHandle displayStateSemaphore;
extern EventGroupHandle_t displayTriggerEvent;

/* -------------------------------------------
 * ---------- FUNCTION PROTOTYPES ------------
 * -------------------------------------------*/
/**
 * @brief Initialize the display state
 * @param pDisplayState Pointer to the display state
 * @return Nothing
 */
void initDisplayState(displayState_t *pDisplayState);

/**
 * @brief Task that handles the display
 * @param p Pointer to the task argument. Not used for now
 * @return Nothing
 */
void displayTask(void *p);

void testDisplay();
#endif /* DISPLAY_H_ */
