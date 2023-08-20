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
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/**
 * @brief Function that converts a char to corresponding int (dec or bin)
 * @param c Char to be converted
 * @return 0 if not possible, but input function already checks for this.
 *   Otherwise returns the int in decimal base.
 */
int charToInt(char c);

/**
 * @brief Convert string to fixed point notation.
 * @param pString String to be converted
 * @param sign True if conversion should be signed, false if unsigned
 * @param decimalPlace Number of bits that represent the fractional bits
 * @param radix Radix of conversion, 10 is decimal, 2 is binary, 16 is hex.
 * @return A 64 bit value representation
 * @warning Conversion from decimal to fixed point is lossy and inacurate
 */
uint64_t strtofp(const char *pString, bool sign, uint16_t decimalPlace,
                 uint8_t radix);

/**
 * @brief Convert fixed point to string.
 * @param pString String to be written to. Ensure that length is adequate
 * @param sign True if conversion should be signed, false if unsigned
 * @param decimalPlace Number of bits that represent the fractional bits
 * @param radix Radix of conversion, 10 is decimal, 2 is binary, 16 is hex.
 * @warning Printing fixed point to decimal is lossy and inaccurate.
 */
void fptostr(char *pString, uint64_t fp, bool sign, uint16_t decimalPlace,
             uint8_t radix);

/**
 * @brief Prints a 64 bit binary number to string.
 * @param pBuf String to be written to. Should at least be num bits+3 long.
 * @param num 64 bit number that is converted to string.
 * @param printAllBits Prints leading zeros as well.
 * @param numBits Number of bits
 * @note This function does *not* print leading zeros
 */
void printToBinary(char *pBuf, uint64_t num, bool printAllBits,
                   uint8_t numBits);