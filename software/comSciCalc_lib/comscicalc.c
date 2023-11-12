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
 * Copyright (c) 2023
 * Oskar von Heideken.
 *
 * Computer Scientist Calculator (comscicalc) C library
 *
 * This file contains functions to run the computer scientist calculator
 * The main kernel takes a character input, and based on the input settings
 * calculates the result dynamically.
 *
 * Input options:
 * - Decimal, hexadecimal or binary input mode (inputBase_t inputBase)
 * - Signed or unsigned entry (bool inputSigned)
 * - IEEE 754 float input mode (bool inputFloat)
 *
 * Output options:
 * - All bases (hex, dec and bin) is always available
 * - Signed or unsigned (bool outputSigned)
 * - IEEE 754 float (bool outputFloat)
 *
 */

/* ----------------- HEADERS ----------------- */
// comsci header file
#include "comscicalc.h"

// utils
#include "print_utils.h"
#include "uart_logger.h"

// Standard library
#include <math.h>
#include <stdio.h>
#include <string.h>

/* ------------- GLOBAL VARIABLES ------------ */
const uint8_t baseToRadix[3] = {
    10, // inputBase_DEC
    16, // inputBase_HEX
    2,  // inputBase_BIN
};
/* ---- CALCULATOR CORE HELPER FUNCTIONS ----- */

// Temporary list containing the allocated pointers
#define ALLOC_TABLE_SZ 400
uint32_t allocatedPointers[ALLOC_TABLE_SZ] = {0};

/**
 * @brief Malloc wrapper to help debug memory leaks
 * @param size Size of malloc
 * @return Pointer that malloc allocates
 */
inputListEntry_t *overloaded_malloc(size_t size) {
    void *ptr = malloc(size);
    if (ptr == NULL) {
        logger(LOGGER_LEVEL_ERROR, "Malloc returned NULL");
        while (1)
            ;
    }
    logger(LOGGER_LEVEL_INFO, "[allocated] : 0x%08x\r\n", ptr);
    // Loop through until an empty place is found
    uint32_t i = 0;
    while (allocatedPointers[i] != 0) {
        i++;
    }
    allocatedPointers[i] = (uint32_t)ptr;
    return ptr;
}

/**
 * @brief Free wrapper to help debug memory leaks
 * @param ptr Pointer to memory being free'd
 */
void overloaded_free(inputListEntry_t *ptr) {
    logger(LOGGER_LEVEL_INFO, "[free] : 0x%08x\r\n", ptr);
    bool okToFree = false;
    for (int i = 0; i < ALLOC_TABLE_SZ; i++) {
        if (allocatedPointers[i] == (uint32_t)ptr) {
            okToFree = true;
            allocatedPointers[i] = 0;
            break;
        }
    }
    if (!okToFree) {
        logger(LOGGER_LEVEL_ERROR,
               "Could not find that 0x%08x was allocated! \r\n", ptr);

        while (1)
            ;
    }
    free(ptr);
}

/**
 * @brief Function to check if char is numerical
 * @param base Hex, dec or bin base.
 * @param c Character to check.
 * @return True if char is numerical within that base. Otherwise false.
 *
 */
static bool charIsNumerical(inputBase_t base, char c) {
    if (base == inputBase_DEC) {
        if (('0' <= c) && (c <= '9')) {
            return true;
        }
    }
    if (base == inputBase_HEX) {
        if ((('0' <= c) && (c <= '9')) || (('a' <= c) && (c <= 'f'))) {
            return true;
        }
    }
    if (base == inputBase_BIN) {
        if (('0' <= c) && (c <= '1')) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Function to check if char is an operator
 * @param c Character to check.
 * @return True if char is in the operator table. Otherwise false.
 */
static bool charIsOperator(char c) {
    // Loop through the operator array and check if the operator is in there.
    // Not a nice way to do it, but the array is fairly small.
    for (uint8_t i = 0; i < NUM_OPERATORS; i++) {
        if (c == operators[i].inputChar) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Function to check if char is a bracket.
 * @param c Character to check.
 * @return True if char is opening or closing bracket. Otherwise false.
 */
static bool charIsBracket(char c) {
    if ((c == '(') || (c == ')')) {
        return true;
    }
    return false;
}

/**
 * @brief Function to check if char is other accepted input.
 * @param c Character to check.
 * @return True if char is accepted input, but not operator or numerical.
 * Otherwise false.
 */
static bool charIsOther(char c) {
    if ((c == ',') || (c == '.')) {
        return true;
    }
    return false;
}

/**
 * @brief Function get the operator entry based on input char.
 * @param c Character which to fetch related operator entry.
 * @return Pointer to operator entry if found, otherwise NULL.
 */
static const operatorEntry_t *getOperator(char c) {
    // Loop through the operator array and check if the operator is in there.
    // Not a nice way to do it, but the array is fairly small.
    for (uint8_t i = 0; i < NUM_OPERATORS; i++) {
        if (c == operators[i].inputChar) {
            // Operator entry found!
            return &operators[i];
        }
    }
    return NULL;
}

/**
 * @brief Function get the list entry based on cursor position.
 * @param calcCoreState Pointer to the core state.
 * @param ppInputListAtCursor Pointer to pointer of the list entry at the
 * cursor.
 * @return Status of finding a list entry at the cursor value, or how much
 * shorter that list is than the cursor.
 *
 * The main goal of this function is to find the list entry at the cursor value.
 * It does that by finding the end of the list, and then traversing backwards.
 * Each modifiable entry has one entry in the list, so traversing is easy.
 * Note that the cursor can traverse the end of the list, in which case we
 * return the number of steps the cursor had left before hitting the start. This
 * always returns the list AT the cursor, and since only backspace is available,
 * that means that the return points to the entry just before the cursor.
 */
static inputModStatus_t
getInputListEntry(calcCoreState_t *calcCoreState,
                  inputListEntry_t **ppInputListAtCursor) {
    uint8_t cursorPosition = calcCoreState->cursorPosition;
    inputListEntry_t *pListEntry = calcCoreState->pListEntrypoint;

    // Check pointer to input list
    if (pListEntry != NULL) {
        // Find the pointer to the last list entry
        while (pListEntry->pNext != NULL) {
            pListEntry = pListEntry->pNext;
        }

        // Prevent wrap-around issues
        if (((int8_t)cursorPosition) < 0) {
            return inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY;
        }

        // Start going backwards.
        for (uint8_t i = 0; i < cursorPosition; i++) {
            // Check if there is a previous entry
            if (pListEntry != NULL) {
                pListEntry = pListEntry->pPrevious;
            } else {
                // If there isn't then return the number
                // of steps taken in this list.
                *ppInputListAtCursor = pListEntry;
                return i;
            }
        }
    }
    // Set the return values and return the state.
    *ppInputListAtCursor = pListEntry;

    return inputModStatus_SUCCESS;
}

/* -------- CALCULATOR CORE FUNCTIONS -------- */

calc_funStatus_t calc_coreInit(calcCoreState_t *pCalcCoreState) {
    // Check the calc code pointer
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    for (int i = 0; i < 100; i++) {
        allocatedPointers[i] = 0;
    }

    // Initialize the cursor to 0
    pCalcCoreState->cursorPosition = 0;

    // Set the input base to NONE
    pCalcCoreState->numberFormat.inputBase = inputBase_NONE;

    // Set the first pointer to NULL
    pCalcCoreState->pListEntrypoint = NULL;

    // Set the allocation counter to 0
    pCalcCoreState->allocCounter = 0;

    // Set the result to 0 and solved to false
    pCalcCoreState->result = 0;
    pCalcCoreState->solved = false;
    pCalcCoreState->numberFormat.inputFormat = INPUT_FMT_INT;
    pCalcCoreState->numberFormat.outputFormat = INPUT_FMT_INT;
    pCalcCoreState->numberFormat.sign = false;
    pCalcCoreState->numberFormat.numBits = 64;
    pCalcCoreState->numberFormat.fixedPointDecimalPlace = 32;

    return calc_funStatus_SUCCESS;
}

calc_funStatus_t calc_coreBufferTeardown(calcCoreState_t *pCalcCoreState) {
    // Check the calc code pointer
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }

    // Get the pointer to the list entry.
    inputListEntry_t *pListEntry = pCalcCoreState->pListEntrypoint;

    // List entry is allowed to be NULL as well
    if (pListEntry != NULL) {

        // Find the first entry, if this isn't it.
        while ((pListEntry->pPrevious) != NULL) {
            pListEntry = (inputListEntry_t *)(pListEntry->pPrevious);
        }

        // Go from start to finish and free all entries
        while (pListEntry != NULL) {
            // Free the list entry
            inputListEntry_t *pNext = (inputListEntry_t *)pListEntry->pNext;
            overloaded_free(pListEntry);
            pCalcCoreState->allocCounter--;
            pListEntry = pNext;
        }
    }

    // Note: We should not free the calcCoreState.
    return calc_funStatus_SUCCESS;
}

calc_funStatus_t calc_addInput(calcCoreState_t *pCalcCoreState,
                               char inputChar) {

    // Check pointer to calculator core state
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;

    // Get the current list and string entries based on the cursor.
    inputListEntry_t *pCurrentListEntry;
    inputModStatus_t listState =
        getInputListEntry(pCalcCoreState, &pCurrentListEntry);

    if (listState > 0) {
        // Cursor went too far. TBD should this be recified here?
        pCalcCoreState->cursorPosition = (uint8_t)listState;
    }

    // Allocate a new entry
    inputListEntry_t *pNewListEntry =
        overloaded_malloc(sizeof(inputListEntry_t));
    if (pNewListEntry == NULL) {
        return calc_funStatus_ALLOCATE_ERROR;
    }
    pCalcCoreState->allocCounter++;
    pNewListEntry->pFunEntry = NULL;

    // Add the input
    pNewListEntry->entry.c = inputChar;
    inputFormat_t inputFormat = pCalcCoreState->numberFormat.inputFormat;
    bool sign = pCalcCoreState->numberFormat.sign;
    // Set the input subresult to 0
    pNewListEntry->entry.subresult = 0;
    if (charIsNumerical(pCalcCoreState->numberFormat.inputBase, inputChar)) {
        pNewListEntry->entry.typeFlag =
            CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                               DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);

    } else if (charIsOperator(inputChar)) {
        // Get the operator
        const operatorEntry_t *pOp = getOperator(inputChar);
        if (pOp->bIncDepth) {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_INCREASE, INPUT_TYPE_OPERATOR);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_OPERATOR);
        }
        pNewListEntry->pFunEntry = (void *)pOp;

    } else if (charIsBracket(inputChar)) {
        if (inputChar == OPENING_BRACKET) {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_INCREASE, INPUT_TYPE_EMPTY);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_DECREASE, INPUT_TYPE_EMPTY);
        }
    } else if (charIsOther(inputChar)) {
        if (inputChar == '.') {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_DECIMAL_POINT);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_EMPTY);
        }
    } else {
        // Unknown input. Free and return
        if (pNewListEntry != NULL) {
            overloaded_free(pNewListEntry);
            pCalcCoreState->allocCounter--;
        }
        return calc_funStatus_UNKNOWN_INPUT;
    }

    // Add the current input base. Note: base change and propagation not handled
    // here
    pNewListEntry->inputBase = pCalcCoreState->numberFormat.inputBase;
    logger(LOGGER_LEVEL_INFO, "Adding %c with input base %i \r\n",
           pNewListEntry->entry.c, pNewListEntry->inputBase);
    // Add the new list entry to the correct place in the list
    if (pCurrentListEntry == NULL) {
        // Top of the list. Add new entry before, and change the list entry
        // point
        pNewListEntry->pNext = pCalcCoreState->pListEntrypoint;
        pNewListEntry->pPrevious = NULL;
        if (pNewListEntry->pNext != NULL) {
            ((inputListEntry_t *)(pNewListEntry->pNext))->pPrevious =
                pNewListEntry;
        }
        pCalcCoreState->pListEntrypoint = pNewListEntry;
    } else {
        // Add entry after the current entry
        pNewListEntry->pPrevious = pCurrentListEntry;
        pNewListEntry->pNext = pCurrentListEntry->pNext;
        if (pNewListEntry->pNext != NULL) {
            ((inputListEntry_t *)(pNewListEntry->pNext))->pPrevious =
                pNewListEntry;
        }
        pCurrentListEntry->pNext = pNewListEntry;
    }
    return calc_funStatus_SUCCESS;
}

