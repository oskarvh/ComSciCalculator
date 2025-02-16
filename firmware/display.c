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
#include "EVE_config.h"

#include "display.h"
#include "firmware_common.h"
#include "print_utils.h"
#include "logger.h"

// Clay is a bit funny in that it pulls in the c files and attaches them on
// top instead of linking through .h files.
#define CLAY_IMPLEMENTATION
#include "clay.h"
#include "clay_renderer_ft81x.c"
#include "gui.h"

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



// /**
//  * @brief Print the calculator state to the top of the screen.
//  *        This function displays the arithmetic used (float, int, fixed),
//  *        the base (hex, dec, bin), the bit length (0-64), and anything else
//  *        that might be useful.
//  * @param pRxBuf Pointer to the input string
//  * @param syntaxErrorIndex Index
//  * @return Nothing
//  */
// void displayCalcState(displayState_t *pDisplayState) {
//     // Get the string and length depending on the base:
//     const char *pBaseString =
//         baseDisplayStrings[pDisplayState->inputOptions.inputBase];
//     uint8_t baseStringLen = strlen(pBaseString);

//     // Get the string and length depending on the input format:
//     const char *pInputFormatString =
//         formatDisplayStrings[pDisplayState->inputOptions.inputFormat];
//     uint8_t inputFormatStringLen = strlen(pInputFormatString);

//     // Display the bit width. Note: for fixed point, it's shown in Q notation
//     char bitWidthString[7] = {0}; // Worst case scenario is 100.28\0
//     if (pDisplayState->inputOptions.inputFormat == INPUT_FMT_FIXED) {
//         // Fixed point require Q notation.
//         uint8_t numBits = pDisplayState->inputOptions.numBits;
//         uint8_t decimalBits =
//             getEffectiveFixedPointDecimalPlace(&(pDisplayState->inputOptions));
//         // Work out the Q notation:
//         uint8_t integerBits = numBits - decimalBits;
//         sprintf(bitWidthString, "%u.%u", integerBits, decimalBits);
//     } else {
//         // Just get the bit width as int and convert to string
//         sprintf(bitWidthString, "%u", pDisplayState->inputOptions.numBits);
//     }

//     // Get the string and length depending on the output format:
//     const char *pOutputFormatString =
//         formatDisplayStrings[pDisplayState->inputOptions.outputFormat];
//     uint8_t outputFormatStringLen = strlen(pOutputFormatString);

//     /*
//     // Display the bit width. Note: for fixed point, it's shown in Q notation
//     // Ignore the output format bit length for now. Since it's shared with the
//     input. char outputBitWidthString[7] = {0}; // Worst case scenario is
//     100.28\0 if(pDisplayState->inputOptions.inputFormat == INPUT_FMT_FIXED){
//         // Fixed point require Q notation.
//         uint8_t numBits = pDisplayState->inputOptions.numBits;
//         uint8_t decimalBits =
//     pDisplayState->inputOptions.fixedPointDecimalPlace;
//         // Work out the Q notation:
//         uint8_t integerBits = numBits-decimalBits;
//         sprintf(outputBitWidthString, "%u.%u", integerBits, decimalBits);
//     } else {
//         // Just get the bit width as int and convert to string
//         sprintf(outputBitWidthString, "%u",
//     pDisplayState->inputOptions.numBits);
//     }
//     */

//     // Find the maximum length of string we can print to the screen.
//     // This has to be based on the font.
//     char pStatusString[60] = {0};
//     sprintf(pStatusString, "%s  BITS:%s  INPUT:%s  OUTPUT:%s\0", pBaseString,
//             bitWidthString, pInputFormatString, pOutputFormatString);
//     font_t *pCurrentFont =
//         pFontLibraryTable[pDisplayState->fontIdx]->pSmallFont;
//     EVE_cmd_text_burst(OUTPUT_STATUS_X0,
//                        OUTPUT_STATUS_YC0(pCurrentFont->font_caps_height),
//                        pCurrentFont->ft81x_font_index, 0, pStatusString);
// }



