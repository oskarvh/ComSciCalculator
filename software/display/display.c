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

// stdlib
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "EVE.h"
#include "display.h"
#include "firmware_common.h"
#include "menu.h"
#include "print_utils.h"
#include "uart_logger.h"

//! Binary result string buffer
char pBinRes[MAX_PRINTED_BUFFER_LEN_BIN] = {0};
//! Hexadecimal result string buffer
char pHexRes[MAX_PRINTED_BUFFER_LEN_HEX] = {0};
//! Decimal result string buffer
char pDecRes[MAX_PRINTED_BUFFER_LEN_DEC] = {0};
//! Color wheel for the brackets
const uint32_t colorWheel[COLORWHEEL_LEN] = {WHITE,  YELLOW,  GREEN, TURQOISE,
                                             BLUE_2, MAGENTA, PURPLE};
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
 * @return Nothing
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
        // TODO: How to find the width of a ROM font?
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
    const char *pBaseString =
        baseDisplayStrings[pDisplayState->inputOptions.inputBase];
    uint8_t baseStringLen = strlen(pBaseString);

    // Get the string and length depending on the input format:
    const char *pInputFormatString =
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
    const char *pOutputFormatString =
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
    if (pDisplayState == NULL) {
        while (1)
            ;
    }

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
    pDisplayState->inMenu = false;
    pDisplayState->pMenuState = &topMenu;
}

/**
 * @brief Function to print
 * @param pString Pointer to string to be printed
 * @param yStart Y start pixel
 * @param xStart X start pixel
 * @param xMax Maximum length of bounding box
 * @param pFont Pointer to font
 * @return Nothing
 */
static void displayResultWithinBounds(char *pString, uint16_t yStart,
                                      uint16_t xStart, uint16_t xMax,
                                      font_t *pFont) {

    // Iterate through each char until null pointer
    uint16_t charIter = 0;
    // Width of the cumulative characters now written
    uint32_t widthWrittenChars = 0;

    // Calculate if the input display should be wrapped, and by how many lines
    uint32_t widthAllChars = VISIBLE_INPUT_X_BUFFER;
    uint16_t numLinesWrap = 0;
    while (pString[charIter] != '\0') {
        widthAllChars += getFontCharWidth(pFont, pString[charIter]);
        // If the current line width is larger than visible area, then that
        // means a new line should be made
        if (widthAllChars >= xMax) {
            widthAllChars = VISIBLE_INPUT_X_BUFFER +
                            getFontCharWidth(pFont, pString[charIter]);
            numLinesWrap++;
        }
        charIter++;
    }

    uint8_t displayWrapOffset = 0; // Track how many lines have been written.
    uint16_t currentLineWidth =
        VISIBLE_INPUT_X_BUFFER; // Tracks the current line width

    while (charIter-- > 0) {

        // Add the width of the char to be written:
        uint8_t currentCharWidth = getFontCharWidth(pFont, pString[charIter]);
        widthWrittenChars += currentCharWidth;
        currentLineWidth += currentCharWidth;
        // If the current line width is larger than visible area, then that
        // means a new line should be made
        if (currentLineWidth >= xMax) {
            currentLineWidth = VISIBLE_INPUT_X_BUFFER + currentCharWidth;
            displayWrapOffset++;
        }
        // Have a temporary buffer to be able to print in different colors. Null
        // terminated
        char pTmpRxBuf[2] = {pString[charIter], '\0'};

        uint32_t yOffset = yStart + (numLinesWrap - displayWrapOffset) *
                                        (pFont->font_caps_height + 5);
        // Print one colored char
        EVE_cmd_text_burst(
            xStart + xMax - currentLineWidth + currentCharWidth,
            yOffset, // INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
            pFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pTmpRxBuf);
    }
}

void printResult(displayState_t *pDisplayState) {
    // Let the color reflect if the operation was OK or not.
    if (pDisplayState->solveStatus == calc_solveStatus_SUCCESS) {
        EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
    } else {
        EVE_cmd_dl_burst(DL_COLOR_RGB | GRAY);
    }
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
    displayResultWithinBounds(pDecRes,
                              OUTPUT_DEC_YC0(pCurrentFont->font_caps_height), 5,
                              OUTPUT_DEC_X_LEN, pCurrentFont);
    displayResultWithinBounds(pBinRes,
                              OUTPUT_BIN_YC0(pCurrentFont->font_caps_height), 5,
                              OUTPUT_BIN_X_LEN, pCurrentFont);
    displayResultWithinBounds(pHexRes,
                              OUTPUT_HEX_YC0(pCurrentFont->font_caps_height),
                              OUTPUT_HEX_XC0, OUTPUT_HEX_X_LEN, pCurrentFont);
}

