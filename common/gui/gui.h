/*
MIT License

Copyright (c) 2025 Oskar von Heideken

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


/**
 * @defgroup ColorDefinitions Definitions for the colors used in the display
 * @{
 */
#define COLORWHEEL_LEN 7
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
#define LIGHT_GRAY 0x707070
#define GRAY 0x303030
#define DARK_GRAY 0x101010
#define TURQOISE 0x00fff7
/**@}*/


/**
 * @brief Display the main screen of the calculator
 * @param pInputText Pointer to the input text
 * @param pHexText Pointer to the hex text
 * @param pDecText Pointer to the dec text
 * @param pBinText Pointer to the bin text
 * @param pSettingsText Pointer to the settings text
 * @param fontId The font id to use
 * @return Nothing
 */
void mainScreen(
    char* pInputText, 
    char* pHexText, 
    char* pDecText, 
    char* pBinText,
    char* pSettingsText,
    uint8_t fontId
    );