// /**
//  * @brief Print the input buffer to the screen
//  * @param pDisplayState Pointer to the display states
//  * @param writeCursor Cursor location
//  * @return Nothing
//  */
// //! Screen left and right buffer area for input
// #define VISIBLE_INPUT_X_BUFFER (5)
// //! How many horizontal pixels are allocated for the input area
// #define VISIBLE_INPUT_X_AREA_PX (EVE_HSIZE - VISIBLE_INPUT_X_BUFFER)
// void displayInputText(displayState_t *pDisplayState, bool writeCursor) {

//     uint8_t colorWheelIndex = 0; // Maximum COLORWHEEL_LEN

//     // If syntaxIssueIndex = -1 then there are no syntax errors.
//     // Iterate through each char until null pointer
//     uint16_t charIter = 0;
//     // Get the current font
//     font_t *pCurrentFont =
//         pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
//     // Width of the cumulative characters now written
//     uint32_t widthWrittenChars = 0;

//     // Calculate if the input display should be wrapped, and by how many lines
//     uint32_t widthAllChars = VISIBLE_INPUT_X_BUFFER;
//     uint16_t numLinesWrap = 0;
//     while (pDisplayState->printedInputBuffer[charIter] != '\0') {
//         widthAllChars += getFontCharWidth(
//             pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
//         // If the current line width is larger than visible area, then that
//         // means a new line should be made
//         if (widthAllChars >= VISIBLE_INPUT_X_AREA_PX) {
//             widthAllChars =
//                 VISIBLE_INPUT_X_BUFFER +
//                 getFontCharWidth(pCurrentFont,
//                                  pDisplayState->printedInputBuffer[charIter]);
//             numLinesWrap++;
//         }
//         charIter++;
//     }
//     charIter = 0;                  // Reset before using again.
//     uint8_t displayWrapOffset = 0; // Track how many lines have been written.
//     uint16_t currentLineWidth =
//         VISIBLE_INPUT_X_BUFFER; // Tracks the current line width
//     while (pDisplayState->printedInputBuffer[charIter] != '\0') {
//         // Increase color index if opening bracket
//         if (pDisplayState->printedInputBuffer[charIter] == '(') {
//             colorWheelIndex++;
//         }
//         uint32_t color = colorWheel[colorWheelIndex % COLORWHEEL_LEN];

//         if (charIter >= ((uint16_t)(pDisplayState->syntaxIssueIndex))) {
//             color = RED;
//         }
//         // Print the current character
//         EVE_cmd_dl_burst(DL_COLOR_RGB | color);

//         // Add the width of the char to be written:
//         widthWrittenChars += getFontCharWidth(
//             pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
//         currentLineWidth += getFontCharWidth(
//             pCurrentFont, pDisplayState->printedInputBuffer[charIter]);
//         // If the current line width is larger than visible area, then that
//         // means a new line should be made
//         if (currentLineWidth >= VISIBLE_INPUT_X_AREA_PX) {
//             currentLineWidth =
//                 VISIBLE_INPUT_X_BUFFER +
//                 getFontCharWidth(pCurrentFont,
//                                  pDisplayState->printedInputBuffer[charIter]);
//             displayWrapOffset++;
//         }
//         // Have a temporary buffer to be able to print in different colors. Null
//         // terminated
//         char pTmpRxBuf[2] = {pDisplayState->printedInputBuffer[charIter], '\0'};

//         uint32_t yOffset = INPUT_TEXT_YC0(pCurrentFont->font_caps_height) -
//                            (numLinesWrap - displayWrapOffset) *
//                                (pCurrentFont->font_caps_height + 5);
//         // Print one colored char
//         EVE_cmd_text_burst(
//             currentLineWidth,
//             yOffset, // INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
//             pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pTmpRxBuf);