calc_funStatus_t calc_removeInput(calcCoreState_t *pCalcCoreState) {
    // Check pointer to calculator core state
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;

    // Get the current list and string entries based on the cursor.
    inputListEntry_t *pCurrentListEntry;
    inputModStatus_t listState =
        getInputListEntry(pCalcCoreState, &pCurrentListEntry);

    // If the current list entry pointer is NULL,
    // then we have either added nothing, or we're at the
    // head of the list.
    if (pCurrentListEntry == NULL) {
        // We cannot do anything, as we cannot remove anything!
        return calc_funStatus_INPUT_LIST_NULL;
    }

    // The aim is to remove the pointer currently pointed at,
    // so first align the pointers of previous and next entires
    // and then free the memory.
    if (pCurrentListEntry->pNext != NULL) {
        ((inputListEntry_t *)(pCurrentListEntry->pNext))->pPrevious =
            pCurrentListEntry->pPrevious;
    }
    if (pCurrentListEntry->pPrevious != NULL) {
        ((inputListEntry_t *)(pCurrentListEntry->pPrevious))->pNext =
            pCurrentListEntry->pNext;
    }
    if (pCurrentListEntry->pPrevious == NULL) {
        // If the previous pointer was NULL, this was the head of the
        // list. Align the entry point.
        pCalcCoreState->pListEntrypoint = pCurrentListEntry->pNext;
    }

    overloaded_free(pCurrentListEntry);
    pCalcCoreState->allocCounter--;

    return calc_funStatus_SUCCESS;
}

/**
 * @brief Functions that find the deepest point between two list entries.
 * @param ppStart Pointer to pointer to start of list.
 * @param ppEnd Pointer to pointer to end of list.
 * @return Status of finding the deepest point.
 *   -1 if deepest point cannot be found, otherwise 0.
 *
 * Pointers to pointers seturns the pointer to the
 * start and end of the deepest point.
 * Works by increasing a counter when the depth increases,
 * and for each consecutive depth increase updates the start
 * pointer, until a depth decrease is found.
 * It always returns the first deepest point.
 * From that starting point, it then locates the next
 * depth decrease, which would be the end.
 */
int8_t findDeepestPoint(inputListEntry_t **ppStart, inputListEntry_t **ppEnd) {
    // Counter to keep track of the current depth.
    int currentDepth = 0;
    int deepestDepth = 0;

    // Local variables to keep track of the start and end point.
    // Set the start and end points to start and end of the buffer,
    inputListEntry_t *pStart = *ppStart;
    inputListEntry_t *pEnd = *ppEnd;
    inputListEntry_t *pIter = *ppStart;

    // Loop until the end of the list has been found
    while (pIter != NULL) {
        if (GET_DEPTH_FLAG(pIter->entry.typeFlag) == DEPTH_CHANGE_INCREASE) {
            // Increase in depth. Increase the current depth
            currentDepth++;

            if (currentDepth > deepestDepth) {
                // This is the new deepest point
                deepestDepth = currentDepth;
                pStart = pIter;
            }
        } else if (GET_DEPTH_FLAG(pIter->entry.typeFlag) ==
                   DEPTH_CHANGE_DECREASE) {
            // Decrease in depth
            currentDepth--;
        }
        pIter = pIter->pNext;
    }

    // If the depth doesn't match up, this cannot be solved.
    if (currentDepth != 0) {
        return -1;
    }

    // Set the pointer to the end to the pointer to the start
    pEnd = pStart;
    // ... and loop through to the next decrease, or end of the list.
    while (pEnd != NULL) {
        if (GET_DEPTH_FLAG(pEnd->entry.typeFlag) == DEPTH_CHANGE_DECREASE) {
            break;
        }
        pEnd = pEnd->pNext;
    }

    // Record the values and return
    *ppStart = pStart;
    *ppEnd = pEnd;
    return 0;
}