/**
 * @brief Calculate the number of lines it would take to
 *        print a string given a max width with the current font.
 *        If one word is longer than the max width, then it would assume that
 *        the word continues on the next line.
 *        A word is separated by any non-alfabetical characters
 * @param pDisplayState Pointer to displayState
 * @param pString Pointer to string, maximum length is 255
 * @param maxWidth Maximum width of the string in pixels
 * @param xOffset Offset in width between the bounding box
 * @return Number of lines that would require to print string.
 */
int getNumberOfLinesForString(displayState_t *pDisplayState, char *pString,
                              int maxWidth) {
    // Before any chars have been written, the width is the offset.
    int charsWidth = 0;
    // Init the counters
    int charIter = 0;
    int numLines = 1;
    // Get the currently selected font
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
    while (pString[charIter] != '\0') {
        // Check if the next word fits on this line
        int tmpCharIter = charIter;
        int wordWidth = 0;
        // Lookahead and print the next word
        int endCharIter = charIter;
        while (isalpha(pString[endCharIter])) {
            wordWidth += getFontCharWidth(pCurrentFont, pString[endCharIter++]);
        }
        // Check if the word combined with the previously written chars are
        // longer than the line width
        if (charsWidth + wordWidth > maxWidth) {
            // Check if the word only is longer than the line width
            while (wordWidth > maxWidth) {
                // Get the string that fits the line and print that
                int tmpWordWidth = 0;
                int tmpEndCharIter = charIter;
                while (tmpWordWidth < maxWidth &&
                       tmpEndCharIter < endCharIter) {
                    tmpWordWidth += getFontCharWidth(pCurrentFont,
                                                     pString[tmpEndCharIter++]);
                }
                tmpEndCharIter--;
                numLines += 1;
                charsWidth = 0;
                wordWidth -= tmpWordWidth;
                charIter = tmpEndCharIter;
                if (wordWidth > 0) {
                    numLines += 1;
                }
            }
            // Check if the remaining word plus the
            if (charsWidth + wordWidth > maxWidth) {
                charsWidth += wordWidth;
                charIter = endCharIter;
                numLines += 1;
            }
        }
        // Create a temporary buffer for the current character
        char pTmpRxBuf[2] = {pString[charIter], '\0'};
        int currentCharsWidth =
            getFontCharWidth(pCurrentFont, pString[charIter]);
        // Increase the written character width, and increase the iterator
        charsWidth += currentCharsWidth;
        charIter++;
    }
    return numLines;
}

/**
 * @brief Calculate the
 * @param pString Pointer to string to calculate width for
 * @param pFont Pointer to font to calculate the width for
 * @return Width in pixels that the word would need to print
 */
void printLine(char *pString, font_t *pCurrentFont, int wordWidth,
               int charsWidth, int numLines, int xOffset, int yOffset, int y1,
               int textHeight, bool leftJustification) {
    if (leftJustification) {
        EVE_cmd_text_burst(EVE_HSIZE - wordWidth + charsWidth - xOffset,
                           y1 + yOffset + (numLines - 1) * (textHeight),
                           pCurrentFont->ft81x_font_index, 0, pString);
    } else {
        EVE_cmd_text_burst(xOffset + charsWidth,
                           y1 + yOffset + (numLines - 1) * (textHeight),
                           pCurrentFont->ft81x_font_index, 0, pString);
    }
}

/**
 * @brief Print a string within a bounding box with a symmetrical offset
 * @param pDisplayState Pointer to displayState
 * @param pString Pointer to string, maximum length is 255
 * @param x1 X-coordinate of the right upper corner of the bounding box
 * @param y1 Y-coordinate of the right upper corner of the bounding box
 * @param x2 X-coordinate of the left bottom corner of the bounding box
 * @param y2 Y-coordinate of the left bottom corner of the bounding box
 * @param xOffset X-offset to add to the characters, from the edge of the screen
 * @param yOffset Y-offset from the bounding box, in pixels
 * @param textHeight Height of the text between one line to the next
 * @param leftJustification True if text is coming from the left, false if
 * writing from the right
 * @return Number of lines that would require to print string.
 */