//         // Decrease color index if closing bracket.
//         if (pDisplayState->printedInputBuffer[charIter] == ')') {
//             colorWheelIndex--;
//         }
//         charIter++;
//     }
//     if (writeCursor) {
//         displayWrapOffset = 0;
//         // Get the width of the chars until the current cursor
//         uint32_t widthWrittenCharsUntilCursor = VISIBLE_INPUT_X_BUFFER;
//         for (int i = 0; i < charIter - pDisplayState->cursorLoc; i++) {
//             widthWrittenCharsUntilCursor += getFontCharWidth(
//                 pCurrentFont, pDisplayState->printedInputBuffer[i]);
//             // Reset if width of the screen has been reached.
//             if (widthWrittenCharsUntilCursor >= VISIBLE_INPUT_X_AREA_PX) {
//                 widthWrittenCharsUntilCursor =
//                     VISIBLE_INPUT_X_BUFFER +
//                     getFontCharWidth(pCurrentFont,
//                                      pDisplayState->printedInputBuffer[i]);
//                 displayWrapOffset++;
//             }
//         }
//         // Cursor is always white
//         EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
//         // Write cursor
//         uint32_t yOffset = INPUT_TEXT_YC0(pCurrentFont->font_caps_height) -
//                            (numLinesWrap - displayWrapOffset) *
//                                (pCurrentFont->font_caps_height + 5);
//         EVE_cmd_text_burst(widthWrittenCharsUntilCursor +
//                                getFontCharWidth(pCurrentFont, ' ') / 2,
//                            yOffset, pCurrentFont->ft81x_font_index,
//                            INPUT_TEXT_OPTIONS, "|");
//     }
// }



// /**
//  * @brief Function to print
//  * @param pString Pointer to string to be printed
//  * @param yStart Y start pixel
//  * @param xStart X start pixel
//  * @param xMax Maximum length of bounding box
//  * @param pFont Pointer to font
//  * @return Nothing
//  */
// static void displayResultWithinBounds(char *pString, uint16_t yStart,
//                                       uint16_t xStart, uint16_t xMax,
//                                       font_t *pFont) {

//     // Iterate through each char until null pointer
//     uint16_t charIter = 0;
//     // Width of the cumulative characters now written
//     uint32_t widthWrittenChars = 0;

//     // Calculate if the input display should be wrapped, and by how many lines
//     uint32_t widthAllChars = VISIBLE_INPUT_X_BUFFER;
//     uint16_t numLinesWrap = 0;
//     while (pString[charIter] != '\0') {
//         widthAllChars += getFontCharWidth(pFont, pString[charIter]);
//         // If the current line width is larger than visible area, then that
//         // means a new line should be made
//         if (widthAllChars >= xMax) {
//             widthAllChars = VISIBLE_INPUT_X_BUFFER +
//                             getFontCharWidth(pFont, pString[charIter]);
//             numLinesWrap++;
//         }
//         charIter++;
//     }

//     uint8_t displayWrapOffset = 0; // Track how many lines have been written.
//     uint16_t currentLineWidth =
//         VISIBLE_INPUT_X_BUFFER; // Tracks the current line width

//     while (charIter-- > 0) {

//         // Add the width of the char to be written:
//         uint8_t currentCharWidth = getFontCharWidth(pFont, pString[charIter]);
//         widthWrittenChars += currentCharWidth;
//         currentLineWidth += currentCharWidth;
//         // If the current line width is larger than visible area, then that
//         // means a new line should be made
//         if (currentLineWidth >= xMax) {
//             currentLineWidth = VISIBLE_INPUT_X_BUFFER + currentCharWidth;
//             displayWrapOffset++;
//         }
//         // Have a temporary buffer to be able to print in different colors. Null
//         // terminated
//         char pTmpRxBuf[2] = {pString[charIter], '\0'};

//         uint32_t yOffset = yStart + (numLinesWrap - displayWrapOffset) *
//                                         (pFont->font_caps_height + 5);
//         // Print one colored char
//         EVE_cmd_text_burst(
//             xStart + xMax - currentLineWidth + currentCharWidth,
//             yOffset, // INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
//             pFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pTmpRxBuf);
//     }
// }

