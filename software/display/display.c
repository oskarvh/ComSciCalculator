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

#include "display.h"
#include "EVE.h"
#include "print_utils.h"
#include "uart_logger.h"

//! Binary result string buffer
char pBinRes[MAX_PRINTED_BUFFER_LEN_BIN] = {0};
//! Hexadecimal result string buffer
char pHexRes[MAX_PRINTED_BUFFER_LEN_HEX] = {0};
//! Decimal result string buffer
char pDecRes[MAX_PRINTED_BUFFER_LEN_DEC] = {0};
//! Color wheel for the brackets
const uint32_t colorWheel[COLORWHEEL_LEN] = {WHITE,    ORANGE, YELLOW,  GREEN,
                                             TURQOISE, BLUE_2, MAGENTA, PURPLE};
//! String for displaying the decimal base to the user on the screen
char dec_display_string[] = "DEC";
//! String for displaying the hexadecimal base to the user on the screen
char hex_display_string[] = "HEX";
//! String for displaying the binary base to the user on the screen
char bin_display_string[] = "BIN";
//! Array of pointers to the strings used to show which base is active on the
//! screen.
const char *baseDisplayStrings[] = {
    [inputBase_DEC] = dec_display_string,
    [inputBase_HEX] = hex_display_string,
    [inputBase_BIN] = bin_display_string,
};

//! String for displaying the integer format to the user
char int_display_string[] = "INT";
//! String for displaying the fixed point format to the user.
char fixed_display_string[] = "FIXED";
//! String for displaying the floating point format to the user
char float_display_string[] = "FLOAT";
//! Array of pointers to the strings used to show which format is active on the
//! screen.
const char *formatDisplayStrings[] = {
    [INPUT_FMT_INT] = int_display_string,
    [INPUT_FMT_FIXED] = fixed_display_string,
    [INPUT_FMT_FLOAT] = float_display_string,
};

/**
 * @brief Print the outline of graphics
 *
 * This attempts to solve the current buffer and reflect the result
 * in the #pCalcCoreState.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of solving the buffer.
 */
static void displayOutline(void) {
    // Outline shall be white
    EVE_color_rgb_burst(WHITE);
    // Write the top line. This parts the options from the input
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X0 * 16, TOP_OUTLINE_Y * 16));
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X1 * 16, TOP_OUTLINE_Y * 16));
    // Write middle line parting input and binary
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X0 * 16, MID_TOP_OUTLINE_Y * 16));
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X1 * 16, MID_TOP_OUTLINE_Y * 16));
    // Lower middle outline parting binary and hex/dec
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X0 * 16, MID_LOW_OUTLINE_Y * 16));
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X1 * 16, MID_LOW_OUTLINE_Y * 16));
    // Vertical line parting hex and dec results
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X * 16, VERT_LOW_OUTLINE_Y0 * 16));
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X * 16, VERT_LOW_OUTLINE_Y1 * 16));
}

/**
 * @brief Starts the display list, clear local buffers and clears color buffers
 */
static void startDisplaylist(void) {
    EVE_start_cmd_burst();
    EVE_cmd_dl_burst(CMD_DLSTART);
    EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | BLACK);
    EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
}

/**
 * @brief End the display list by sending display and swap DL
 */
static void endDisplayList(void) {
    EVE_cmd_dl_burst(DL_DISPLAY);
    EVE_cmd_dl_burst(CMD_SWAP);
    EVE_end_cmd_burst();
    // Wait until EVE is not busy anymore to ensure conclusion of command.
    while (EVE_busy())
        ;
}

//! Offset used when programming custom fonts into RAM_G
static uint32_t ram_g_address_offset = 0;
/**
 * @brief Program font to the FT81x
 * @param pFont Pointer to the font struct
 * @param fontIndex The index in FT81x this font will get
 * @return Nothing
 * @warning This function assumes that the first char is a space.
 * If this is not the case, the setfont2 functions last parameter
 * must be changed.
 */
