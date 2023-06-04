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

#ifndef FONTS_FONT_H_
#define FONTS_FONT_H_

// Standard library
#include <stdint.h>
#include <stdlib.h>

#define MAX_FONT_NAME_LEN 40

/**
 * @brief Struct for holding the font
 * metadata. Note that if this changes,
 * the script for converting the fonts are also
 * required to change accordingly.
 */
typedef struct font {
    /**
     * @param rom_font True if this is a ROM font.
     * Otherwise it's placed in RAM.
     */
    bool rom_font;
    /**
     * @param ft81x_font_index Font index in the FT81x
     */
    uint8_t ft81x_font_index;
    /**
     * @param font_name Null terminated string
     * containing the name of the font
     */
    char font_name[MAX_FONT_NAME_LEN];
    /**
     * @param font_format Core font postscript level (I think)
     */
    uint8_t font_format;
    /**
     * @param font_size Font size
     */
    uint8_t font_size;
    /**
     * @param font_baseline Font baseline
     */
    uint8_t font_baseline;
    /**
     * @param font_caps_height Height of the font
     */
    uint8_t font_caps_height;
    /**
     * @param font_x_width Font width in x.
     * @warning Only monospaced fonts supported
     */
    uint8_t font_x_width;
    /**
     * @param pFontTable Pointer to the font table
     */
    const uint8_t *pFontTable;
    /**
     * @param fontTableSize Size of the font table in bytes
     */
    uint32_t fontTableSize;
}font_t;


#endif /* FONTS_FONT_H_ */