// void printResult(displayState_t *pDisplayState) {
//     // Let the color reflect if the operation was OK or not.
//     if (pDisplayState->solveStatus == calc_solveStatus_SUCCESS) {
//         EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
//     } else {
//         EVE_cmd_dl_burst(DL_COLOR_RGB | GRAY);
//     }
//     // Get the result and output formats
//     SUBRESULT_INT result = pDisplayState->result;

//     // Clear the buffers
//     memset(pDecRes, 0, MAX_PRINTED_BUFFER_LEN_DEC);
//     memset(pBinRes, 0, MAX_PRINTED_BUFFER_LEN_BIN);
//     memset(pHexRes, 0, MAX_PRINTED_BUFFER_LEN_HEX);

//     // Convert to each base
//     convertResult(pDecRes, result, &(pDisplayState->inputOptions),
//                   inputBase_DEC);
//     convertResult(pBinRes, result, &(pDisplayState->inputOptions),
//                   inputBase_BIN);
//     convertResult(pHexRes, result, &(pDisplayState->inputOptions),
//                   inputBase_HEX);

//     // Get the current font:
//     font_t *pCurrentFont =
//         pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
//     displayResultWithinBounds(pDecRes,
//                               OUTPUT_DEC_YC0(pCurrentFont->font_caps_height), 5,
//                               OUTPUT_DEC_X_LEN, pCurrentFont);
//     displayResultWithinBounds(pBinRes,
//                               OUTPUT_BIN_YC0(pCurrentFont->font_caps_height), 5,
//                               OUTPUT_BIN_X_LEN, pCurrentFont);
//     displayResultWithinBounds(pHexRes,
//                               OUTPUT_HEX_YC0(pCurrentFont->font_caps_height),
//                               OUTPUT_HEX_XC0, OUTPUT_HEX_X_LEN, pCurrentFont);
// }

// /**
//  * @brief Print a line to the screen
//  * @param pString Pointer to string to be printed
//  * @param pCurrentFont Pointer to the current font
//  * @param start_x X-coordinate of where to start printing the line
//  * @param start_y Y-coordinate of where to start printing the line
//  * @param rightJustification If true, then print from the right. Otherwise print
//  * from the left.
//  * @return Width in pixels that the word would need to print
//  */
// void printLine(char *pString, font_t *pCurrentFont, uint16_t start_x,
//                uint16_t end_x, uint16_t start_y, bool rightJustification) {
//     if (rightJustification) {
//         EVE_cmd_text_burst(end_x, start_y, pCurrentFont->ft81x_font_index,
//                            EVE_OPT_RIGHTX, pString);
//     } else {
//         EVE_cmd_text_burst(start_x, start_y, pCurrentFont->ft81x_font_index, 0,
//                            pString);
//     }
// }

