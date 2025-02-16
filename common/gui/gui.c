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

/*
This file contains the common parts of the GUI which can be used both for embedded and PC application.
Note that these are all the high level components which are defined within clay, 
and not including the outlined GUI items themselves.

This is due to small differences in the GUI layout between the two platforms; for example the PC version
may have embedded buttons, while the hardware version has physical buttons.
*/

#ifndef GUI_H_ 
#define GUI_H_  

#include <string.h>
#include "clay.h"
#include "gui.h"

const Clay_Color COLOR_LIGHT = (Clay_Color) {224, 215, 210, 255};
const Clay_Color COLOR_RED = (Clay_Color) {168, 66, 28, 255};
const Clay_Color COLOR_ORANGE = (Clay_Color) {225, 138, 50, 255};
const Clay_Color COLOR_BLACK = (Clay_Color) {0, 0, 0, 255};

// // Layout config is just a struct that can be declared statically, or inline
// Clay_LayoutConfig sidebarItemLayout = {
//     .sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(50) },
// };

// // Re-useable components are just normal functions
// inline void SidebarItemComponent() {
//     CLAY(CLAY_LAYOUT(sidebarItemLayout), CLAY_RECTANGLE({ .color = COLOR_ORANGE })) {}
// }


void mainScreen(
    char* pInputText, 
    char* pHexText, 
    char* pDecText, 
    char* pBinText,
    char* pSettingsText,
    uint8_t fontId
    )
{
    // Convert chars to clay strings
    Clay_String inputText = {.chars = pInputText, .length = strlen(pInputText)};
    Clay_String hexText = {.chars = pHexText, .length = strlen(pHexText)};
    Clay_String decText = {.chars = pDecText, .length = strlen(pDecText)};
    Clay_String binText = {.chars = pBinText, .length = strlen(pBinText)};
    Clay_String settingsText = {.chars = pSettingsText, .length = strlen(pSettingsText)};
    CLAY(
        CLAY_ID("OuterContainer"),
        CLAY_LAYOUT({.sizing = {CLAY_SIZING_GROW(0), CLAY_SIZING_GROW(0)}, .padding = CLAY_PADDING_ALL(16), .childGap = 16}),
        CLAY_RECTANGLE({ .color = COLOR_BLACK})
        ){
            CLAY(
                CLAY_ID("SettingsContainer"),
                CLAY_LAYOUT({.sizing = { .width = CLAY_SIZING_GROW(0), .height = CLAY_SIZING_FIXED(20)}}),
                CLAY_RECTANGLE({ .color = COLOR_BLACK})
            ){
                CLAY_TEXT(settingsText, CLAY_TEXT_CONFIG({ .fontId = fontId, .textColor = COLOR_LIGHT}));
            }
        }
}



#endif // GUI_H_