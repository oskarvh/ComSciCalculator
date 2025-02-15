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

/*
 * BRIEF:
 * Contains non-standard functions used by the comscicalc,
 * but not intrinsic to the function of it.
 * For example converting string to fixed point, converting
 * char to int and so on.
 */

// Header file
#include "print_utils.h"

// Included utils
#include "logger.h"

// Standard library
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int charToInt(char c) {
    if ((c >= '0') && (c <= '9')) {
        return (int)(c - '0');
    }
    if ((c >= 'a') && (c <= 'f')) {
        return (int)(c - 'a' + 10);
    }
    return 0;
}

uint64_t strtofp(const char *pString, bool sign, uint16_t decimalPlace,
                 uint8_t radix) {
    const char *pLocalPtr = pString;
    if (radix == 10) {
        char *pDecimalPlace = NULL;
        double temp = strtof(pString, &pDecimalPlace);
        return (uint64_t)(round(temp * (1ULL << decimalPlace)));
    } else if (false) {
        // Problem here.
        // In order to translate the decimal part using strto[u]ll,
        // the string must be zero extended.
        char *pDecimalPlace = NULL;
        char *pEndPtr = NULL;
        uint64_t integerPart = 0;
        uint64_t decimalPart = 0;

        if (sign) {
            integerPart = strtoll(pLocalPtr, &pDecimalPlace, radix);
        } else {
            integerPart = strtoull(pLocalPtr, &pDecimalPlace, radix);
        }
        if (*pDecimalPlace == '.') {
            pDecimalPlace++;
        } else {
            logger(LOGGER_LEVEL_ERROR,
                   "Error: Expected a . in the fixed point string %s\r\n",
                   pLocalPtr);
        }
        decimalPart = strtoull(pDecimalPlace, &pEndPtr, radix);

        return (integerPart << decimalPlace) | decimalPart;
    }
    if (radix == 2) {
        // Go through the integer, then handle the decimal part
        // in a special way.
        char *pDecimalPlace = NULL;
        char *pEndPtr = NULL;
        uint64_t integerPart = 0;
        uint64_t decimalPart = 0;

        if (sign) {
            integerPart = strtoll(pLocalPtr, &pDecimalPlace, radix);
        } else {
            integerPart = strtoull(pLocalPtr, &pDecimalPlace, radix);
        }
        if (*pDecimalPlace == '.') {
            pDecimalPlace++;
        } else {
            logger(LOGGER_LEVEL_ERROR,
                   "Error: Expected a . in the fixed point string %s\r\n",
                   pLocalPtr);
        }
        // Here, loop through the number of decimal places
        // to achieve full padding.
        for (int i = decimalPlace - 1; i >= 0; i--) {
            if (*pDecimalPlace != '\0') {
                char currentChar = *pDecimalPlace++;
                // Convert the current char to binary:
                uint64_t currentVal = (currentChar - '0');
                decimalPart += currentVal << i;
            }
        }
        return (integerPart << decimalPlace) | decimalPart;
    }
    if (radix == 16) {
        // Go through the integer, then handle the decimal part
        // in a special way.
        char *pDecimalPlace = NULL;
        char *pEndPtr = NULL;
        uint64_t integerPart = 0;
        uint64_t decimalPart = 0;

        if (sign) {
            integerPart = strtoll(pLocalPtr, &pDecimalPlace, radix);
        } else {
            integerPart = strtoull(pLocalPtr, &pDecimalPlace, radix);
        }
        if (*pDecimalPlace == '.') {
            pDecimalPlace++;
        } else {
            logger(LOGGER_LEVEL_ERROR,
                   "Error: Expected a . in the fixed point string %s\r\n",
                   pLocalPtr);
        }
        // Here, loop through the number of decimal places
        // to achieve full padding.
        for (int i = decimalPlace - 1; i >= 0; i -= 4) {
            if (*pDecimalPlace != '\0') {
                char currentChar = *pDecimalPlace++;
                // Convert the current char to binary:
                uint64_t currentVal = 0;
                if (currentChar >= '0' && currentChar <= '9')
                    currentVal = currentChar - '0';
                else if (currentChar >= 'a' && currentChar <= 'f')
                    currentVal = currentChar - 'a' + 10;
                else if (currentChar >= 'A' && currentChar <= 'F')
                    currentVal = currentChar - 'A' + 10;
                decimalPart += currentVal << (i - 3ULL);
            }
        }
        return (integerPart << decimalPlace) | decimalPart;
    }
}