// uint8_t printMenuOptionString(displayState_t *pDisplayState, char *pString,
//                               uint16_t x1, uint16_t y1, uint16_t x2,
//                               uint16_t linePadding, bool rightJustification,
//                               bool print) {
//     // Write line by line, evaluating the next word.
//     font_t *pCurrentFont =
//         pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;
//     int charIter = 0;
//     int writtenCharsWidth = 0;
//     int maxWidth = x2 - x1;
//     int numLines = 0;
//     // Loop until the character is null
//     while (pString[charIter] != '\0') {
//         // Get the length of the next word:
//         bool writeLine = false;
//         // Find the first non-whitespace char:
//         while (isspace(pString[charIter])) {
//             charIter++;
//         }
//         int endStringIter = charIter;
//         int stringStartIter = charIter;
//         // Loop until it's time to write a line to the screen.
//         while (!writeLine) {
//             int nextWordWidth = 0;
//             if (isalnum(pString[charIter])) {
//                 // This character is a letter/number, chances are the next ones
//                 // are as well:
//                 while (isalnum(pString[endStringIter])) {
//                     nextWordWidth += getFontCharWidth(pCurrentFont,
//                                                       pString[endStringIter++]);
//                 }
//             } else {
//                 // Current char is not alfanumeric, meaning the next word width
//                 // is the width of the char
//                 if (pString[charIter] == '\0' || pString[charIter] == '\n') {
//                     // This is the end of the string, break and print.
//                     writeLine = true;
//                     break;
//                 }
//                 nextWordWidth =
//                     getFontCharWidth(pCurrentFont, pString[charIter]);
//                 endStringIter++;
//             }
//             // The word between charIter and endStringIter is the next word.
//             // Check if it fits on this line, given the width of the characters
//             // already written
//             if (writtenCharsWidth + nextWordWidth > maxWidth) {
//                 // Is the word itself longer than the line?
//                 while (nextWordWidth > maxWidth) {
//                     // This word is longer than the max width of the
//                     // bounding box. If there are charachters already pipelined
//                     // for writing, break and print those and come back here
//                     // next time around.
//                     if (writtenCharsWidth > 0) {
//                         break;
//                     }
//                     // Here, we know that it's for sure a new line.
//                     // We need to splice the word up, and find the end iterator
//                     // for where the word would spill over.
//                     int splitNextWordWidth = 0;
//                     int splitNextWordIter = stringStartIter;
//                     int currentCharWidth = 0;
//                     while (splitNextWordWidth < maxWidth) {
//                         currentCharWidth = getFontCharWidth(
//                             pCurrentFont, pString[splitNextWordIter]);
//                         splitNextWordWidth += currentCharWidth;
//                         splitNextWordIter++;
//                     }
//                     // Disregard the last character, since it makes the string
//                     // wider than the limit
//                     splitNextWordWidth -= currentCharWidth;
//                     splitNextWordIter -= 1;
//                     // Here, we need to print the word between stringStartIter
//                     // and splitNextWordIter, increase the line before and
//                     // after.
//                     int wordLen = splitNextWordIter - stringStartIter;
//                     if (print) {
//                         char *pTmpBuf = malloc(sizeof(char) * (wordLen + 1));
//                         // Set all to null chars to terminate
//                         memset(pTmpBuf, '\0', wordLen + 1);
//                         memcpy(pTmpBuf, &(pString[stringStartIter]),
//                                sizeof(char) * wordLen);
//                         uint16_t yCoordLine =
//                             y1 + numLines * (linePadding +
//                                              pCurrentFont->font_caps_height);
//                         printLine(pTmpBuf, pCurrentFont, x1, x2, yCoordLine,
//                                   rightJustification);
//                         free(pTmpBuf);
//                     }
//                     numLines += 1;
//                     // We're now at a new line, so set the
//                     // next word width minus what we've written here
//                     nextWordWidth -= splitNextWordWidth;
//                     // A new line with no chars has been written,
//                     // so reset the written chars width:
//                     writtenCharsWidth = 0;
//                     // Set the character iterator to the last written character
//                     charIter = splitNextWordIter;
//                     stringStartIter = splitNextWordIter;
//                 }
//                 // Retest, since we might have shaved off some letters
//                 // in the while loop before this
//                 if (writtenCharsWidth + nextWordWidth > maxWidth) {
//                     // Here, the combination of written characters
//                     // and the new word would exceed the width of the line.
//                     // Meaning that we'd want to print the line.
//                     writeLine = true;
//                     // Set the end character iterator to what was before this
//                     // current char.
//                     endStringIter = charIter;
//                     // Remove any trailing spaces.
//                     while (!isalnum(pString[endStringIter - 1])) {
//                         writtenCharsWidth -= getFontCharWidth(
//                             pCurrentFont, pString[endStringIter--]);
//                     }
//                 }
//             }
//             if (writtenCharsWidth + nextWordWidth <= maxWidth) {
//                 // we can fit the current word on this line.
//                 // Add it and move on to the next character
//                 writtenCharsWidth += nextWordWidth;
//                 charIter = endStringIter;
//             }
//             // If the character after the end of this substring is null, then
//             // print.
//             if (pString[charIter] == '\0' || pString[charIter] == '\n') {
//                 writeLine = true;
//             }
//         }
//         // Here, we've either breaked due to some error, or it's time to write a
//         // line
//         if (writeLine) {
//             if (print) {
//                 int wordLen = endStringIter - stringStartIter;
//                 char *pTmpBuf = malloc(sizeof(char) * (wordLen + 1));
//                 // Set all to null chars to terminate
//                 memset(pTmpBuf, '\0', wordLen + 1);
//                 memcpy(pTmpBuf, &(pString[stringStartIter]),
//                        sizeof(char) * wordLen);
//                 uint16_t yCoordLine =
//                     y1 +
//                     numLines * (linePadding + pCurrentFont->font_caps_height);
//                 printLine(pTmpBuf, pCurrentFont, x1, x2, yCoordLine,
//                           rightJustification);
//                 free(pTmpBuf);
//             }
//             // Increase the line number
//             numLines += 1;
//             // Reset the written character width
//             writtenCharsWidth = 0;
//         } else {
//             // We braked due to something else.
//             logger(LOGGER_LEVEL_ERROR, "Error in printing line\r\n");
//         }
//     }
//     return numLines;
// }