void programFont(font_t *pFont, uint8_t fontIndex) {
    uint32_t thisFontsAddress = EVE_RAM_G + ram_g_address_offset;
    // Null check the pointer
    if (pFont == NULL) {
        return;
    }
    // If this is a ROM font, or of the pointer to the table is NULL, then
    // return
    if (pFont->rom_font == true || pFont->pFontTable == NULL) {
        return;
    }
    // Write the bitmap of the pFont to the graphics RAM if there is room.
    if (ram_g_address_offset + pFont->fontTableSize < EVE_RAM_G_SIZE) {
        // Program the font
        EVE_memWrite_sram_buffer(EVE_RAM_G + ram_g_address_offset,
                                 pFont->pFontTable, pFont->fontTableSize);
        // Program the offset for the font.
        // This is at offset 144 in the font table,
        // and is per default set to 148, which is the offset
        // of the font to the start of the raw data.
        // However, if programming more than 1 font, this offset
        // needs to change, as it's the offset to the start of RAM_G
        // Hence, it should be the ram_g_address_offset
        uint32_t offsetInRam = ram_g_address_offset + 148;
        EVE_memWrite_sram_buffer(EVE_RAM_G + ram_g_address_offset + 144,
                                 (uint8_t *)&offsetInRam, 4);
        ram_g_address_offset += pFont->fontTableSize;
        // Round up to the nearest four byte aligned address:
        uint32_t offset = ram_g_address_offset % 4;
        ram_g_address_offset += offset;
    } else {
        logger(LOGGER_LEVEL_ERROR, "\r\nERROR: Font not programmed!\r\n\r\n");
        return;
    }
    // This must be in a display list to register the new font
    startDisplaylist();
    // CMD_SETBITMAP not mandatory if using setfont2
    EVE_cmd_setfont2_burst(fontIndex, thisFontsAddress, 32);

    endDisplayList();
}

/**
 * @brief Programs the font library
 * @return None
 */
void programFontLibrary(void) {
    // Go through all entries in the font library,
    // and program the large font at the even indexes, followed by the
    // small font add the odd ones.
    for (uint8_t i = 0; i < MAX_LEN_FONT_LIBRARY_TABLE; i++) {
        if (pFontLibraryTable[i] != NULL) {
            // Program the large index:
            programFont(pFontLibraryTable[i]->pLargeFont, i * 2);
            pFontLibraryTable[i]->pLargeFont->ft81x_font_index = i * 2;
            programFont(pFontLibraryTable[i]->pSmallFont, i * 2 + 1);
            pFontLibraryTable[i]->pSmallFont->ft81x_font_index = i * 2 + 1;
        }
    }
}
/**
 * @brief Find the width of the font, in pixels
 * @param pFont Pointer to the font struct
 * @param c Character to find the width of
 * @return Width of the current char at that font in pixels
 * @warning This function assumes that the first char is a space,
 * and follows the ASCII char setup
 */
uint8_t getFontCharWidth(font_t *pFont, char c) {
    // The font table contains a LUT for the char widths,
    // so using the char as an index, the width is simply the
    // value at that index. Conveniently, it's the first data
    // in the struct
    if (!pFont->rom_font) {
        return pFont->pFontTable[(uint8_t)c];
    } else {
        // TODO!!
        return pFont->font_x_width;
    }
}

/**
 * @brief Print the calculator state to the top of the screen.
 *        This function displays the arithmetic used (float, int, fixed),
 *        the base (hex, dec, bin), the bit length (0-64), and anything else
 *        that might be useful.
 * @param pRxBuf Pointer to the input string
 * @param syntaxErrorIndex Index
 * @return Nothing
 */
