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

//! Binary result string buffer
char pBinRes[MAX_PRINTED_BUFFER_LEN] = {0};
//! Hexadecimal result string buffer
char pHexRes[MAX_PRINTED_BUFFER_LEN] = {0};
//! Decimal result string buffer
char pDecRes[MAX_PRINTED_BUFFER_LEN] = {0};
//! Color wheel for the brackets
const uint32_t colorWheel[COLORWHEEL_LEN] = {
   WHITE,
   GREEN,
   TURQOISE,
   BLUE_2,
   MAGENTA
};

/**
 * @brief Print the outline of graphics
 *
 * This attempts to solve the current buffer and reflect the result
 * in the #pCalcCoreState.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of solving the buffer.
 */
static void displayOutline(void){
    // Outline shall be white
    EVE_color_rgb_burst(WHITE);
    // Write the top line. This parts the options from the input
    EVE_cmd_dl_burst(DL_BEGIN | EVE_LINES);
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X0*16,TOP_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(TOP_OUTLINE_X1*16,TOP_OUTLINE_Y*16));
    // Write middle line parting input and binary
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X0*16,MID_TOP_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(MID_TOP_OUTLINE_X1*16,MID_TOP_OUTLINE_Y*16));
    // Lower middle outline parting binary and hex/dec
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X0*16,MID_LOW_OUTLINE_Y*16));
    EVE_cmd_dl(VERTEX2F(MID_LOW_OUTLINE_X1*16,MID_LOW_OUTLINE_Y*16));
    // Vertical line parting hex and dec results
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X*16,VERT_LOW_OUTLINE_Y0*16));
    EVE_cmd_dl(VERTEX2F(VERT_LOW_OUTLINE_X*16,VERT_LOW_OUTLINE_Y1*16));
}

/**
 * @brief Starts the display list, clear local buffers and clears color buffers
 */
static void startDisplaylist(void){
    EVE_start_cmd_burst();
    EVE_cmd_dl_burst(CMD_DLSTART);
    EVE_cmd_dl_burst(DL_CLEAR_COLOR_RGB | BLACK);
    EVE_cmd_dl_burst(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);
}

/**
 * @brief End the display list by sending display and swap DL
 */
static void endDisplayList(void){
    EVE_cmd_dl_burst(DL_DISPLAY);
    EVE_cmd_dl_burst(CMD_SWAP);
    EVE_end_cmd_burst();
    // Wait until EVE is not busy anymore to ensure conclusion of command.
    while(EVE_busy());
}

/**
 * @brief Convert binary value to string
 * @param pBuf Pointer to buffer to write the binary string
 * @param num Binary number to convert to string
 * @return Nothing
 */