// /**
//  * @brief Function that prints a box and text for a menu item.
//  * @param pDisplayState Pointer to displayState
//  * @param pMenuOption Pointer to the menu option to print
//  * @param start_x X-coordinate in pixels where to start the option frame
//  * @param end_x X-coordinate in pixels where to end the option frame
//  * @param start_y Y-coordinate in pixels to the top of the option frame. The
//  * ending will depend on the length of the string(s) that are printed.
//  * @param textPadding_x X-axis padding between the option frame and the text,
//  * one sided
//  * @param textPadding_y Y-axis padding between the option frame and the text,
//  * one sided
//  * @param selected_item True if this is the currently selected item, and should
//  * be formatted as such
//  * @return number of pixels that were used for this option.
//  */
// uint16_t printMenuItem(displayState_t *pDisplayState, menuOption_t *pMenuOption,
//                        uint16_t start_x, uint16_t end_x, uint16_t start_y,
//                        uint16_t textPadding_x, uint16_t textPadding_y,
//                        bool selected_item) {
//     font_t *pCurrentFont =
//         pFontLibraryTable[pDisplayState->fontIdx]->pLargeFont;

//     // Calculate the number of lines for the menu option
//     int numLines = printMenuOptionString(
//         pDisplayState, pMenuOption->pOptionString, start_x + textPadding_x,
//         start_y + textPadding_y,
//         (end_x - start_x) / 2 -
//             textPadding_x / 2, // Should print to the center of the screen.
//         MENU_OPTION_LINE_TEXT_PADDING,
//         false, // left justified
//         false  // Don't print the string to screen
//     );

//     if (pMenuOption->pDisplayFun != NULL) {
//         // Run the display function here to get the string
//         // that displays the selected option.
//         char pString[MAX_MENU_DISPLAY_FUN_STRING] = {0};
//         (*((non_interactive_menu_function *)(pMenuOption->pDisplayFun)))(
//             pDisplayState, pString);
//         int resultNumLines = printMenuOptionString(
//             pDisplayState, pString,
//             (end_x - start_x) / 2 +
//                 textPadding_x / 2, // Start printing in the middle of the screen
//             start_y + textPadding_y, end_x - textPadding_x,
//             MENU_OPTION_LINE_TEXT_PADDING,
//             true, // right justified
//             false // Don't print the string to screen
//         );
//         if (resultNumLines > numLines) {
//             // The result is longer than the option string. Use the result
//             // string numLines
//             numLines = resultNumLines;
//         }
//     }

//     if (numLines < 1) {
//         logger(LOGGER_LEVEL_ERROR, "Trying to print %d lines, returning.\r\n",
//                numLines);
//         return 0;
//     }
//     uint16_t menuOptionFrameHeight =
//         2 * textPadding_y + numLines * pCurrentFont->font_caps_height +
//         (numLines - 1) * MENU_OPTION_LINE_TEXT_PADDING;
//     // Outline shall be white
//     if (selected_item) {
//         EVE_color_rgb_burst(WHITE);
//     } else {
//         EVE_color_rgb_burst(GRAY);
//     }
//     // Write the rectangle encompassing the option.
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_RECTS);
//     EVE_cmd_dl(VERTEX2F(start_x * 16, start_y * 16));
//     EVE_cmd_dl(VERTEX2F(end_x * 16, (start_y + menuOptionFrameHeight) * 16));