void convertResult(char *pString, SUBRESULT_INT result,
                   numberFormat_t *pNumberFormat, uint8_t base) {
    // NULL check on pointer
    if (pString == NULL) {
        return;
    }
    // Based on the result format, convert to the appropriate format.
    if (pNumberFormat->outputFormat == INPUT_FMT_INT) {
        // Output format is integer. Convert based on input format.
        if (pNumberFormat->inputFormat == INPUT_FMT_INT) {
            // Input format is integer, meaning that the result is
            // on integer format as well.
            // Simply print long long.
            if (base == inputBase_DEC) {
                sprintf(pString, "%lli", result);
            } else if (base == inputBase_BIN) {
                printToBinary(pString, result, false, pNumberFormat->numBits,
                              true);
            } else if (base == inputBase_HEX) {
                sprintf(pString, "0x%llX", result);
            }
        } else if (pNumberFormat->inputFormat == INPUT_FMT_FLOAT) {
            // Floating point format. First convert to either float or double,
            // based on the number format, and then round and print.
            SUBRESULT_INT tmpResInt = 0;
            if (pNumberFormat->numBits == 32) {
                float tmpRes = 0.0;
                // Copy the result over to float to keep formatting
                memcpy(&tmpRes, &result, sizeof(float));
                // Round to integer:
                tmpResInt = round(tmpRes);
            } else if (pNumberFormat->numBits == 64) {
                double tmpRes = 0.0;
                // Copy the result over to double to keep formatting
                memcpy(&tmpRes, &result, sizeof(double));
                // Round to integer:
                tmpResInt = round(tmpRes);
            }
            if (base == inputBase_DEC) {
                sprintf(pString, "%1lli", tmpResInt);
            } else if (base == inputBase_BIN) {
                printToBinary(pString, tmpResInt, false, pNumberFormat->numBits,
                              true);
            } else if (base == inputBase_HEX) {
                sprintf(pString, "0x%1llX", tmpResInt);
            }

        } else if (pNumberFormat->inputFormat == INPUT_FMT_FIXED) {
            // Just truncate.
            SUBRESULT_INT decimalPlace = pNumberFormat->fixedPointDecimalPlace;
            SUBRESULT_INT tmpRes = result >> ((SUBRESULT_INT)decimalPlace);
            if (base == inputBase_DEC) {
                sprintf(pString, "%1lli", tmpRes);
            } else if (base == inputBase_BIN) {
                printToBinary(pString, tmpRes, false, pNumberFormat->numBits,
                              true);
            } else if (base == inputBase_HEX) {
                sprintf(pString, "0x%1llX", tmpRes);
            }
        }
    } else if (pNumberFormat->outputFormat == INPUT_FMT_FLOAT) {
        // Output format is floating point. Convert based on input format.
        if (pNumberFormat->inputFormat == INPUT_FMT_INT) {
            // Convert integer to floating point format:
            // Using only double to be sure.
            if (base == inputBase_DEC) {
                if (pNumberFormat->numBits == 32) {
                    float tmpRes = (float)result;
                    sprintf(pString, "%g.0", tmpRes);
                } else if (pNumberFormat->numBits == 64) {
                    double tmpRes = (double)result;
                    sprintf(pString, "%lg.0", tmpRes);
                }
            } else if (base == inputBase_BIN) {
                SUBRESULT_INT tmpRes = 0;
                if (pNumberFormat->numBits == 32) {
                    float tmpResf = (float)result;
                    memcpy(&tmpRes, &tmpResf, sizeof(float));
                    // Here, we want to print all bits to make it
                    // easier to read

                } else if (pNumberFormat->numBits == 64) {
                    double tmpResd = (double)result;
                    memcpy(&tmpRes, &tmpResd, sizeof(double));
                }
                printToBinary(pString, tmpRes, true, pNumberFormat->numBits,
                              true);
            } else if (base == inputBase_HEX) {
                if (pNumberFormat->numBits == 32) {
                    float tmpResf = (float)result;
                    SUBRESULT_INT tmpRes = 0;
                    memcpy(&tmpRes, &tmpResf, sizeof(float));
                    sprintf(pString, "0x%1lX", tmpRes);
                } else if (pNumberFormat->numBits == 64) {
                    double tmpResd = (double)result;
                    SUBRESULT_INT tmpRes;
                    memcpy(&tmpRes, &tmpResd, sizeof(double));
                    sprintf(pString, "0x%1llX", tmpRes);
                }
            }
        } else if (pNumberFormat->inputFormat == INPUT_FMT_FLOAT) {
            // Input is float and output is float. Just make sure
            // to keep track of bit width.
            if (base == inputBase_DEC) {
                if (pNumberFormat->numBits == 32) {
                    float tmpRes = 0.0;
                    memcpy(&tmpRes, &result, sizeof(float));
                    sprintf(pString, "%lg", tmpRes);
                } else if (pNumberFormat->numBits == 64) {
                    double tmpRes = 0.0;
                    memcpy(&tmpRes, &result, sizeof(double));
                    sprintf(pString, "%lg", tmpRes);
                }
            } else if (base == inputBase_BIN) {
                // Nothing special, just print as is.
                printToBinary(pString, result, true, pNumberFormat->numBits,
                              true);
            } else if (base == inputBase_HEX) {
                // Nothing special, just print as is.
                sprintf(pString, "0x%1llX", result);
            }
        } else if (pNumberFormat->inputFormat == INPUT_FMT_FIXED) {
            // Fixed point to floating point conversion.
            if (base == inputBase_DEC) {
                // For decimal, this is just a straight forward printing,
                // as there are no differences between floating point
                // or fixed point (actually, float is used in the conversion
                // from fixed point, so this gives the same format. )
                fptostr(pString, result, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 10);
            } else {
                // Here though, a conversion from fixed point to float is
                // required.
                uint16_t decimalPlace = pNumberFormat->fixedPointDecimalPlace;
                uint64_t decPart = result >> decimalPlace;
                uint64_t mask = (1ULL << decimalPlace) - 1;
                uint64_t tmpRes = 0;
                if (pNumberFormat->numBits == 32) {
                    float res = 0.0;
                    float mult = 0.5;
                    mask = 1ULL << (decimalPlace - 1);
                    while (mask != 0) {
                        if (mask & result) {
                            res += mult;
                        }
                        mask = mask >> 1;
                        mult /= 2.0;
                    }
                    res += decPart;
                    memcpy(&tmpRes, &res, sizeof(float));
                } else if (pNumberFormat->numBits == 64) {
                    double res = 0.0;
                    double mult = 0.5;
                    mask = 1ULL << (decimalPlace - 1);
                    while (mask != 0) {
                        if (mask & result) {
                            res += mult;
                        }
                        mask = mask >> 1;
                        mult /= 2.0;
                    }
                    res += decPart;
                    memcpy(&tmpRes, &res, sizeof(double));
                }
                if (base == inputBase_BIN) {
                    printToBinary(pString, tmpRes, true, pNumberFormat->numBits,
                                  true);
                } else if (base == inputBase_HEX) {
                    sprintf(pString, "0x%1llX", tmpRes);
                }
            }
        }
    } else if (pNumberFormat->outputFormat == INPUT_FMT_FIXED) {
        // Output format is fixed point. Convert based on input format.
        if (pNumberFormat->inputFormat == INPUT_FMT_INT) {
            // Integer to fixed point conversion. Just shift by decimal place.
            SUBRESULT_INT tmpRes = result
                                   << (pNumberFormat->fixedPointDecimalPlace);
            if (base == inputBase_DEC) {
                // Hack: integer to fixed point will always result in xxx.0
                sprintf(pString, "%1lli.0", result);
            } else if (base == inputBase_BIN) {
                printToBinary(pString, result, false, pNumberFormat->numBits,
                              true);
                strcat(pString, ".0\0");
            } else if (base == inputBase_HEX) {
                sprintf(pString, "0x%1llX.0", result);
            }
        } else if (pNumberFormat->inputFormat == INPUT_FMT_FLOAT) {
            // This requires floating point to fixed point conversion.
            // Start by constructing the integer and fractional parts
            SUBRESULT_INT fp_res = 0;
            if (pNumberFormat->numBits == 32) {
                float tmpResf = 0.0;
                memcpy(&tmpResf, &result, sizeof(float));
                SUBRESULT_INT decPart = floor(tmpResf);
                float fractPart = tmpResf - floor(tmpResf);
                uint16_t decimalPlace = pNumberFormat->fixedPointDecimalPlace;
                fp_res = decPart << decimalPlace;
                float mult = 0.5;

                for (int i = decimalPlace - 1; i >= 0; i--) {
                    if (fractPart >= mult) {
                        fp_res |= 1ULL << i;
                        fractPart -= mult;
                    }
                    mult /= 2.0;
                }

            } else if (pNumberFormat->numBits == 64) {
                double tmpResf = 0.0;
                memcpy(&tmpResf, &result, sizeof(double));
                SUBRESULT_INT decPart = floor(tmpResf);
                double fractPart = tmpResf - floor(tmpResf);
                uint16_t decimalPlace = pNumberFormat->fixedPointDecimalPlace;
                fp_res = decPart << decimalPlace;
                double mult = 0.5;

                for (int i = decimalPlace - 1; i >= 0; i--) {
                    if (fractPart >= mult) {
                        fp_res |= 1ULL << i;
                        fractPart -= mult;
                    }
                    mult /= 2.0;
                }
            }
            if (base == inputBase_DEC) {
                // For decimal, this is just a straight forward printing,
                // as there are no differences between floating point
                // or fixed point (actually, float is used in the conversion
                // from fixed point, so this gives the same format. )
                fptostr(pString, fp_res, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 10);
            } else if (base == inputBase_BIN) {
                fptostr(pString, fp_res, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 2);
            } else if (base == inputBase_HEX) {
                fptostr(pString, fp_res, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 16);
            }
        } else if (pNumberFormat->inputFormat == INPUT_FMT_FIXED) {
            // Fixed point to fixed point conversion. Just print the result.
            if (base == inputBase_DEC) {
                fptostr(pString, result, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 10);
            } else if (base == inputBase_BIN) {
                fptostr(pString, result, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 2);
            } else if (base == inputBase_HEX) {
                fptostr(pString, result, pNumberFormat->sign,
                        pNumberFormat->fixedPointDecimalPlace, 16);
            }
        }
    }
}

/**
 * @brief Converts an input list containing chars, and converts all chars to
 * appropriate format
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @param ppSolverListStart Pointer to pointer to start of list.
 * @return Status of the conversion.
 *
 * The conversion from char to int will reduce the length of the list, where
 * e.g. '1'->'2'->'3' will be converted to 123 (from 3 entries to 1).
 */
