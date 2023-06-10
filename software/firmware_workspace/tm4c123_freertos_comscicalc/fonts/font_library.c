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

// Standard library
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

// Included fonts.
#include "font_library.h"

// Included fonts. Add new fonts here.
#include "Cascadia_38_L2.h"
#include "Comic_Sans_MS_38_L2.h"


font_t rom_font_0 = {
    .rom_font = true,
    .ft81x_font_index = 18,
    .font_name = "ROM18",
    .font_format = 0,
    .font_size = 8,
    .font_baseline = 0,
    .font_caps_height = 18,
    .font_x_width = 8,
    .pFontTable = NULL,
    .fontTableSize = 0
};
font_t rom_font_1 = {
    .rom_font = true,
    .ft81x_font_index = 31,
    .font_name = "ROM24",
    .font_format = 0,
    .font_size = 8,
    .font_baseline = 0,
    .font_caps_height = 40,
    .font_x_width = 24,
    .pFontTable = NULL,
    .fontTableSize = 0
};

font_t *pFontLibraryTable[MAX_LEN_FONT_LIBRARY_TABLE] = {
    [0] = &rom_font_0,
    [1] = &font_Cascadia_38_L2,
    [2] = &font_Comic_Sans_MS_38_L2,
    [3] = NULL,
    [4] = NULL,
    [5] = NULL,
    [6] = NULL,
    [7] = NULL,
    [8] = NULL,
    [9] = NULL
};