//     // Select the color of the text
//     if (selected_item) {
//         EVE_color_rgb_burst(BLACK);
//     } else {
//         EVE_color_rgb_burst(WHITE);
//     }

//     // Go through and print each char, checking that the total width doesn't
//     // go past the halfway point. If it does, then wrap and start a new line.
//     numLines = printMenuOptionString(
//         pDisplayState, pMenuOption->pOptionString, start_x + textPadding_x,
//         start_y + textPadding_y, //+ pCurrentFont->font_caps_height,
//         (end_x - start_x) / 2 -
//             textPadding_x / 2, // Should print to the center of the screen.
//         MENU_OPTION_LINE_TEXT_PADDING,
//         false, // left justified
//         true   // print this time
//     );

//     if (pMenuOption->pDisplayFun != NULL) {
//         // Run the display function here to get the string
//         // that displays the selected option.
//         char pString[MAX_MENU_DISPLAY_FUN_STRING] = {0};
//         (*((non_interactive_menu_function *)(pMenuOption->pDisplayFun)))(
//             pDisplayState, pString);
//         int resultNumLines = printMenuOptionString(
//             pDisplayState, pString,
//             (end_x - start_x) / 2 +
//                 textPadding_x / 2, // Start printing in the middle of the screen
//             start_y + textPadding_y, end_x - textPadding_x,
//             MENU_OPTION_LINE_TEXT_PADDING,
//             true, // Right justified
//             true  // Print this time
//         );
//     }
//     return menuOptionFrameHeight;
// }

// void printMenuOutline(uint16_t spacing_x, uint16_t spacing_y) {
//     // Outline shall be white
//     EVE_color_rgb_burst(WHITE);
//     // Write the top line
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
//     EVE_cmd_dl(VERTEX2F((spacing_x / 2) * 16, (spacing_y / 2) * 16));
//     EVE_cmd_dl(
//         VERTEX2F((EVE_HSIZE - spacing_x / 2) * 16, (spacing_y / 2) * 16));

//     // Write the bottom line
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
//     EVE_cmd_dl(
//         VERTEX2F((spacing_x / 2) * 16, (EVE_VSIZE - (spacing_y / 2)) * 16));
//     EVE_cmd_dl(VERTEX2F((TOP_OUTLINE_X1 - (spacing_x / 2)) * 16,
//                         (EVE_VSIZE - (spacing_y / 2)) * 16));

//     // Write the right vertical line
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
//     EVE_cmd_dl(
//         VERTEX2F((EVE_HSIZE - spacing_x / 2) * 16, (spacing_y / 2) * 16));
//     EVE_cmd_dl(VERTEX2F((TOP_OUTLINE_X1 - (spacing_x / 2)) * 16,
//                         (EVE_VSIZE - (spacing_y / 2)) * 16));

//     // Write the left vertical line
//     EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
//     EVE_cmd_dl(VERTEX2F((spacing_x / 2) * 16, (spacing_y / 2) * 16));
//     EVE_cmd_dl(
//         VERTEX2F((spacing_x / 2) * 16, (EVE_VSIZE - (spacing_y / 2)) * 16));
// }


/**
 * @brief Print the main screen of the calculator
 * @param pDisplayState Pointer to the display states
 * @param writeCursor True if the cursor should be written
 * @return Nothing
 */