static calc_funStatus_t
copyAndConvertList(calcCoreState_t *pCalcCoreState,
                   inputListEntry_t **ppSolverListStart) {

    inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;
    // If no input list, simply return NULL
    if (pCurrentListEntry == NULL) {
        return calc_funStatus_INPUT_LIST_NULL;
    }
    inputListEntry_t *pNewListEntry = NULL;
    inputListEntry_t *pPreviousListEntry = NULL;
    // Loop through the input list and allocate new
    // instances.
    while (pCurrentListEntry != NULL) {

        // Allocate a new entry
        pNewListEntry = overloaded_malloc(sizeof(inputListEntry_t));
        if (pNewListEntry == NULL) {
            return calc_funStatus_ALLOCATE_ERROR;
        }
        pCalcCoreState->allocCounter++;
        // Copy all parameters over
        memcpy(pNewListEntry, pCurrentListEntry, sizeof(inputListEntry_t));

        // Check if this is the start of the list, in which case save
        // the parameter.
        if (pCurrentListEntry == pCalcCoreState->pListEntrypoint) {
            *ppSolverListStart = pNewListEntry;
        }
        if (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
            INPUT_TYPE_NUMBER) {
            // If the current entry is numerical, aggregate this
            // until the entry is either NULL or not numerical.
            // Note that we cannot start with a decimal point here.
            inputFormat_t inputFormat =
                GET_FMT_TYPE(pCurrentListEntry->entry.typeFlag);
            uint8_t inputBase = pCurrentListEntry->inputBase;
            bool sign = pCalcCoreState->numberFormat.sign;
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_INT,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
            pNewListEntry->entry.subresult = 0;

            uint8_t numberOfNumberEntries = 0;
            inputListEntry_t *pTmpEntry = pCurrentListEntry;
            while ((GET_INPUT_TYPE(pTmpEntry->entry.typeFlag) ==
                    INPUT_TYPE_NUMBER) ||
                   (GET_INPUT_TYPE(pTmpEntry->entry.typeFlag) ==
                    INPUT_TYPE_DECIMAL_POINT)) {
                // Count how manu number and decimal entries there are
                numberOfNumberEntries += 1;
                pTmpEntry = pTmpEntry->pNext;

                if (pTmpEntry == NULL) {
                    break;
                }
            }
            // Allocate a string of the length we just found (+1 for the null
            // terminator)
            char *pCurrentString =
                malloc(sizeof(char) * (numberOfNumberEntries + 1));
            memset(pCurrentString, 0, sizeof(char) * numberOfNumberEntries);

            if (pCurrentListEntry == NULL) {
                return calc_funStatus_ALLOCATE_ERROR;
            }
            char *pCurrentChar = pCurrentString;
            // Time to copy those entries over to the string
            while ((GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
                    INPUT_TYPE_NUMBER) ||
                   (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
                    INPUT_TYPE_DECIMAL_POINT)) {
                // Copy the current character to the string
                *pCurrentChar++ = pCurrentListEntry->entry.c;

                pCurrentListEntry = pCurrentListEntry->pNext;

                if (pCurrentListEntry == NULL) {
                    break;
                }
            }
            // Finally, cap it off with a null terminator
            *pCurrentChar = '\0';

            // Now that we have a string to work with, based on the input
            // format and base, we can convert using the UNIX string-to-X
            // functions.
            char *endPtr = NULL;
            if (inputBase == inputBase_DEC) {
                if (inputFormat == INPUT_FMT_INT) {
                    // Convert string to int.
                    // TBD: I think this should work with shorter strings as
                    // well
                    if (sign) {
                        pNewListEntry->entry.subresult =
                            strtoll(pCurrentString, &endPtr, 10);
                    } else {
                        pNewListEntry->entry.subresult =
                            strtoull(pCurrentString, &endPtr, 10);
                    }
                } else if (inputFormat == INPUT_FMT_FLOAT) {
                    if (pCalcCoreState->numberFormat.numBits == 32) {
                        float tempFloat = strtof(pCurrentString, &endPtr);
                        memcpy(&(pNewListEntry->entry.subresult), &tempFloat,
                               sizeof(float));
                    }
                    if (pCalcCoreState->numberFormat.numBits == 64) {
                        double tempFloat = strtod(pCurrentString, &endPtr);
                        memcpy(&(pNewListEntry->entry.subresult), &tempFloat,
                               sizeof(double));
                    }
                } else if (inputFormat == INPUT_FMT_FIXED) {
                    // Use function to convert to fixed point with radix 10
                    pNewListEntry->entry.subresult = strtofp(
                        pCurrentString, sign,
                        pCalcCoreState->numberFormat.fixedPointDecimalPlace,
                        10);
                }
            } else if (inputBase == inputBase_HEX) {
                if (inputFormat == INPUT_FMT_INT) {
                    if (sign) {
                        pNewListEntry->entry.subresult =
                            strtoll(pCurrentString, &endPtr, 16);
                    } else {
                        pNewListEntry->entry.subresult =
                            strtoull(pCurrentString, &endPtr, 16);
                    }
                } else if (inputFormat == INPUT_FMT_FLOAT) {
                    // Floats have no specific format in hex, so just
                    // read out as int, but note the difference between 32 and
                    // 64 bits
                    if (pCalcCoreState->numberFormat.numBits == 32) {
                        pNewListEntry->entry.subresult =
                            strtoul(pCurrentString, &endPtr, 16);
                    } else if (pCalcCoreState->numberFormat.numBits == 64) {
                        pNewListEntry->entry.subresult =
                            strtoull(pCurrentString, &endPtr, 16);
                    }
                } else if (inputFormat == INPUT_FMT_FIXED) {
                    pNewListEntry->entry.subresult = strtofp(
                        pCurrentString, sign,
                        pCalcCoreState->numberFormat.fixedPointDecimalPlace,
                        16);
                }

            } else if (inputBase == inputBase_BIN) {
                if (inputFormat == INPUT_FMT_INT) {
                    // Convert string to int.
                    // TBD: I think this should work with shorter strings as
                    // well
                    if (sign) {
                        pNewListEntry->entry.subresult =
                            strtoll(pCurrentString, &endPtr, 2);
                    } else {
                        pNewListEntry->entry.subresult =
                            strtoull(pCurrentString, &endPtr, 2);
                    }
                } else if (inputFormat == INPUT_FMT_FLOAT) {
                    // Floats have no specific format in binary, so just
                    // read out as int, but note the difference between 32 and
                    // 64 bits
                    if (pCalcCoreState->numberFormat.numBits == 32) {
                        pNewListEntry->entry.subresult =
                            strtoul(pCurrentString, &endPtr, 2);
                    } else if (pCalcCoreState->numberFormat.numBits == 64) {
                        pNewListEntry->entry.subresult =
                            strtoull(pCurrentString, &endPtr, 2);
                    }
                } else if (inputFormat == INPUT_FMT_FIXED) {
                    pNewListEntry->entry.subresult = strtofp(
                        pCurrentString, sign,
                        pCalcCoreState->numberFormat.fixedPointDecimalPlace, 2);
                }
            } else {
                free(pCurrentString);
                return calc_funStatus_INPUT_BASE_ERROR;
            }
            // Free the string
            free(pCurrentString);

        } else {
            // Not a number input, therefore move on to the next entry directly
            if (pCurrentListEntry != NULL) {
                pCurrentListEntry = pCurrentListEntry->pNext;
            } else {
                break;
            }
        }

        // Set the next and previous pointer of new entry
        pNewListEntry->pNext = NULL;
        pNewListEntry->pPrevious = pPreviousListEntry;
        if (pPreviousListEntry != NULL) {
            pPreviousListEntry->pNext = pNewListEntry;
        }
        pPreviousListEntry = pNewListEntry;
    }
    return calc_funStatus_SUCCESS;
}

/**
 * @brief Function to count arguments between pointers
 * @param pStart Pointer to start of list
 * @param pEnd Pointer to end of list.
 * @return The number of arguments. 0 if anything went wrong.
 *
 * This function counts the number of arguments, where an argument
 * is divided by comma ',' and if no comma present then there is only
 * one argument.
 */
uint8_t countArgs(inputListEntry_t *pStart, inputListEntry_t *pEnd) {

    if (pStart == NULL) {
        return 0;
    }
    // Check if the first entry is a number.
    // if (GET_INPUT_TYPE(pStart->entry.typeFlag) != INPUT_TYPE_NUMBER) {
    //    logger("First entry is not a number! %i\r\n",
    //    GET_INPUT_TYPE(pStart->entry.typeFlag)); return 0;
    //}
    uint8_t count = 1;
    while ((pStart != pEnd) && (pStart != NULL)) {
        if (pStart->entry.c == ',') {
            count++;
        }
        pStart = pStart->pNext;
    }
    return count;
}

/**
 * @brief Function to read out operator function arguments to array.
 * @param pArgs Pointer to argument destination. Needs to be pre-allocated.
 * @param numArgs Number of arguments that should be read out.
 * @param pStart Pointer to start of list
 * @return 0 if OK, otherwise -1
 */
int8_t readOutArgs(inputType_t *pArgs, int8_t numArgs, inputListEntry_t *pStart,
                   inputListEntry_t *pEnd) {
    // pArgs must have been allocated
    if (pStart == NULL) {
        return -1;
    }
    // Read out numArgs arguments from the list.
    uint8_t readArgs = 0;
    while (readArgs < numArgs) {
        if (pStart == pEnd) {
            return -2;
        }
        if (pStart == NULL) {
            return -1;
        }
        if (GET_SUBRESULT_TYPE(pStart->entry.typeFlag) == SUBRESULT_TYPE_INT) {
            // Argument found.
            logger(LOGGER_LEVEL_INFO, "Argument[%i] = %i\r\n", readArgs,
                   pStart->entry.subresult);
            memcpy(pArgs++, &(pStart->entry), sizeof(inputType_t));
            readArgs++;
        }
        pStart = pStart->pNext;
    }
    return 0;
}

/**
 * @brief Function to solve a single expression.
 * @param pCalcCoreState Pointer to core state
 * @param ppResult Pointer to pointer to result entry
 * @param pExprStart Pointer to start of expression
 * @param pExprEnd Pointer to end of expression
 * @return Status of function
 *
 * This function solves an expression in the format of:
 * [bracket/function/operator/none][expression][bracket/none]
 *
 * It solved it using the priority given by the operator,
 * and then finally the outer operator if any
 * @warning This function free's and rearranges the input list.
 * TODO: Save the location of an error.
 */