void displayCalcState(displayState_t *pDisplayState) {
    // Get the string and length depending on the base:
    char *pBaseString =
        baseDisplayStrings[pDisplayState->inputOptions.inputBase];
    uint8_t baseStringLen = strlen(pBaseString);

    // Get the string and length depending on the input format:
    char *pInputFormatString =
        formatDisplayStrings[pDisplayState->inputOptions.inputFormat];
    uint8_t inputFormatStringLen = strlen(pInputFormatString);

    // Display the bit width. Note: for fixed point, it's shown in Q notation
    char bitWidthString[7] = {0}; // Worst case scenario is 100.28\0
    if (pDisplayState->inputOptions.inputFormat == INPUT_FMT_FIXED) {
        // Fixed point require Q notation.
        uint8_t numBits = pDisplayState->inputOptions.numBits;
        uint8_t decimalBits =
            pDisplayState->inputOptions.fixedPointDecimalPlace;
        // Work out the Q notation:
        uint8_t integerBits = numBits - decimalBits;
        sprintf(bitWidthString, "%u.%u", integerBits, decimalBits);
    } else {
        // Just get the bit width as int and convert to string
        sprintf(bitWidthString, "%u", pDisplayState->inputOptions.numBits);
    }

    // Get the string and length depending on the output format:
    char *pOutputFormatString =
        formatDisplayStrings[pDisplayState->inputOptions.outputFormat];
    uint8_t outputFormatStringLen = strlen(pOutputFormatString);

    /*
    // Display the bit width. Note: for fixed point, it's shown in Q notation
    // Ignore the output format bit length for now. Since it's shared with the
    input. char outputBitWidthString[7] = {0}; // Worst case scenario is
    100.28\0 if(pDisplayState->inputOptions.inputFormat == INPUT_FMT_FIXED){
        // Fixed point require Q notation.
        uint8_t numBits = pDisplayState->inputOptions.numBits;
        uint8_t decimalBits =
    pDisplayState->inputOptions.fixedPointDecimalPlace;
        // Work out the Q notation:
        uint8_t integerBits = numBits-decimalBits;
        sprintf(outputBitWidthString, "%u.%u", integerBits, decimalBits);
    } else {
        // Just get the bit width as int and convert to string
        sprintf(outputBitWidthString, "%u",
    pDisplayState->inputOptions.numBits);
    }
    */

    // Find the maximum length of string we can print to the screen.
    // This has to be based on the font.
    // uint16_t maxDisplayStrLen =
    char pStatusString[60] = {0};
    sprintf(pStatusString, "%s  BITS:%s  INPUT:%s  OUTPUT:%s\0", pBaseString,
            bitWidthString, pInputFormatString, pOutputFormatString);
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pSmallFont;
    EVE_cmd_text_burst(OUTPUT_STATUS_X0,
                       OUTPUT_STATUS_YC0(pCurrentFont->font_caps_height),
                       pCurrentFont->ft81x_font_index, 0, pStatusString);
}

/**
 * @brief Print the input buffer to the screen
 * @param pDisplayState Pointer to the display states
 * @param writeCursor Cursor location
 * @return Nothing
 */