static void printToBinary(char* pBuf, uint32_t num){
    // First, reset the buffer
    pBuf[0] = '0';
    pBuf[1] = 'b';

    // In order to print the buffer with non-leading zeros,
    // the last index must be the least significant bit.
    // The approach is to go through each bit, starting from
    // the MSB, find the first non-zero bit and start printing
    // that.
    bool firstBitFound = false;
    uint8_t index = 2;
    if(num != 0){
        for(int i = 31 ; i >= 0 ; i--){
            // Get the bit at the current index.
            uint8_t currentBit = (num & (1 << i)) >> i;
            // If the current bit is 1, and the first bit
            // hasn't been found, then set the firstBitFound
            // as true, causing the string to start printing
            if(!firstBitFound && currentBit == 1){
                firstBitFound = true;
            }
            if(firstBitFound){
                pBuf[index++] = currentBit + '0';
            }
        }
    } else {
        // If the result is 0, then print a 0
        pBuf[index++] = '0';
    }
    // Set the last index to a null terminator
    pBuf[index] = '\0';
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
void programFont(font_t *pFont, uint8_t fontIndex){
    uint32_t thisFontsAddress = EVE_RAM_G + ram_g_address_offset;
    // Null check the pointer
    if(pFont == NULL){
        return;
    }
    // If this is a ROM font, or of the pointer to the table is NULL, then return
    if(pFont->rom_font == true || pFont->pFontTable == NULL){
        return;
    }
    // Write the bitmap of the pFont to the graphics RAM if there is room.
    if(ram_g_address_offset + pFont->fontTableSize < EVE_RAM_G_SIZE){
        // Program the font
        EVE_memWrite_sram_buffer(EVE_RAM_G + ram_g_address_offset,
                                 pFont->pFontTable,
                                 pFont->fontTableSize);
        // Program the offset for the font.
        // This is at offset 144 in the font table,
        // and is per default set to 148, which is the offset
        // of the font to the start of the raw data.
        // However, if programming more than 1 font, this offset
        // needs to change, as it's the offset to the start of RAM_G
        // Hence, it should be the ram_g_address_offset
        uint32_t offsetInRam = ram_g_address_offset + 148;
        EVE_memWrite_sram_buffer(EVE_RAM_G + ram_g_address_offset + 144,
                                     (uint8_t*)&offsetInRam,
                                     4);
        ram_g_address_offset += pFont->fontTableSize;
        // Round up to the nearest four byte aligned address:
        uint32_t offset = ram_g_address_offset%4;
        ram_g_address_offset += offset;
    } else {
        logger("\r\nERROR: Font not programmed!\r\n\r\n");
        return;
    }
    // This must be in a display list to register the new font
    startDisplaylist();
    // CMD_SETBITMAP not mandatory if using setfont2
    EVE_cmd_setfont2_burst(fontIndex, thisFontsAddress, 32);

    endDisplayList();
}
/**
 * @brief Find the width of the font, in pixels
 * @param pFont Pointer to the font struct
 * @param c Character to find the width of
 * @return Width of the current char at that font in pixels
 * @warning This function assumes that the first char is a space,
 * and follows the ASCII char setup
 */
uint8_t getFontCharWidth(font_t *pFont, char c){
    // The font table contains a LUT for the char widths,
    // so using the char as an index, the width is simply the
    // value at that index. Conveniently, it's the first data
    // in the struct
    if(!pFont->rom_font){
        return pFont->pFontTable[(uint8_t)c];
    } else {
        // TODO!!
        return pFont->font_x_width;
    }
}

/**
 * @brief Print the input buffer to the screen
 * @param pRxBuf Pointer to the input string
 * @param syntaxErrorIndex Index
 * @return Nothing
 */
void displayInputText(displayState_t *pDisplayState, bool writeCursor){

    uint8_t colorWheelIndex = 0; // Maximum COLORWHEEL_LEN

    // If syntaxIssueIndex = -1 then there are no syntax errors.
    // Iterate through each char until null pointer
    uint16_t charIter = 0;
    // Get the current font
    font_t *pCurrentFont = pFontLibraryTable[pDisplayState->fontIdx];
    // Width of the cumulative characters now written
    uint32_t widthWrittenChars = 0;
    while(pDisplayState->printedInputBuffer[charIter] != '\0'){
        // Increase color index if opening bracket
        if(pDisplayState->printedInputBuffer[charIter] == '('){
            colorWheelIndex++;
        }
        uint32_t color = colorWheel[colorWheelIndex%COLORWHEEL_LEN];

        if(charIter >= ((uint16_t)(pDisplayState->syntaxIssueIndex))){
            color = RED;
        }
        // Print the current character
        EVE_cmd_dl_burst(DL_COLOR_RGB | color);

        // Add the width of the char to be written:
        widthWrittenChars += getFontCharWidth(pCurrentFont, pDisplayState->printedInputBuffer[charIter]);

        // Have a temporary buffer to be able to print in different colors. Null terminated
        char pTmpRxBuf[2] = {pDisplayState->printedInputBuffer[charIter], '\0'};

        // Print one colored char
        EVE_cmd_text_burst(5+widthWrittenChars,
                           INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
                           pCurrentFont->ft81x_font_index,
                           INPUT_TEXT_OPTIONS,
                           pTmpRxBuf);


        // Decrease color index if closing bracket.
        if(pDisplayState->printedInputBuffer[charIter] == ')'){
            colorWheelIndex--;
        }
        charIter++;
    }
    if(writeCursor){
        EVE_cmd_text_burst(5+widthWrittenChars + getFontCharWidth(pCurrentFont, ' ')/2,
                           INPUT_TEXT_YC0(pCurrentFont->font_caps_height),
                           pCurrentFont->ft81x_font_index,
                           INPUT_TEXT_OPTIONS,
                           "|");
    }
}


void initDisplayState(displayState_t *pDisplayState){
    pDisplayState->inputOptions.currentInputArith = 0; // TBD
    pDisplayState->inputOptions.currentInputBase = 0; // TBD
    pDisplayState->solveStatus = 0;
    pDisplayState->printStatus = 0;
    pDisplayState->fontIdx = 1; // Try the custom RAM font
    memset(pDisplayState->printedInputBuffer, '\0', MAX_PRINTED_BUFFER_LEN);
    pDisplayState->syntaxIssueIndex = -1;

}


// TODO:
// 1. Support different color output + freeze output if
//    calculation was unsuccessful. DONE
// 2. Support non-monospaced fonts (it's possible to find the
//    spacing from the font meta data) DONE
// 3. Support multiple fonts. DONE
// 3. Support blinking cursor.
//      Blinking done but I need the font spacing to be completed before I can
//      set this as done.
void displayTask(void *p){
    // Write the outlines:
    startDisplaylist();
    displayOutline();
    endDisplayList();

    // Loop through the custom fonts and program as many
    // as fits.

    for(uint8_t i = 0 ; i < MAX_LEN_FONT_LIBRARY_TABLE ; i++){

        if(pFontLibraryTable[i] != NULL){
            programFont(pFontLibraryTable[i], i);
            pFontLibraryTable[i]->ft81x_font_index = i;
        }
    }

    // Update the screen to begin with
    bool updateScreen = true;
    bool writeCursor = true;
    displayState_t localDisplayState;

    while(1){
        // Wait for the trigger event from the calculator task to tell this task
        // to update the display, since new data is available and ready.
        // Both updating the cursor and updating the values on screen shall trigger an event.
        uint32_t eventbits = xEventGroupWaitBits(displayTriggerEvent, (DISPLAY_EVENT_CURSOR | DISPLAY_EVENT_NEW_DATA), pdTRUE, pdFALSE, portMAX_DELAY);
        if(eventbits & DISPLAY_EVENT_CURSOR){
            // Invert the cursor to blink it.
            writeCursor = !writeCursor;
        }

        // Wait (forever) for the semaphore to be available.
        if( xSemaphoreTake( displayStateSemaphore, portMAX_DELAY) ){
            // To save time, copy the display state. Sort of waste of space. Not sure if this is the best approach..
            memcpy(&localDisplayState, &displayState, sizeof(displayState_t));
            // Release the semaphore, since we're done with the display state global variable
            xSemaphoreGive(displayStateSemaphore);
            // Trigger a screen update
            updateScreen = true;
        }
        if(updateScreen){
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
            printToBinary(pBinRes, localDisplayState.result);
            sprintf(pHexRes, "0x%1X", localDisplayState.result);
            // Update the screen:
            startDisplaylist();
            displayOutline();
            // Write the input text
            displayInputText(&localDisplayState, writeCursor);
            //EVE_cmd_text_burst(INPUT_TEXT_XC0, INPUT_TEXT_YC0, FONT, INPUT_TEXT_OPTIONS, pRxBuf);

            // Let the color reflect if the operation was OK or not.
            if(localDisplayState.solveStatus == calc_solveStatus_SUCCESS){
                EVE_cmd_dl_burst(DL_COLOR_RGB | WHITE);
            } else {
                EVE_cmd_dl_burst(DL_COLOR_RGB | GRAY);
            }
            font_t *pCurrentFont = pFontLibraryTable[localDisplayState.fontIdx];
            EVE_cmd_text_burst(OUTPUT_DEC_XC0, OUTPUT_DEC_YC0(pCurrentFont->font_caps_height), pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pDecRes);
            EVE_cmd_text_burst(OUTPUT_BIN_XC0, OUTPUT_BIN_YC0(pCurrentFont->font_caps_height), pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pBinRes);
            EVE_cmd_text_burst(OUTPUT_HEX_XC0, OUTPUT_HEX_YC0(pCurrentFont->font_caps_height), pCurrentFont->ft81x_font_index, INPUT_TEXT_OPTIONS, pHexRes);
            endDisplayList();
            // Screen has been updated, set update variable to false
            updateScreen = false;
        }
    }
}