static int solveExpression(calcCoreState_t *pCalcCoreState,
                           inputListEntry_t **ppResult,
                           inputListEntry_t **ppExprStart,
                           inputListEntry_t *pExprEnd) {

    // Quick sanity check so that the start of the expression isn't NULL
    inputListEntry_t *pExprStart = *ppExprStart;
    if (pExprStart == NULL) {
        logger(LOGGER_LEVEL_ERROR, "ERROR! Input pointer is NULL\r\n");
        return calc_solveStatus_INPUT_LIST_NULL;
    }

    bool solveOuterOperator = false; // Boolean to indicate that outer operator
                                     // needs solving.
    inputListEntry_t *pStart =
        pExprStart; // Temporary variable to hold the start pointer
    inputListEntry_t *pEnd =
        pExprEnd; // Temporary variable to hold the end pointer
    // Check if the first and last entries are depth increasing:
    if (GET_DEPTH_FLAG(pExprStart->entry.typeFlag) == DEPTH_CHANGE_INCREASE) {
        // First entry in the list is depth increasing, therefore
        // the last must be depth decreasing.
        if (pExprEnd == NULL) {
            logger(LOGGER_LEVEL_ERROR, "Error: last entry is NULL\r\n");
            return calc_solveStatus_BRACKET_ERROR;
        }
        if (GET_DEPTH_FLAG(pExprEnd->entry.typeFlag) != DEPTH_CHANGE_DECREASE) {
            logger(LOGGER_LEVEL_ERROR,
                   "Error: last entry expected to be closing bracket!\r\n");
            return calc_solveStatus_BRACKET_ERROR;
        }
        if (pExprStart->pNext == pExprEnd) {
            logger(LOGGER_LEVEL_INFO,
                   "No arguments in operator or brackets. \r\n");
            return calc_solveStatus_INVALID_NUM_ARGS;
        }
        // If the start and end is an operator, we must solve that last,
        // but if it's just brackets then we can free those and move on.
        if (GET_INPUT_TYPE(pExprStart->entry.typeFlag) == INPUT_TYPE_OPERATOR) {

            logger(LOGGER_LEVEL_INFO, "There is an outer operator\r\n");
            solveOuterOperator = true;
            pStart = pExprStart->pNext;
            pEnd = pExprEnd->pPrevious;

        } else if (GET_INPUT_TYPE(pExprStart->entry.typeFlag) ==
                   INPUT_TYPE_EMPTY) {
            // This is a bracket, free the first and last entries and re-point.
            logger(LOGGER_LEVEL_INFO, "Remove brackets, free and re-point\r\n");
            pStart = pExprStart->pNext;
            pEnd = pExprEnd->pPrevious;
            pEnd->pNext = pExprEnd->pNext;
            pStart->pPrevious = pExprStart->pPrevious;
            if (pExprStart->pPrevious != NULL) {
                ((inputListEntry_t *)(pExprStart->pPrevious))->pNext = pStart;
            } else {
                // If the previous entry was NULL, then fix the starting pointer
                // too.
                *ppExprStart = pStart;
            }
            if (pExprEnd->pNext != NULL) {
                ((inputListEntry_t *)(pExprEnd->pNext))->pPrevious = pEnd;
            }
            overloaded_free(pExprStart);
            overloaded_free(pExprEnd);
            pExprEnd = pEnd;
            pCalcCoreState->allocCounter -= 2;

        } else {
            // This should not happen, throw an error
            logger(LOGGER_LEVEL_INFO,
                   "Depth increase was not operator or bracket!\r\n");
            return calc_solveStatus_INPUT_LIST_ERROR;
        }
    }
    // There are now three valid possibilies; a solvable expression in format
    // of NUM1 -> OP -> NUM2, or NUM1->','->NUM2->','->... or just a single
    // entry. Only one of these has an operator. We need to solve between
    // pTmpStart and pTmpEnd. Loop until there is a single entry left.
    bool noOperatorsLeft = false;
    inputListEntry_t *pResult = NULL;
    while (!noOperatorsLeft) {
        logger(LOGGER_LEVEL_INFO, "pTmpStart = 0x%08x, pTmpEnd = 0x%08x\r\n",
               pStart, pEnd);

        // Find the highest priorty operator
        inputListEntry_t *pHigestPrioOp = NULL;
        inputListEntry_t *pCurrentEntry = pStart;
        uint8_t highestOpPrio = 255;
        while (pCurrentEntry != pEnd) {
            if (GET_INPUT_TYPE(pCurrentEntry->entry.typeFlag) ==
                INPUT_TYPE_OPERATOR) {
                // This entry is an operator. Find the priority.
                uint8_t currentOpPrio =
                    ((operatorEntry_t *)(pCurrentEntry->pFunEntry))->solvPrio;
                if (currentOpPrio < highestOpPrio) {
                    // Found a new higher order priority.
                    pHigestPrioOp = pCurrentEntry;
                    highestOpPrio = currentOpPrio;
                }
            }
            pCurrentEntry = pCurrentEntry->pNext;
            if (pCurrentEntry == NULL) {
                // If the new list entry points to NULL then break the loop
                break;
            }
        }
        logger(LOGGER_LEVEL_INFO, "Operator evaluation complete.\r\n");
        // Check if an operator was found:
        if (pHigestPrioOp != NULL) {
            logger(LOGGER_LEVEL_INFO, "Found highest order operator: %s\r\n",
                   ((operatorEntry_t *)(pHigestPrioOp->pFunEntry))->opString);
            // The higest order operator is now solved in the following way:
            // pHigestOrderOp->pFun(pHigestOrderOp->pPrevious->value,
            // pHigestOrderOp->pNext->value). The result is then placed in the
            // highest order operator, which is converted to a number entry
            // instead of an operator entry. But first, do some sanity checking
            // on the previous and next entries:
            inputListEntry_t *pPrevEntry = pHigestPrioOp->pPrevious;
            inputListEntry_t *pNextEntry = pHigestPrioOp->pNext;
            if ((pNextEntry == NULL) || (pPrevEntry == NULL)) {
                logger(LOGGER_LEVEL_ERROR,
                       "ERROR: Pointer(s) before or after operator are NULL\n");
                return calc_solveStatus_OPERATOR_POINTER_ERROR;
            }
            if ((GET_INPUT_TYPE(pNextEntry->entry.typeFlag) !=
                 INPUT_TYPE_NUMBER) ||
                (GET_INPUT_TYPE(pPrevEntry->entry.typeFlag) !=
                 INPUT_TYPE_NUMBER)) {
                logger(
                    LOGGER_LEVEL_ERROR,
                    "ERROR: %c and %c surrounding %c are not numbers!\n",
                    pNextEntry->entry.c, pPrevEntry->entry.c,
                    ((operatorEntry_t *)(pHigestPrioOp->pFunEntry))->opString);
                return calc_solveStatus_OPERATOR_POINTER_ERROR;
            }
            if ((GET_SUBRESULT_TYPE(pNextEntry->entry.typeFlag) !=
                 SUBRESULT_TYPE_INT) ||
                (GET_SUBRESULT_TYPE(pPrevEntry->entry.typeFlag) !=
                 SUBRESULT_TYPE_INT)) {
                logger(LOGGER_LEVEL_ERROR,
                       "ERROR: %c and %c are not resolved integers!\n",
                       pNextEntry->entry.c, pPrevEntry->entry.c);
                return calc_solveStatus_OPERATOR_POINTER_ERROR;
            }
            // Now that the operator is surrounded by valid input, solve the
            // expression
            logger(LOGGER_LEVEL_INFO, "Solving %i",
                   pPrevEntry->entry.subresult);
            logger(LOGGER_LEVEL_INFO, " %s ",
                   ((operatorEntry_t *)(pHigestPrioOp->pFunEntry))->opString);
            logger(LOGGER_LEVEL_INFO, "%i\r\n", pNextEntry->entry.subresult);

            inputFormat_t inputFormat =
                pCalcCoreState->numberFormat.inputFormat;
            bool sign = pCalcCoreState->numberFormat.sign;
            // Get the function pointer casted to the correct format.
            int8_t (*pFun)(SUBRESULT_INT * pResult, numberFormat_t numberFormat,
                           int num_args, inputType_t *args) =
                (function_operator
                     *)(((operatorEntry_t *)(pHigestPrioOp->pFunEntry))->pFun);
            inputType_t pArgs[2];
            memcpy(&(pArgs[0]), &(pPrevEntry->entry), sizeof(inputType_t));
            memcpy(&(pArgs[1]), &(pNextEntry->entry), sizeof(inputType_t));

            // Solve the operation and save the results to the operator
            // subresults.
            int8_t calcStatus = (*pFun)(&(pHigestPrioOp->entry.subresult),
                                        pCalcCoreState->numberFormat, 2, pArgs);
            if (calcStatus < 0) {
                logger(LOGGER_LEVEL_ERROR, "ERROR: Calculation not solvable");
                return calc_solveStatus_CALC_NOT_SOLVABLE;
            }
            if (calcStatus > 0) {
                logger(LOGGER_LEVEL_INFO,
                       "Warning: calculation had some problems");
            }
            logger(LOGGER_LEVEL_INFO,
                   "Solved. Result was calculated to 0x%x\r\n",
                   pHigestPrioOp->entry.subresult);
            pHigestPrioOp->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_INT,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
            // Now repoint and free the input to each side of this operator.
            pHigestPrioOp->pNext = pNextEntry->pNext;
            pHigestPrioOp->pPrevious = pPrevEntry->pPrevious;
            if (pNextEntry->pNext != NULL) {
                ((inputListEntry_t *)(pNextEntry->pNext))->pPrevious =
                    pHigestPrioOp;
            }
            if (pPrevEntry->pPrevious != NULL) {
                ((inputListEntry_t *)(pPrevEntry->pPrevious))->pNext =
                    pHigestPrioOp;
            }
            logger(LOGGER_LEVEL_INFO, "pHigestOrderOp : 0x%08x\r\n",
                   pHigestPrioOp);
            logger(LOGGER_LEVEL_INFO, "pHigestOrderOp->pPrev : 0x%08x\r\n",
                   pHigestPrioOp->pPrevious);
            logger(LOGGER_LEVEL_INFO, "pHigestOrderOp->pNext : 0x%08x\r\n",
                   pHigestPrioOp->pNext);
            overloaded_free(pPrevEntry);
            overloaded_free(pNextEntry);
            pCalcCoreState->allocCounter -= 2;

            // Check if we just erased the starting point. If so the repoint
            // that as well.
            if (pPrevEntry == pStart) {
                logger(
                    LOGGER_LEVEL_INFO,
                    "Start of the expressions just free'd. Repoint that.\r\n");
                pStart = pHigestPrioOp;
            }
            if (pNextEntry == pEnd) {
                logger(LOGGER_LEVEL_INFO,
                       "End of the expressions just free'd. Repoint that.\r\n");
                pEnd = pHigestPrioOp;
            }
            // Check if we just erased the starting point of expressions as
            // well:
            if (pPrevEntry == *ppExprStart) {
                logger(LOGGER_LEVEL_INFO,
                       "Pointer to expression was also free'd. repoint.\r\n");
                *ppExprStart = pHigestPrioOp;
                logger(LOGGER_LEVEL_INFO, "New start: 0x%x.\r\n",
                       (uint32_t)pHigestPrioOp);
            }

            pResult = pHigestPrioOp;
        } else {
            // No operator was found, but that's OK, it could be a single
            // expression, or something going into a multi-input depth
            // increasing outer operator
            noOperatorsLeft = true;
            if (pStart->pNext == NULL && pStart->pPrevious == NULL) {
                if ((GET_SUBRESULT_TYPE(pStart->entry.typeFlag) ==
                     SUBRESULT_TYPE_INT)) {
                    pResult = pStart;
                }
            } else if (pStart == pEnd) {
                pResult = pStart;
            }
            logger(LOGGER_LEVEL_INFO, "Operator not found or none left. \r\n");
        }
    }
    logger(LOGGER_LEVEL_INFO, "Expression solved \r\n");
    if (solveOuterOperator) {
        // But wait, there's more!
        // Still need to solve the outer operator!
        uint8_t numArgsInBuffer = countArgs(pExprStart, pExprEnd);
        logger(LOGGER_LEVEL_INFO, "Number of args: %i\r\n", numArgsInBuffer);

        if (GET_INPUT_TYPE(pExprStart->entry.typeFlag) == INPUT_TYPE_OPERATOR) {
            // This is a depth increasing operator.
            // There can be a variable amount of arguments here,
            // separated by ',' if needed.
            // Check if function accepts variable input
            int8_t operatorNumArgs =
                ((operatorEntry_t *)(pExprStart->pFunEntry))->numArgs;
            if (operatorNumArgs == 0) {
                // Variable amount of arguments.
                // Reserved.
                logger(LOGGER_LEVEL_INFO,
                       "Operator arguments is 0. Does not make sense\r\n");
                return calc_solveStatus_INVALID_NUM_ARGS;
            } else if (operatorNumArgs > 0) {
                // Fixed amount of arguments.
                if (numArgsInBuffer != operatorNumArgs) {
                    logger(LOGGER_LEVEL_INFO,
                           "Operator accepts %i arguments, but %i arguments "
                           "was given.\r\n",
                           operatorNumArgs, numArgsInBuffer);
                    return calc_solveStatus_INVALID_NUM_ARGS;
                }
            }
            // Allocate a temporary buffer to hold all arguments and calculate
            inputType_t *pArgs = malloc(numArgsInBuffer * sizeof(inputType_t));
            if (pArgs == NULL) {
                logger(LOGGER_LEVEL_INFO,
                       "Arguments could not be allocated!\r\n");
                return calc_solveStatus_ALLOCATION_ERROR;
            }
            if (readOutArgs(pArgs, numArgsInBuffer, pExprStart, pExprEnd) !=
                0) {
                logger(LOGGER_LEVEL_ERROR, "Error: Incorrect arguments.\r\n");
                return calc_solveStatus_INVALID_ARGS;
            }
            inputFormat_t inputFormat =
                pCalcCoreState->numberFormat.inputFormat;
            bool sign = pCalcCoreState->numberFormat.sign;
            int8_t (*pFun)(SUBRESULT_INT * pResult, numberFormat_t numberFormat,
                           int num_args, inputType_t *pArgs) =
                (function_operator
                     *)(((operatorEntry_t *)(pExprStart->pFunEntry))->pFun);

            int8_t calcStatus =
                (*pFun)(&(pExprStart->entry.subresult),
                        pCalcCoreState->numberFormat, numArgsInBuffer, pArgs);
            logger(LOGGER_LEVEL_INFO, "Result of outer expression = %i\r\n",
                   pExprStart->entry.subresult);

            // Record the result and clean up the solved buffer.
            pResult = pExprStart;

            // Free the rest of the buffer, and repoint the buffers if needed.
            while (pExprStart->pNext != NULL && pExprStart->pNext != pExprEnd) {
                inputListEntry_t *pNextEntry = pExprStart->pNext;
                // Before pNextEntry can be freed, the surrounding pointers
                // must be repointed.
                pExprStart->pNext = pNextEntry->pNext;
                ((inputListEntry_t *)(pNextEntry->pNext))->pPrevious =
                    pExprStart;
                overloaded_free(pNextEntry);
                pCalcCoreState->allocCounter--;
            }
            pExprStart->pNext = pExprEnd->pNext;
            // The buffer now consists of pExprStart->pNext = pExprEnd/NULL.
            if (pExprEnd != NULL) {
                // The end entry must be free'd as well
                if (pExprEnd->pNext != NULL) {
                    ((inputListEntry_t *)(pExprEnd->pNext))->pPrevious =
                        pExprStart;
                }
                overloaded_free(pExprEnd);
                pCalcCoreState->allocCounter--;
            }
            // Construct a new typeflag for the result.
            pExprStart->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_INT,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
            pResult = pExprStart;
            logger(LOGGER_LEVEL_INFO,
                   "pResult = 0x%08x, pResult->pNext = 0x%08x", pResult,
                   pResult->pNext);
            free(pArgs);
        } else {
            // If there is more that one arguments, but no operator,
            // that is an error
            // The empty bracket case is already handled.
            return calc_solveStatus_ARGS_BUT_NO_OPERATOR;
        }
    }
    *ppResult = pResult;
    logger(LOGGER_LEVEL_INFO, "Returning. \r\n");
    return calc_solveStatus_SUCCESS;
}
calc_funStatus_t calc_solver(calcCoreState_t *pCalcCoreState) {
    pCalcCoreState->solved = false;

    // Local variables to keep track while the solver is
    // at work. Should be copied to core state when done.
    SUBRESULT_INT result = 0;
    bool solved = false;
    int depth = 0;
    bool overflow = false;

    // Copy the list and convert the numbers from
    // chars into actual ints (or floats if that's the case)

    inputListEntry_t *pSolverListStart = NULL;
    logger(LOGGER_LEVEL_INFO, "Copy and convert list.\r\n");
    copyAndConvertList(pCalcCoreState, &pSolverListStart);
    // Loop through and find the deepest point of the buffer
    inputListEntry_t *pStart = pSolverListStart;
    inputListEntry_t *pEnd = NULL;

    // Do a NULL check on the start:
    if (pStart == NULL) {
        // No list to solve for. Simply return
        logger(LOGGER_LEVEL_ERROR, "ERROR: No input list\r\n");
        return calc_funStatus_INPUT_LIST_NULL;
    }

    // Loop until the pointers are back at start and end
    calc_funStatus_t returnStatus = calc_funStatus_SUCCESS;
    while (!solved) {
        // Find the deepest calculation
        pStart = pSolverListStart;
        pEnd = NULL;
        if (findDeepestPoint(&pStart, &pEnd) < 0) {
            // There was an error in finding the bracket.
            logger(LOGGER_LEVEL_INFO,
                   "Could not find the deepest point between 0x%08x and "
                   "0x%08x\r\n",
                   pStart, pEnd);
            returnStatus = calc_funStatus_SOLVE_INCOMPLETE;
            break;
        }
        // The deepest calculation is now between pStart and pEnd.
        // This is in the form of:
        // [bracket/function/operator/none][expression][bracket/none]
        // The expression can consist of however many operators,
        // but no depth increasing ones. In the case of the last
        // expression, there might be no brackets or expressions.

        // The numbers in the expression is always integers,
        // since the input list has been converted.
        // It's now a case of continously shrinking that list,
        // solving each expression at a time
        //
        // Solving the expression will shrink the list,
        // and it's not certain that the start of the solver
        // list isn't freed. Therefore the expression solver
        // needs to return the list entry where it put the result
        inputListEntry_t *pResult = NULL;

        bool startOfExpressionSameAsStartOfList = false;
        if (pSolverListStart == pStart) {
            logger(LOGGER_LEVEL_INFO,
                   "Start of the expression same as start of list\r\n");
            startOfExpressionSameAsStartOfList = true;
        }
        if (solveExpression(pCalcCoreState, &pResult, &pStart, pEnd) < 0) {
            logger(LOGGER_LEVEL_ERROR, "ERROR: Could not solve expression\r\n");
            returnStatus = calc_funStatus_SOLVE_INCOMPLETE;
        }
        if (startOfExpressionSameAsStartOfList) {
            // The start of the list was free'd. Repoint.
            pSolverListStart = pStart;
        }

        if (pResult == NULL) {
            logger(LOGGER_LEVEL_INFO,
                   "No result written, but expression solver returned OK. \n");
            // Check that there is exactly one expression,
            // and if that is a numerical entry.
            // If yes, then save that to the result
            returnStatus = calc_funStatus_SOLVE_INCOMPLETE;
            break;
        }
        if ((pSolverListStart->pNext == NULL) &&
            (pSolverListStart->pPrevious == NULL)) {
            logger(LOGGER_LEVEL_INFO, "SOLVED! Result is %i\r\n",
                   pResult->entry.subresult);
            pCalcCoreState->result = pResult->entry.subresult;
            solved = true;
        } else {
            logger(LOGGER_LEVEL_INFO, "next \r\n");
        }
        logger(LOGGER_LEVEL_INFO, "Done with expression\r\n");
    }
    logger(LOGGER_LEVEL_INFO, "Done with solving.\r\n");

    // Free the rest if any. Should only occur if solution could not be found.
    while (pSolverListStart != NULL) {
        logger(LOGGER_LEVEL_INFO, "In free loop\r\n");
        inputListEntry_t *pNext = pSolverListStart->pNext;
        logger(LOGGER_LEVEL_INFO, "pNext = 0x%x\r\n", (uint32_t)pNext);
        if (pCalcCoreState->allocCounter == 0) {
            return -1;
        }
        if (pSolverListStart != NULL) {
            overloaded_free(pSolverListStart);
            pCalcCoreState->allocCounter--;
        } else {
            break;
        }
        pSolverListStart = pNext;
    }

    logger(LOGGER_LEVEL_INFO, "Returning. \r\n");
    return returnStatus;
}