static void displayMainScreen(displayState_t *pDisplayState, bool writeCursor){
    
    memset(pDecRes, 0, MAX_PRINTED_BUFFER_LEN_DEC);
    memset(pBinRes, 0, MAX_PRINTED_BUFFER_LEN_BIN);
    memset(pHexRes, 0, MAX_PRINTED_BUFFER_LEN_HEX);

    // Convert to each base
    SUBRESULT_INT result = pDisplayState->result;
    convertResult(pDecRes, result, &(pDisplayState->inputOptions),
                  inputBase_DEC);
    convertResult(pBinRes, result, &(pDisplayState->inputOptions),
                  inputBase_BIN);
    convertResult(pHexRes, result, &(pDisplayState->inputOptions),
                  inputBase_HEX);
    
    uint8_t fontId = pDisplayState->fontIdx;

    // Start the layout routine
    Clay_BeginLayout();
    mainScreen(
        pDisplayState->printedInputBuffer,
        pHexRes,
        pDecRes,
        pBinRes,
        "Settings",
        fontId
    );
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    Clay_ft81x_Render(renderCommands);
}

/**
 * @brief Function to print the menu
 * @param pDisplayState Pointer to displayState
 * @return Nothing
 */
static void displayMenu(displayState_t *pDisplayState) {
    Clay_BeginLayout();
    CLAY(CLAY_ID("OuterContainer"), CLAY_LAYOUT({ .sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(16), .childGap = 16 }), CLAY_RECTANGLE({ .color = {250,250,255,255} })) {

    }
    Clay_RenderCommandArray renderCommands = Clay_EndLayout();
    Clay_ft81x_Render(renderCommands);
}

/**
 * @brief Function to handle clay errors
 * @param errorData Error data
 * @return Nothing
 */
static void HandleClayErrors(Clay_ErrorData errorData) {
    // See the Clay_ErrorData struct for more information
    logger(LOGGER_LEVEL_ERROR,"%s", errorData.errorText.chars);
    switch(errorData.errorType) {
        // etc
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
    pDisplayState->inputOptions.numBits = 64;                // TBD
    pDisplayState->inputOptions.sign = 0;                    // TBD
    pDisplayState->solveStatus = calc_solveStatus_INPUT_LIST_NULL;
    pDisplayState->printStatus = 0;
    pDisplayState->fontIdx = 0; // Try the custom RAM font
    memset(pDisplayState->printedInputBuffer, '\0', MAX_PRINTED_BUFFER_LEN);
    pDisplayState->syntaxIssueIndex = -1;
    pDisplayState->inMenu = false;
    pDisplayState->pMenuState = NULL;//&topMenu;
}

void displayTask(void *p) {

    // Set the max number of elements to something we can handle
    Clay_SetMaxElementCount(50);

    // Initialize the FT81x and SPI
    Clay_ft81x_Initialize();

    // Initialize clay's memory arena
    uint64_t totalMemorySize = Clay_MinMemorySize();
    Clay_Arena arena = Clay_CreateArenaWithCapacityAndMemory(totalMemorySize, malloc(totalMemorySize));

    // Initialize Clay
    Clay_Initialize(arena, (Clay_Dimensions) { EVE_HSIZE, EVE_VSIZE }, (Clay_ErrorHandler) { HandleClayErrors });
    Clay_SetMeasureTextFunction(ft81x_MeasureText, 0);

    // Note: The physical dimensions of the screen never change.
    Clay_SetLayoutDimensions((Clay_Dimensions) { EVE_HSIZE, EVE_VSIZE });


    // Update the screen to begin with
    bool updateScreen = true;
    bool writeCursor = true;
    displayState_t localDisplayState;
    initDisplayState(&localDisplayState);

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
                // Display the menu
                displayMenu(&localDisplayState);
                // Hijack the uartReceiveQueue and update accordingly
                // Note, we wait for uart in this function
                //updateMenuState(&localDisplayState, &displayState);
                // Create a task delay to give other equal priority a chance
                // tasks to run.
                vTaskDelay(10 / portTICK_PERIOD_MS);
            }

        } else {
            displayMainScreen(&localDisplayState, writeCursor);
        }

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

// void testDisplay() {
//     while (1) {
//         logger(LOGGER_LEVEL_DEBUG, "DISPLAY TEST: DISPLAY THE OUTLINE\r\n");
//         // HACK: temporary function just to check that the display works
//         startDisplaylist();
//         displayOutline();
//         endDisplayList();
//         vTaskDelay(1000 / portTICK_PERIOD_MS);
//     }
// }