void fptostr(char *pString, uint64_t fp, bool sign, uint16_t decimalPlace,
             uint8_t radix) {
    // NULL check on pointer
    if (pString == NULL) {
        return;
    }
    if (radix == 10) {
        // NOTE:
        // There will be some losses converting to decimal.
        double res = 0.0;
        double mult = 0.5;
        uint64_t decPart = fp >> decimalPlace;
        uint64_t mask = 1ULL << (decimalPlace - 1);
        while (mask != 0) {
            if (mask & fp) {
                res += mult;
            }
            mask = mask >> 1;
            mult /= 2.0;
        }
        res += decPart;
        sprintf(pString, "%lg", res);
    }
    if (radix == 2) {
        uint64_t decPart = fp >> decimalPlace;
        uint64_t mask = (1ULL << decimalPlace) - 1;
        uint64_t fractPart = fp & mask;
        // Print the decimal part. Note, this removes leading zeros.
        printToBinary(pString, decPart, false, 64, true);
        // Cannot reuse the printToBinary due to removing leading zeros,
        // so just run through the rest.
        // First, find where the pString ends:
        char *pStringFractPart = pString;
        while (*pStringFractPart != '\0') {
            pStringFractPart++;
        }
        *pStringFractPart++ = '.';
        if (fractPart != 0) {
            uint8_t numFractWritten = 0;
            for (int i = decimalPlace - 1; i >= 0; i--) {
                // Get the bit at the current index.
                if (numFractWritten > 1 && numFractWritten % 4 == 0) {
                    *pStringFractPart++ = ' ';
                }
                uint64_t mask = 1ULL << i;
                uint64_t currentBit = (fractPart & mask) >> (i);
                uint64_t bitsLeftMask = (1ULL << i) - 1;
                *pStringFractPart++ = '0' + currentBit;
                if ((fractPart & bitsLeftMask) == 0) {
                    break;
                }
                numFractWritten++;
            }
            *pStringFractPart = '\0';
        } else {
            // If the result is 0, then print a 0
            *pStringFractPart++ = '0';
            // Top it off with a null char:
            *pStringFractPart = '\0';
        }
    }
    if (radix == 16) {
        uint64_t decPart = fp >> decimalPlace;
        uint64_t mask = (1ULL << decimalPlace) - 1;
        uint64_t fractPart = fp & mask;
        // Print the decimal part using sprintf:
        sprintf(pString, "0x%llX", decPart);
        // Find the place of and add decimal point:
        char *pStringFractPart = pString;
        while (*pStringFractPart != '\0') {
            pStringFractPart++;
        }
        *pStringFractPart++ = '.';
        // Now the fun starts.
        // Each hex number is 4 bits

        if (fractPart != 0) {
            for (int i = decimalPlace - 1; i >= 0; i -= 4) {
                // Get the bit at the current index.
                uint64_t mask = 0xfULL << i - 3;
                uint64_t currentBits = (fractPart & mask);
                uint64_t bitsLeftMask = (1ULL << (i - 3)) - 1;
                sprintf(pStringFractPart++, "%llX", currentBits);
                if ((fractPart & bitsLeftMask) == 0) {
                    break;
                }
            }
        } else {
            // If the result is 0, then print a 0
            *pStringFractPart++ = '0';
        }
        // Top it off with a null char:
        *pStringFractPart = '\0';
    }
}

void printToBinary(char *pBuf, uint64_t num, bool printAllBits, uint8_t numBits,
                   bool print0b) {
    // Allocate a temporary buffer to hold the converted
    // chars:
    char *pTempBuf = malloc(numBits + 2);

    // In order to print the buffer with non-leading zeros,
    // the last index must be the least significant bit.
    // The approach is to go through each bit, starting from
    // the MSB, find the first non-zero bit and start printing
    // that.
    bool firstBitFound = printAllBits;
    int16_t index = 0;
    if (num != 0) {
        for (int i = numBits - 1; i >= 0; i--) {
            // Get the bit at the current index.
            uint64_t mask = 1ULL << i;
            uint64_t currentBit = (num & mask) >> i;
            // If the current bit is 1, and the first bit
            // hasn't been found, then set the firstBitFound
            // as true, causing the string to start printing
            if (!firstBitFound && currentBit == 1) {
                firstBitFound = true;
            }
            if (firstBitFound) {
                pTempBuf[index++] = currentBit + '0';
            }
        }
    } else {
        // If the result is 0, then print a 0
        pTempBuf[index++] = '0';
    }
    // Reduce the index by one since we didn't write the last one
    index--;

    // Go through the tempbuf backwards and add a space
    // every for chars, along with copying over the temporary
    // buffer to the final buffer.

    // Find the final buffer size, to find the starting index.
    // This is then the number of chars written, plus '0b' and
    // all spaces
    int16_t bufIdx = index + 1; // all 0,1 chars and \0.
    if (print0b) {
        // Add two slots for "0b"
        bufIdx += 2;
    }
    // Add due to spaces:
    uint8_t charsWritten = 0;
    for (int16_t i = index; i >= 0; i--) {
        if ((charsWritten % 4 == 0) && (charsWritten > 1)) {
            bufIdx++;
        }
        charsWritten++;
    }

    // First, set the null terminator:
    pBuf[bufIdx--] = '\0';
    charsWritten = 0;
    for (; index >= 0; index--) {
        if (((charsWritten) % 4 == 0) && (charsWritten > 1)) {
            pBuf[bufIdx--] = ' ';
        }
        pBuf[bufIdx--] = pTempBuf[index];

        charsWritten++;
    }
    // bufIds should now be 1, meaning there are two
    // spaces left for "0b". Check that it's true, and then add '0b'
    if (print0b) {
        if (bufIdx != 1) {
            logger(LOGGER_LEVEL_ERROR,
                   "ERROR: binary conversion didn't add up. bufIdx: %i\r\n",
                   bufIdx);
        }
        pBuf[0] = '0';
        pBuf[1] = 'b';
    }
    free(pTempBuf);
}