int printMenuOptionString(displayState_t *pDisplayState, char *pString, int x1,
                          int y1, int x2, int y2, int xOffset, int yOffset,
                          int textHeight, bool leftJustification, bool print) {
    // Write line by line, evaluating the next word.

    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
    int charIter = 0;
    int writtenCharsWidth = 0;
    int maxWidth = x2 - x1 - 2 * xOffset;
    int numLines = 0;
    // Loop until the character is null
    while (pString[charIter] != '\0') {
        // Get the length of the next word:
        bool writeLine = false;
        // Find the first non-whitespace char:
        while (isspace(pString[charIter])) {
            charIter++;
        }
        int endStringIter = charIter;
        int stringStartIter = charIter;
        // Loop until it's time to write a line to the screen.
        while (!writeLine) {
            int nextWordWidth = 0;
            if (isalnum(pString[charIter])) {
                // This character is a letter/number, chances are the next ones
                // are as well:
                while (isalnum(pString[endStringIter])) {
                    nextWordWidth += getFontCharWidth(pCurrentFont,
                                                      pString[endStringIter++]);
                }
            } else {
                // Current char is not alfanumeric, meaning the next word width
                // is the width of the char
                if (pString[charIter] == '\0' || pString[charIter] == '\n') {
                    // This is the end of the string, break and print.
                    writeLine = true;
                    break;
                }
                nextWordWidth =
                    getFontCharWidth(pCurrentFont, pString[charIter]);
                endStringIter++;
            }
            // The word between charIter and endStringIter is the next word.
            // Check if it fits on this line, given the width of the characters
            // already written
            if (writtenCharsWidth + nextWordWidth > maxWidth) {
                // Is the word itself longer than the line?
                while (nextWordWidth > maxWidth) {
                    // This word is longer than the max width of the
                    // bounding box. If there are charachters already pipelined
                    // for writing, break and print those and come back here
                    // next time around.
                    if (writtenCharsWidth > 0) {
                        break;
                    }
                    // Here, we know that it's for sure a new line.
                    // We need to splice the word up, and find the end iterator
                    // for where the word would spill over.
                    int splitNextWordWidth = 0;
                    int splitNextWordIter = stringStartIter;
                    int currentCharWidth = 0;
                    while (splitNextWordWidth < maxWidth) {
                        currentCharWidth = getFontCharWidth(
                            pCurrentFont, pString[splitNextWordIter]);
                        splitNextWordWidth += currentCharWidth;
                        splitNextWordIter++;
                    }
                    // Disregard the last character, since it makes the string
                    // wider than the limit
                    splitNextWordWidth -= currentCharWidth;
                    splitNextWordIter -= 1;
                    // Here, we need to print the word between stringStartIter
                    // and splitNextWordIter, increase the line before and
                    // after.
                    int wordLen = splitNextWordIter - stringStartIter;
                    numLines += 1;
                    if (print) {
                        char *pTmpBuf = malloc(sizeof(char) * (wordLen + 1));
                        // Set all to null chars to terminate
                        memset(pTmpBuf, '\0', wordLen + 1);
                        memcpy(pTmpBuf, &(pString[stringStartIter]),
                               sizeof(char) * wordLen);
                        printLine(pTmpBuf, pCurrentFont, splitNextWordWidth, 0,
                                  numLines, xOffset, yOffset, y1, textHeight,
                                  leftJustification);
                        free(pTmpBuf);
                    }
                    // We're now at a new line, so set the
                    // next word width minus what we've written here
                    nextWordWidth -= splitNextWordWidth;
                    // A new line with no chars has been written,
                    // so reset the written chars width:
                    writtenCharsWidth = 0;
                    // Set the character iterator to the last written character
                    charIter = splitNextWordIter;
                    stringStartIter = splitNextWordIter;
                }
                // Retest, since we might have shaved off some letters
                // in the while loop before this
                if (writtenCharsWidth + nextWordWidth > maxWidth) {
                    // Here, the combination of written characters
                    // and the new word would exceed the width of the line.
                    // Meaning that we'd want to print the line.
                    writeLine = true;
                    // Set the end character iterator to what was before this
                    // current char.
                    endStringIter = charIter;
                    // Remove any trailing spaces.
                    while (!isalnum(pString[endStringIter - 1])) {
                        writtenCharsWidth -= getFontCharWidth(
                            pCurrentFont, pString[endStringIter--]);
                    }
                }
            }
            if (writtenCharsWidth + nextWordWidth <= maxWidth) {
                // we can fit the current word on this line.
                // Add it and move on to the next character
                writtenCharsWidth += nextWordWidth;
                charIter = endStringIter;
            }
            // If the character after the end of this substring is null, then
            // print.
            if (pString[charIter] == '\0' || pString[charIter] == '\n') {
                writeLine = true;
            }
        }
        // Here, we've either breaked due to some error, or it's time to write a
        // line
        if (writeLine) {
            // Increase the line number
            numLines += 1;
            if (print) {
                int wordLen = endStringIter - stringStartIter;
                char *pTmpBuf = malloc(sizeof(char) * (wordLen + 1));
                // Set all to null chars to terminate
                memset(pTmpBuf, '\0', wordLen + 1);
                memcpy(pTmpBuf, &(pString[stringStartIter]),
                       sizeof(char) * wordLen);
                printLine(pTmpBuf, pCurrentFont, writtenCharsWidth, 0, numLines,
                          xOffset, yOffset, y1, textHeight, leftJustification);
                free(pTmpBuf);
            }
            // Reset the written character width
            writtenCharsWidth = 0;
        } else {
            // We braked due to something else.
            logger(LOGGER_LEVEL_ERROR, "Error in printing line\r\n");
        }
    }
    return numLines;
}