/**
 * @brief Function to add the syntax issue position to a pointer
 * @param pSyntaxIssuePos Pointer to syntax issue variable
 * @param numCharsWritten Number of characters that the issue is at.
 */
void calc_recordSyntaxIssuePos(int16_t *pSyntaxIssuePos,
                               uint16_t numCharsWritten) {
    if (pSyntaxIssuePos != NULL) {
        if (*pSyntaxIssuePos == -1) {
            *pSyntaxIssuePos = numCharsWritten - 1;
        }
    }
}

calc_funStatus_t calc_printBuffer(calcCoreState_t *pCalcCoreState,
                                  char *pResString, uint16_t stringLen,
                                  int16_t *pSyntaxIssuePos) {

    // Check pointer to calculator core state
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;

    // Check pointer to input list
    if (pCurrentListEntry == NULL) {
        return calc_funStatus_INPUT_LIST_NULL;
    }

    // Check the string entry
    if (pResString == NULL) {
        return calc_funStatus_STRING_BUFFER_ERROR;
    }

    // Make a local variable of the string entry to iterate on.
    char *pString = pResString;

    // Variable to keep track of the number of chars written to string
    // Add one as as we need a null terminator at the end.
    uint16_t numCharsWritten = 1;

    // Save the previous input type of checking if 0x or 0b should be printed
    uint8_t previousInputType = INPUT_TYPE_EMPTY;
    // Loop through all buffers
    while (pCurrentListEntry != NULL) {
        // Depending on the input type, print different things.
        uint8_t currentInputType =
            GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag);

        if (currentInputType == INPUT_TYPE_NUMBER) {
            // If the previous input type wasn't a number,
            // then print the precursor. For hex it's 0x, for bin it's 0b
            if (previousInputType != currentInputType &&
                previousInputType != INPUT_TYPE_DECIMAL_POINT) {
                // Check if we are allowed to print a numerical
                // entry if the previous one was not a number.
                // The only operation not allowing a number after
                // another entry, is the closing bracket.
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                            ->entry.c == ')') {
                        // This is an "illegal" entry of a number. Mark it.
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  numCharsWritten);
                    }
                }
                if (pCurrentListEntry->inputBase == inputBase_HEX) {
                    // Print '0x' if there is room
                    if (numCharsWritten < (stringLen - 2)) {
                        numCharsWritten += sprintf(pString, "0x");
                        // Increase the pointer two steps.
                        pString += 2;
                    } else {
                        return calc_funStatus_STRING_BUFFER_ERROR;
                    }
                }
                if (pCurrentListEntry->inputBase == inputBase_BIN) {
                    // Print '0b' if there is room
                    if (numCharsWritten < (stringLen - 2)) {
                        numCharsWritten += sprintf(pString, "0b");
                        // Increase the pointer two steps.
                        pString += 2;
                    } else {
                        return calc_funStatus_STRING_BUFFER_ERROR;
                    }
                }
            }
            if (numCharsWritten < stringLen) {
                *pString++ = pCurrentListEntry->entry.c;
                numCharsWritten++;
            } else {
                return calc_funStatus_STRING_BUFFER_ERROR;
            }
        } else if (currentInputType == INPUT_TYPE_OPERATOR) {
            // Input is operator. Print the string related to that operator.
            const operatorEntry_t *pOperator =
                (operatorEntry_t *)pCurrentListEntry->pFunEntry;
            // Record the starting point of the number of chars written for
            // the syntax issue recording
            uint16_t charsWrittenBeforeOperator = numCharsWritten;
            // Write the operator.
            uint8_t tmpStrLen = strlen(pOperator->opString);
            if (numCharsWritten < stringLen - tmpStrLen) {
                numCharsWritten += sprintf(pString, "%s", pOperator->opString);
                pString += tmpStrLen;
            } else {
                return calc_funStatus_STRING_BUFFER_ERROR;
            }
            // If the operator increase depth, then print an opening bracket too
            if (GET_DEPTH_FLAG(pCurrentListEntry->entry.typeFlag) ==
                DEPTH_CHANGE_INCREASE) {
                // Check if previous entry was allowed for a depth increasing
                // operator.
                // This is only the case for closing brackets or numbers
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                                ->entry.c == ')' ||
                        previousInputType == INPUT_TYPE_NUMBER) {
                        // This is an "illegal" entry of a number. Mark it.
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  charsWrittenBeforeOperator);
                    }
                }
                if (numCharsWritten < stringLen) {
                    *pString++ = OPENING_BRACKET;
                    numCharsWritten++;
                } else {
                    return calc_funStatus_STRING_BUFFER_ERROR;
                }
            } else {
                // Non depth increasing operator. Previous entry must have been
                // either a closing bracket or a number
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                                ->entry.c != ')' &&
                        previousInputType != INPUT_TYPE_NUMBER) {
                        // This is an "illegal" entry of a number. Mark it.
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  charsWrittenBeforeOperator);
                    }
                }
            }

        } else if ((currentInputType == INPUT_TYPE_EMPTY) ||
                   (currentInputType == INPUT_TYPE_DECIMAL_POINT)) {
            // This is either bracket or punctuation.
            // Depending on what it is, the syntax can differ.
            if (pCurrentListEntry->entry.c == OPENING_BRACKET) {
                // If it's an opening bracket, the previous
                // entry must be either nothing, a comma, or an
                // operator.
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (previousInputType == INPUT_TYPE_NUMBER) {
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  numCharsWritten);
                    } else if (previousInputType == INPUT_TYPE_EMPTY) {
                        if (((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                                ->entry.c != '(') {
                            calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                      numCharsWritten);
                        }
                    }
                }
            } else if (pCurrentListEntry->entry.c == CLOSING_BRACKET) {
                // For a closing bracket, the previous entry must have
                // been a number, or another closing bracket.
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (previousInputType != INPUT_TYPE_NUMBER &&
                        ((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                                ->entry.c != CLOSING_BRACKET) {
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  numCharsWritten);
                    }
                } else {
                    calc_recordSyntaxIssuePos(pSyntaxIssuePos, numCharsWritten);
                }
            } else if (pCurrentListEntry->entry.c == '.') {
                // For a dot, the previous entry must always be a number only.
                if (pCurrentListEntry->pPrevious != NULL) {
                    if (previousInputType != INPUT_TYPE_NUMBER) {
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  numCharsWritten);
                    }
                } else {
                    calc_recordSyntaxIssuePos(pSyntaxIssuePos, numCharsWritten);
                }
            } else if (pCurrentListEntry->entry.c == ',') {
                // For a comma, the previous entry must have been a closing
                // bracket or a number, and the depth must be at least larger
                // than 0 due to a depth increasing function.
                if (pCurrentListEntry->pPrevious != NULL) {
                    // Go backwards in the list to check if the last opening
                    // bracket was due to a depth increasing function, or an
                    // opening bracket.
                    bool inDepthIncreasingFunction = false;
                    inputListEntry_t *pTmpListEntry =
                        pCurrentListEntry->pPrevious;
                    while (pTmpListEntry != NULL) {
                        uint8_t tmpInputType =
                            GET_INPUT_TYPE(pTmpListEntry->entry.typeFlag);
                        // Check if entry is a depth increasing operator.
                        // If we found one, then break the loop, and reflect in
                        // variable
                        if (tmpInputType == INPUT_TYPE_OPERATOR) {
                            if (GET_DEPTH_FLAG(pTmpListEntry->entry.typeFlag) ==
                                DEPTH_CHANGE_INCREASE) {
                                inDepthIncreasingFunction = true;
                                break;
                            }
                        }
                        // Also check if the entry was a closing bracket, in
                        // which case just break.
                        if (tmpInputType == INPUT_TYPE_EMPTY) {
                            if (pTmpListEntry->entry.c == '(') {
                                break;
                            }
                        }
                        pTmpListEntry = pTmpListEntry->pPrevious;
                    }
                    if (previousInputType != INPUT_TYPE_NUMBER ||
                        !inDepthIncreasingFunction) {
                        calc_recordSyntaxIssuePos(pSyntaxIssuePos,
                                                  numCharsWritten);
                    }
                } else {
                    calc_recordSyntaxIssuePos(pSyntaxIssuePos, numCharsWritten);
                }
            } else {
                logger(LOGGER_LEVEL_INFO,
                       "Unknown other char [%c] to be syntax checked\r\n",
                       pCurrentListEntry->entry.c);
            }
            // Print the char to the buffer.
            if (numCharsWritten < stringLen) {
                *pString++ = pCurrentListEntry->entry.c;
                numCharsWritten++;
            } else {
                return calc_funStatus_STRING_BUFFER_ERROR;
            }
        } else {
            // List was broken. This should not happen.
            return calc_funStatus_ENTRY_LIST_ERROR;
        }

        previousInputType = currentInputType;
        pCurrentListEntry = pCurrentListEntry->pNext;
    }
    return calc_funStatus_SUCCESS;
}