//! Screen left and right buffer area for input
#define VISIBLE_INPUT_X_BUFFER (5)
//! How many horizontal pixels are allocated for the input area
#define VISIBLE_INPUT_X_AREA_PX (EVE_HSIZE - VISIBLE_INPUT_X_BUFFER)
void displayInputText(displayState_t *pDisplayState, bool writeCursor) {

    uint8_t colorWheelIndex = 0; // Maximum COLORWHEEL_LEN

    // If syntaxIssueIndex = -1 then there are no syntax errors.
    // Iterate through each char until null pointer
    uint16_t charIter = 0;
    // Get the current font
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
    // Width of the cumulative characters now written
    uint32_t widthWrittenChars = 0;

    // Calculate if the input display should be wrapped, and by how many lines
    uint32_t widthAllChars = VISIBLE_INPUT_X_BUFFER;
    uint16_t numLinesWrap = 0;
    while (pDisplayState->printedInputBuffer[charIter] != '\0') {
        widthAllChars += getFontCharWidth(
            pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
        // If the current line width is larger than visible area, then that
        // means a new line should be made
        if (widthAllChars >= VISIBLE_INPUT_X_AREA_PX) {
            widthAllChars =
                VISIBLE_INPUT_X_BUFFER +
                getFontCharWidth(pCurrentFont,
                                 pDisplayState->printedInputBuffer[charIter]);
            numLinesWrap++;
        }
        charIter++;
    }
    charIter = 0;                  // Reset before using again.
    uint8_t displayWrapOffset = 0; // Track how many lines have been written.
    uint16_t currentLineWidth =
        VISIBLE_INPUT_X_BUFFER; // Tracks the current line width
    while (pDisplayState->printedInputBuffer[charIter] != '\0') {
        // Increase color index if opening bracket
        if (pDisplayState->printedInputBuffer[charIter] == '(') {
            colorWheelIndex++;
        }
        uint32_t color = colorWheel[colorWheelIndex % COLORWHEEL_LEN];

        if (charIter >= ((uint16_t)(pDisplayState->syntaxIssueIndex))) {
            color = RED;
        }
        // Print the current character
        EVE_cmd_dl_burst(DL_COLOR_RGB | color);

        // Add the width of the char to be written:
        widthWrittenChars += getFontCharWidth(
            pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
        currentLineWidth += getFontCharWidth(
            pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
        // If the current line width is larger than visible area, then that
        // means a new line should be made
        if (currentLineWidth >= VISIBLE_INPUT_X_AREA_PX) {
            currentLineWidth =
                VISIBLE_INPUT_X_BUFFER +
                getFontCharWidth(pCurrentFont,
                                 pDisplayState->printedInputBuffer[charIter]);
            displayWrapOffset++;
        }
        // Have a temporary buffer to be able to print in different colors. Null
        // terminated
        char pTmpRxBuf[2] = {pDisplayState->printedInputBuffer[charIter], '\0'};

        uint32_t yOffset = INPUT_TEXT_YC0(pCurrentFont->font_caps_height) -
                           (numLinesWrap - displayWrapOffset) *
                               (pCurrentFont->font_caps_height + 5);
        // Print one colored char
        EVE_cmd_text_burst(
            currentLineWidth,
            yOffset, // INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
            pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pTmpRxBuf);

        // Decrease color index if closing bracket.
        if (pDisplayState->printedInputBuffer[charIter] == ')') {
            colorWheelIndex--;
        }
        charIter++;
    }
    if (writeCursor) {
        displayWrapOffset = 0;
        // Get the width of the chars until the current cursor
        uint32_t widthWrittenCharsUntilCursor = VISIBLE_INPUT_X_BUFFER;
        for (int i = 0; i < charIter - pDisplayState->cursorLoc; i++) {
            widthWrittenCharsUntilCursor += getFontCharWidth(
                pCurrentFont, pDisplayState->printedInputBuffer[i]);
            // Reset if width of the screen has been reached.
            if (widthWrittenCharsUntilCursor >= VISIBLE_INPUT_X_AREA_PX) {
                widthWrittenCharsUntilCursor =
                    VISIBLE_INPUT_X_BUFFER +
                    getFontCharWidth(pCurrentFont,
                                     pDisplayState->printedInputBuffer[i]);
                displayWrapOffset++;
            }
        }
        // Cursor is always white
        EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
        // Write cursor
        uint32_t yOffset = INPUT_TEXT_YC0(pCurrentFont->font_caps_height) -
                           (numLinesWrap - displayWrapOffset) *
                               (pCurrentFont->font_caps_height + 5);
        EVE_cmd_text_burst(widthWrittenCharsUntilCursor +
                               getFontCharWidth(pCurrentFont, ' ') / 2,
                           yOffset, pCurrentFont->ft81x_font_index,
                           INPUT_TEXT_OPTIONS, "|");
    }
}

void initDisplayState(displayState_t *pDisplayState) {
    pDisplayState->inputOptions.fixedPointDecimalPlace = 32; // TBD
    pDisplayState->inputOptions.inputFormat = 0;             // TBD
    pDisplayState->inputOptions.outputFormat = 0;            // TBD
    pDisplayState->inputOptions.inputBase = 0;               // TBD
    pDisplayState->inputOptions.numBits = 0;                 // TBD
    pDisplayState->inputOptions.sign = 0;                    // TBD
    pDisplayState->solveStatus = calc_solveStatus_INPUT_LIST_NULL;
    pDisplayState->printStatus = 0;
    pDisplayState->fontIdx = 0; // Try the custom RAM font
    memset(pDisplayState->printedInputBuffer, '\0', MAX_PRINTED_BUFFER_LEN);
    pDisplayState->syntaxIssueIndex = -1;
}

void printResult(displayState_t *pDisplayState) {
    // Get the result and output formats
    SUBRESULT_INT result = pDisplayState->result;

    // Clear the buffers
    memset(pDecRes, 0, MAX_PRINTED_BUFFER_LEN_DEC);
    memset(pBinRes, 0, MAX_PRINTED_BUFFER_LEN_BIN);
    memset(pHexRes, 0, MAX_PRINTED_BUFFER_LEN_HEX);

    // Convert to each base
    convertResult(pDecRes, result, &(pDisplayState->inputOptions),
                  inputBase_DEC);
    convertResult(pBinRes, result, &(pDisplayState->inputOptions),
                  inputBase_BIN);
    convertResult(pHexRes, result, &(pDisplayState->inputOptions),
                  inputBase_HEX);

    // Get the current font:
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
    EVE_cmd_text_burst(
        OUTPUT_DEC_XC0, OUTPUT_DEC_YC0(pCurrentFont->font_caps_height),
        pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pDecRes);
    EVE_cmd_text_burst(
        OUTPUT_BIN_XC0, OUTPUT_BIN_YC0(pCurrentFont->font_caps_height),
        pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pBinRes);
    EVE_cmd_text_burst(
        OUTPUT_HEX_XC0, OUTPUT_HEX_YC0(pCurrentFont->font_caps_height),
        pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pHexRes);
}

// HACK: temporary function just to check that the display works
void displaySanityCheck() {
    startDisplaylist();
    displayOutline();
    endDisplayList();
}

// TODO:
// 1. Support different color output + freeze output if
//    calculation was unsuccessful. DONE
// 2. Support non-monospaced fonts (it's possible to find the
//    spacing from the font meta data) DONE
// 3. Support multiple fonts. DONE
// 3. Support blinking cursor.
//      Blinking done but I need the font spacing to be completed before I can
//      set this as done.DONE
// 4. For non-monospaced operators (e.g. SUM), the cursor is at the incorrect
// location.
void displayTask(void *p) {

    // Program the font library
    programFontLibrary();

    // Update the screen to begin with
    bool updateScreen = true;
    bool writeCursor = true;
    displayState_t localDisplayState;
    initDisplayState(&localDisplayState);

    // Write the outlines:
    startDisplaylist();
    displayOutline();
    displayCalcState(&localDisplayState);
    endDisplayList();

    while (1) {
        // Wait for the trigger event from the calculator task to tell this task
        // to update the display, since new data is available and ready.
        // Both updating the cursor and updating the values on screen shall
        // trigger an event.
        uint32_t eventbits =
            xEventGroupWaitBits(displayTriggerEvent,
                                (DISPLAY_EVENT_CURSOR | DISPLAY_EVENT_NEW_DATA),
                                pdTRUE, pdFALSE, portMAX_DELAY);
        if (eventbits & DISPLAY_EVENT_CURSOR) {
            // Invert the cursor to blink it.
            writeCursor = !writeCursor;
        }
        if (eventbits & DISPLAY_EVENT_NEW_DATA) {
            // If new data is available, then set the cursor to true
            writeCursor = true;
        }

        // Wait (forever) for the semaphore to be available.
        if (xSemaphoreTake(displayStateSemaphore, portMAX_DELAY)) {
            // To save time, copy the display state. Sort of waste of space. Not
            // sure if this is the best approach..
            memcpy(&localDisplayState, &displayState, sizeof(displayState_t));
            // Release the semaphore, since we're done with the display state
            // global variable
            xSemaphoreGive(displayStateSemaphore);
            // Trigger a screen update
            // updateScreen = true;
        }
        // if(updateScreen){
#ifdef PRINT_RESULT_TO_UART
#ifdef VERBOSE
#error Cannot print result to UART and have VERBOSE UART logging at the same time!
#else
        // Print the results to UART
        UARTprintf("IN: [%s]\r\n", pRxBuf);
        UARTprintf("DEC: [%s]\r\n", pDecRes);
        UARTprintf("BIN: [%s]\r\n", pBinRes);
        UARTprintf("HEX: [%s]\r\n", pHexRes);
#endif
#endif
        // The result is now in calcResult, print to the different types
        sprintf(pDecRes, "%1i", localDisplayState.result);
        printToBinary(pBinRes, localDisplayState.result, false,
                      localDisplayState.inputOptions.numBits);
        sprintf(pHexRes, "0x%1X", localDisplayState.result);
        // Update the screen:
        startDisplaylist();
        displayOutline();
        // Write the calculator setting state:
        displayCalcState(&localDisplayState);
        // Write the input text
        displayInputText(&localDisplayState, writeCursor);
        // EVE_cmd_text_burst(INPUT_TEXT_XC0, INPUT_TEXT_YC0, FONT,
        // INPUT_TEXT_OPTIONS, pRxBuf);

        // Let the color reflect if the operation was OK or not.
        if (localDisplayState.solveStatus == calc_solveStatus_SUCCESS) {
            EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
        } else {
            EVE_cmd_dl_burst(DL_COLOR_RGB | GRAY);
        }
        printResult(&localDisplayState),

            endDisplayList();
        // Screen has been updated, set update variable to false
        // updateScreen = false;
        //}
    }
}