/**
 * @brief Function to print the menu
 * @param pDisplayState Pointer to displayState
 * @param pMenuOption Pointer to the menu option to print
 * @param offset_y Offset of Y for the box, in pixels
 * @param offset_x Offset of X for the box, in pixels
 * @param spacing_y Spacing in the Y axis between each box
 * @param selected_item This is the currently selected item.
 * @param charsHeight Height, or offset, in pixels in Y-direction.
 * @return Number of rows written.
 */
int printMenuItem(displayState_t *pDisplayState, menuOption_t *pMenuOption,
                  uint16_t offset_y, uint16_t offset_x, uint16_t spacing_y,
                  bool selected_item, int charsHeight) {
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;

    // Get the length of the string as it would be printed in the
    // current font:
    uint8_t charIter = 0;
    int widthAllChars = 0;
    int charsWidth = offset_x + 10;
    int numLines =
        printMenuOptionString(pDisplayState, pMenuOption->pOptionString,
                              offset_x, offset_y, EVE_HSIZE / 2 - (offset_x), 0,
                              20, spacing_y, charsHeight, false, false);

    if (pMenuOption->pDisplayFun != NULL) {
        // Run the display function here to get the string
        // that displays the selected option.
        char pString[MAX_MENU_DISPLAY_FUN_STRING] = {0};
        (*((menu_function *)(pMenuOption->pDisplayFun)))(pDisplayState,
                                                         pString);
        int numLinesFn =
            printMenuOptionString(pDisplayState, pString, offset_x, offset_y,
                                  EVE_HSIZE / 2 - (offset_x), 0, 20, spacing_y,
                                  charsHeight, true, true);
        if (numLinesFn > numLines) {
            // The result is longer than the option string. Use the result
            // string numLines
            numLines = numLinesFn;
        }
    }

    uint16_t menuOptionOffsetY = (numLines - 1) * charsHeight +
                                 pCurrentFont->font_caps_height + spacing_y * 2;
    // Outline shall be white
    if (selected_item) {
        EVE_color_rgb_burst(WHITE);
    } else {
        EVE_color_rgb_burst(GRAY);
    }
    // Write the rectangle encompassing the option.
    EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
    EVE_cmd_dl(VERTEX2F((offset_x)*16, (offset_y)*16));
    EVE_cmd_dl(VERTEX2F((EVE_HSIZE - (offset_x)) * 16,
                        ((offset_y + menuOptionOffsetY)) * 16));

    // Select the color of the text
    if (selected_item) {
        EVE_color_rgb_burst(BLACK);
    } else {
        EVE_color_rgb_burst(WHITE);
    }

    // Go through and print each char, checking that the total width doesn't
    // go past the halfway point. If it does, then wrap and start a new line.
    printMenuOptionString(pDisplayState, pMenuOption->pOptionString, offset_x,
                          offset_y, EVE_HSIZE / 2 - (offset_x),
                          (offset_y + menuOptionOffsetY), 20, spacing_y,
                          charsHeight, false, true);

    if (pMenuOption->pDisplayFun != NULL) {
        // Run the display function here to get the string
        // that displays the selected option.
        char pString[MAX_MENU_DISPLAY_FUN_STRING] = {0};
        (*((menu_function *)(pMenuOption->pDisplayFun)))(pDisplayState,
                                                         pString);
        printMenuOptionString(pDisplayState, pString, offset_x, offset_y,
                              EVE_HSIZE / 2 - (offset_x),
                              (offset_y + menuOptionOffsetY), 20, spacing_y,
                              charsHeight, true, true);
    }
    return numLines;
}
/**
 * @brief Function to print the menu
 * @param pDisplayState Pointer to displayState
 * @return Nothing
 */