uint8_t calc_getCursorLocation(calcCoreState_t *pCalcCoreState) {
    // Check pointer to calculator core state
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;

    // Check pointer to input list
    if (pCurrentListEntry == NULL) {
        return calc_funStatus_INPUT_LIST_NULL;
    }

    // Get the end of the list
    while (pCurrentListEntry->pNext != NULL) {
        pCurrentListEntry = pCurrentListEntry->pNext;
    }

    // Start going backwards for the length of the cursor, and
    // save the number of chars in the entry.
    uint8_t numChars = 0;
    uint8_t cursorCounter = 0;
    while (pCurrentListEntry != NULL) {
        if (cursorCounter >= pCalcCoreState->cursorPosition) {
            return numChars;
        }
        // Get the input type
        uint8_t currentInputType =
            GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag);
        if (currentInputType == INPUT_TYPE_NUMBER) {
            numChars += 1;
            // This entry is always 1 char wide, unless the
            // entry base is not decimal, then the
            // first entry has 1+2 chars to account for.
            if ((pCurrentListEntry->inputBase == inputBase_HEX) ||
                (pCurrentListEntry->inputBase == inputBase_BIN)) {
                if (pCurrentListEntry->pPrevious != NULL) {
                    // Check if the previous entry has a different type than
                    // number
                    if (GET_INPUT_TYPE(
                            ((inputListEntry_t *)(pCurrentListEntry->pPrevious))
                                ->entry.typeFlag) != INPUT_TYPE_NUMBER) {
                        numChars += 2;
                    }
                } else {
                    // This is the first entry, hence there are always
                    // either a 0x or a 0b entry here.
                    numChars += 2;
                }
            }
        } else if (currentInputType == INPUT_TYPE_OPERATOR) {
            // Get the operator, and add the length of the string
            const operatorEntry_t *pOperator =
                (operatorEntry_t *)pCurrentListEntry->pFunEntry;
            numChars += strlen(pOperator->opString);
            // If the operator is depth increasing, this is always printed with
            // an opening bracket automatically, so include that as well
            if (pOperator->bIncDepth) {
                numChars += 1;
            }
        } else {
            // Other inputs always have width 1.
            numChars += 1;
        }
        pCurrentListEntry = pCurrentListEntry->pPrevious;
        if (pCurrentListEntry == NULL) {
            // Cap the cursor position here
            // pCalcCoreState->cursorPosition = cursorCounter;
        }
        cursorCounter++;
    }
    return numChars;
}

/**
 * @brief Convert an integer to a binary string
 *
 * @param pBuf Pointer to a string buffer of size of #i number of bits
 * @param number The integer to be converted
 * @return Nothing
 */