static void displayMenu(displayState_t *pDisplayState) {
    // display list is already started, will be ended outside of this function.

    // Get the small font, which is what is to be used in the menu.
    font_t *pCurrentFont =
        pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;

    // Find the offset based on font size
    // Take the size of the caps, plus 10 pixels
    uint8_t menuOptionOffsetY = pCurrentFont->font_caps_height + 20;

    // Offsets around the boxes
    uint16_t offset_y =
        10; // Starting offset. Will later depend on the font size
    uint16_t spacing_y = 10;    // 10 px spacing between items
    uint16_t spacing_boxes = 4; // Spacing between each box
    uint16_t offset_x = 10;     // 10 px spacing around x to the border.
    // Print a box around the menu:

    // Outline shall be white
    EVE_color_rgb_burst(WHITE);
    // Write the top line
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F((offset_x / 2) * 16, (spacing_y / 2) * 16));
    EVE_cmd_dl(VERTEX2F((EVE_HSIZE - offset_x / 2) * 16, (spacing_y / 2) * 16));

    // Write the bottom line
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(
        VERTEX2F((offset_x / 2) * 16, (EVE_VSIZE - (spacing_y / 2)) * 16));
    EVE_cmd_dl(VERTEX2F((TOP_OUTLINE_X1 - (offset_x / 2)) * 16,
                        (EVE_VSIZE - (spacing_y / 2)) * 16));

    // Write the right vertical line
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F((EVE_HSIZE - offset_x / 2) * 16, (spacing_y / 2) * 16));
    EVE_cmd_dl(VERTEX2F((TOP_OUTLINE_X1 - (offset_x / 2)) * 16,
                        (EVE_VSIZE - (spacing_y / 2)) * 16));

    // Write the left vertical line
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F((offset_x / 2) * 16, (spacing_y / 2) * 16));
    EVE_cmd_dl(
        VERTEX2F((offset_x / 2) * 16, (EVE_VSIZE - (spacing_y / 2)) * 16));

    // Get the menu state
    menuState_t *pMenuState = pDisplayState->pMenuState;
    menuOption_t *pCurrentMenuOption = pMenuState->pCurrentMenuOption;
    menuOption_t *pMenuOption = pMenuState->pMenuOptionList;
    uint8_t menuItemCount = 0;
    // Print the top level menu
    // Go through the list
    while (pMenuOption->pOptionString != NULL) {
        // The pMenuOption points to a list of menu options.
        // Go through and print.
        bool selected_item = false;
        if (pCurrentMenuOption == pMenuOption) {
            selected_item = true;
        } else {
            selected_item = false;
        }
        int rows = printMenuItem(pDisplayState, pMenuOption, offset_y, offset_x,
                                 spacing_y, selected_item, menuOptionOffsetY);
        pMenuOption++;
        offset_y += rows * menuOptionOffsetY + spacing_boxes;
    }
}

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
        }
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

        if (localDisplayState.inMenu) {
            // Loop until the internal state changes
            while (localDisplayState.inMenu) {
                // Update the screen:
                startDisplaylist();
                // Display the menu
                displayMenu(&localDisplayState);
                // End the display list
                endDisplayList();
                // Hijack the uartReceiveQueue and update accordingly
                // Note, we wait for uart in this function
                updateMenuState(&localDisplayState, &displayState);
                // Create a task delay to give other equal priority a chance
                // tasks to run.
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }

        } else {
            // Update the screen:
            startDisplaylist();
            // Display the outline
            displayOutline();
            // Write the calculator setting state:
            displayCalcState(&localDisplayState);
            // Write the input text
            displayInputText(&localDisplayState, writeCursor);
            // Print the results
            printResult(&localDisplayState);
            // End the display list
            endDisplayList();
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void testDisplay() {
    while (1) {
        logger(LOGGER_LEVEL_DEBUG, "DISPLAY TEST: DISPLAY THE OUTLINE\r\n");
        // HACK: temporary function just to check that the display works
        startDisplaylist();
        displayOutline();
        endDisplayList();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}