static void intToBin(char *pBuf, SUBRESULT_INT number) {
    const uint8_t numBits = sizeof(SUBRESULT_INT) * 8;
    // Since we want the most significant bit at the start of
    // the buffer, e.g. number 10 = 0b1010 should be printed
    // with the 1 representing the 8th place/bit 4 at the top
    // of the buffer, start by decrementing a mask from the MSB
    // until the first 1 is detected.

    SUBRESULT_INT i = numBits - 1;
    while (((number >> i) == 0) && (i != 0)) {
        i--;
    }

    uint8_t strCount = 0;
    for (i; i >= 0; i--) {
        SUBRESULT_INT tmpNumber = (number >> i) & 0x01;
        pBuf[strCount++] = ((number >> i) & 0x01) + '0';
    }
}

void calc_updateBase(calcCoreState_t *pCalcCoreState) {
    // Null check the calc state
    if (pCalcCoreState == NULL) {
        return;
    }

    if (pCalcCoreState->pListEntrypoint == NULL) {
        // No entries in the list, just return.
        return;
    }

    // Get the entry for where the cursor is
    inputListEntry_t *pCurrentEntry = NULL;
    inputListEntry_t *pEntryAtCursor = NULL;
    getInputListEntry(pCalcCoreState, &pEntryAtCursor);

    // If this returned as NULL, we're either at the start of the
    // list. Hence we check the first entry.
    if (pEntryAtCursor == NULL) {
        // This is the first entry to the list.
        // Check if the entry is numeric
        if (GET_INPUT_TYPE(pCalcCoreState->pListEntrypoint->entry.typeFlag) !=
            INPUT_TYPE_NUMBER) {
            // Nothing to do, the first entry is not a number, so just exit
            logger(LOGGER_LEVEL_INFO,
                   "CONVERT:First entry is not a number\r\n");
            return;
        }
        // We only need to look "forward" to change the current entry
        // Set the current entry to the first entry of the list, to iterate
        // forward on
        pCurrentEntry = pCalcCoreState->pListEntrypoint;

    } else {
        if (GET_INPUT_TYPE(pEntryAtCursor->entry.typeFlag) !=
            INPUT_TYPE_NUMBER) {
            // TBD! Do we also need to check the previous here? If we're
            // just in between an operator and a number, e.g. 123|+,
            // will that return as an operator or a number?
            // Nothing to do, this entry is not a number, so just exit
            logger(LOGGER_LEVEL_INFO,
                   "CONVERT:Char at location not a number\r\n");
            return;
        }
        // This is not the first entry. Look "backwards" to find the start of
        // the current input.
        pCurrentEntry = pEntryAtCursor;
        while (pCurrentEntry->pPrevious != NULL) {
            // Check if the previous entry was a number.
            if (GET_INPUT_TYPE(((inputListEntry_t *)(pCurrentEntry->pPrevious))
                                   ->entry.typeFlag) == INPUT_TYPE_NUMBER) {
                // The previous entry was a number, set the current entry
                // to previous entry to go backwards
                pCurrentEntry = pCurrentEntry->pPrevious;
            } else {
                // If the previous entry wasn't a number, then break here.
                break;
            }
        }
    }
    // Given that we are currently pointing at the start of a number,
    // that number needs to be converted to a number, which in turn needs to be
    // converted back to the new base.

    // Allocate a temporary buffer to store the converted string to int.
    // Size is the maximum size of binary input, i.e. 64 bits
    char pTempCharBuffer[65] = {'\0'};

    inputBase_t newEntryInputBase = pCalcCoreState->numberFormat.inputBase;
    inputListEntry_t *pTempInputEntry = pCurrentEntry;
    uint8_t inputFormat = GET_FMT_TYPE(pTempInputEntry->entry.typeFlag);
    bool sign = pCalcCoreState->numberFormat.sign;
    if (inputFormat == INPUT_FMT_INT) {
        uint8_t charCount = 0;
        while (GET_INPUT_TYPE(pTempInputEntry->entry.typeFlag) ==
               INPUT_TYPE_NUMBER) {
            pTempCharBuffer[charCount++] = pTempInputEntry->entry.c;
            // Move on to the next entry
            pTempInputEntry = pTempInputEntry->pNext;
            if (pTempInputEntry == NULL) {
                break;
            }
        }
        pTempCharBuffer[charCount] = '\0';
        // Convert string to integer:
        SUBRESULT_INT stringInInt = 0;
        if (pCalcCoreState->numberFormat.sign) {
            stringInInt = strtoll(pTempCharBuffer, NULL,
                                  baseToRadix[pCurrentEntry->inputBase]);
        } else {
            stringInInt = strtoull(pTempCharBuffer, NULL,
                                   baseToRadix[pCurrentEntry->inputBase]);
        }
        memset(pTempCharBuffer, 0, 65);
        // All the entries have now been accounted for.
        // Therefore, make new entries with the new base,
        // repoint, and free the old entries.
        if (newEntryInputBase == inputBase_HEX) {
            // TBD: Do these support 64 bit?
            sprintf(pTempCharBuffer, "%llx", stringInInt);
        } else if (newEntryInputBase == inputBase_BIN) {
            intToBin(pTempCharBuffer, stringInInt);
        } else if (newEntryInputBase == inputBase_DEC) {
            sprintf(pTempCharBuffer, "%lli", stringInInt);
        }
    } else if (inputFormat == INPUT_FMT_FLOAT) {
        // TODO
    } else if (inputFormat == INPUT_FMT_FIXED) {
        // TODO
    } else {
        // THIS SHOULD NOT HAPPEN
        logger(LOGGER_LEVEL_ERROR, "ERROR! Unknown input format! %i\r\n",
               inputFormat);
        while (1)
            ;
    }
    inputListEntry_t *pNextNonCharEntry = pTempInputEntry;
    // NOTE: IMPORTANT! Need to reallocate the entries here, given that the new
    // base will have different amount of characters associated with the entry.

    // Loop through the string until a NULL char is reached
    uint8_t charCounter = 0;
    inputListEntry_t *pPreviousEntry = pCurrentEntry->pPrevious;
    inputListEntry_t *pStartOfNewList = NULL;
    inputListEntry_t *pNewListEntry = NULL;
    while (pTempCharBuffer[charCounter] != '\0') {
        pNewListEntry = overloaded_malloc(sizeof(inputListEntry_t));
        pCalcCoreState->allocCounter++;
        if (pStartOfNewList == NULL) {
            // Save the first new list entry that we allocate
            pStartOfNewList = pNewListEntry;
        }

        // Initialize the fields of the new entry, as for a regular number.
        pNewListEntry->pPrevious = pPreviousEntry;

        pNewListEntry->entry.c = pTempCharBuffer[charCounter];
        pNewListEntry->entry.subresult = 0;
        pNewListEntry->entry.typeFlag =
            CONSTRUCT_TYPEFLAG(sign, inputFormat, SUBRESULT_TYPE_CHAR,
                               DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
        pNewListEntry->inputBase = pCalcCoreState->numberFormat.inputBase;
        pNewListEntry->pFunEntry = NULL;

        // Set the previous entries next entry to this one
        if (pPreviousEntry != NULL) {
            pPreviousEntry->pNext = pNewListEntry;
        }

        // Set the previous entry to this entry now
        pPreviousEntry = pNewListEntry;
        charCounter++;
    }
    if (pNewListEntry == NULL) {
        logger(LOGGER_LEVEL_ERROR, "ERROR: CONVERT New list entry is NULL\r\n");
        return;
    }
    // If the old entries previous entry was NULL, that means it was the start
    // of the list.
    if (pCurrentEntry->pPrevious == NULL) {
        // Override the entry of the list to the first newly allocated one.
        pCalcCoreState->pListEntrypoint = pStartOfNewList;
    }
    // Set the next non-character entries previous entry to
    // point to the end of the new list that we just made,
    // along with setting the last new entires next entry
    // to point to the next non-character entry.
    pNewListEntry->pNext = pNextNonCharEntry;
    if (pNextNonCharEntry != NULL) {
        pNextNonCharEntry->pPrevious = pNewListEntry;
    }

    // Free the entries from pCurrentEntry to the last char entry.
    while (GET_INPUT_TYPE(pCurrentEntry->entry.typeFlag) == INPUT_TYPE_NUMBER) {
        // Make a temporary copy to free
        inputListEntry_t *pTmpListEntry = pCurrentEntry;
        // Move pointer to the next entry
        pCurrentEntry = pCurrentEntry->pNext;
        overloaded_free(pTmpListEntry);
        pCalcCoreState->allocCounter--;
        // If the new current entry is NULL, then break
        if (pCurrentEntry == NULL) {
            break;
        }
    }
}

calc_funStatus_t calc_updateOutputFormat(calcCoreState_t *pCalcCoreState,
                                         uint8_t outputFormat) {
    // Validate inputs:
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    if (outputFormat >= INPUT_FMT_RESERVED) {
        return calc_funStatus_UNKNOWN_PARAMETER;
    }

    // There not that much to updating the output format.
    // There really are no restrictions, so just update it.
    pCalcCoreState->numberFormat.outputFormat = outputFormat;

    return calc_funStatus_SUCCESS;
}

calc_funStatus_t calc_updateInputFormat(calcCoreState_t *pCalcCoreState,
                                        uint8_t inputFormat) {
    // Validate inputs:
    if (pCalcCoreState == NULL) {
        return calc_funStatus_CALC_CORE_STATE_NULL;
    }
    if (inputFormat >= INPUT_FMT_RESERVED) {
        return calc_funStatus_UNKNOWN_PARAMETER;
    }

    // Get the current input list object pointed at by the cursor:
    inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;
    if (pCurrentListEntry != NULL) {
        inputModStatus_t listState =
            getInputListEntry(pCalcCoreState, &pCurrentListEntry);
        // Check if the current list entry is numerical
        if (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
            INPUT_TYPE_NUMBER) {
            // Conversion at numerical input is not allowed.
            return calc_funStatus_FORMAT_ERROR;
        }
    }
    // If we get here, then we can safely update the input format
    pCalcCoreState->numberFormat.inputFormat = inputFormat;

    return calc_funStatus_SUCCESS;
}
