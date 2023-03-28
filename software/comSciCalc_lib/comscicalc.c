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
 * TODO list:
 * 0. Finish the basic implementaion of the solver. This requires extension
 * later on. (DONE)
 * 1. Enable entry of unsigned, signed, floating point and fixed point.
 * 2. Extend calulation funciton to handle varialble arguments (DONE).
 * 3. Extend input conversion to handle signed, unsigned, float and fixed point.
 * 4. Add support for comma sign in depth increasing functions. (DONE)
 *
 * GENERAL FEATURES
 * Add C formatter to pre-commit.
 * Add doxygen and clean up the source code because it looks like shit now.
 *
 */

/*
Overview:
List entries are a doubly linked list where each element consists of either:
1. A string (list)
2. an operator
3. a custom function
4. empty.

- A list entry can have a string and an operator, if that operator does not
increase depth.
- A list that has an that operator increase depth cannot have a string.
- A custom function acts like an operator, but always increase depth.
- A bracket entry is an entry which increases depth, but does not have a custom
function, an operator or a string.
- An entry can only be empty with none of those, only if there is no input.

*/

/* ----------------- HEADERS ----------------- */
// comsci header file
#include "comscicalc.h"

// Standard library
#include <stdio.h>
#include <string.h>

/* ------------- GLOBAL VARIABLES ------------ */

/* ---- CALCULATOR CORE HELPER FUNCTIONS ----- */

void logger(char *msg, ...) {
#if VERBOSE
    va_list argp;
    va_start(argp, msg);
    vprintf(msg, argp);
    va_end(argp);
#endif
}

/**
 * @brief Malloc wrapper to help debug memory leaks
 * @param size Size of malloc
 * @return Pointer that malloc allocates
 */
inputListEntry_t *overloaded_malloc(size_t size) {
    void *ptr = malloc(size);
    logger("[allocated] : 0x%08x\r\n", ptr);
    return ptr;
}

/**
 * @brief Free wrapper to help debug memory leaks
 * @param ptr Pointer to memory being free'd
 */
void overloaded_free(inputListEntry_t *ptr) {
    logger("[free] : 0x%08x\r\n", ptr);
    free(ptr);
    ptr = NULL;
}

/**
 * @brief Function to check if char is numerical
 * @param base Hex, dec or bin base.
 * @param c Character to check.
 * @return True if char is numerical within that base. Otherwise false.
 *
 */
static bool charIsNumerical(inputBase_t base, char c) {
    switch (base) {
    case inputBase_DEC:
        if (('0' <= c) && (c <= '9')) {
            return true;
        }
        break;
    case inputBase_HEX:
        if ((('0' <= c) && (c <= '9')) || (('a' <= c) && (c <= 'f'))) {
            return true;
        }
        break;
    case inputBase_BIN:
        if (('0' <= c) && (c <= '1')) {
            return true;
        }
        break;
    default:
        // Return false if input base is set to None.
        break;
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

    // Initialize the cursor to 0
    pCalcCoreState->cursorPosition = 0;

    // Set the input base to NONE
    pCalcCoreState->inputBase = inputBase_NONE;

    // Set the first pointer to NULL
    pCalcCoreState->pListEntrypoint = NULL;

    // Set the allocation counter to 0
    pCalcCoreState->allocCounter = 0;

    // Set the result to 0 and solved to false
    pCalcCoreState->result = 0;
    pCalcCoreState->solved = false;
    pCalcCoreState->inputFormat = INPUT_FMT_UINT;

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
    inputFormat_t inputFormat = pCalcCoreState->inputFormat;
    // Set the input subresult to 0
    pNewListEntry->entry.subresult = 0;
    if (charIsNumerical(pCalcCoreState->inputBase, inputChar)) {
        pNewListEntry->entry.typeFlag =
            CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                               DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
    } else if (charIsOperator(inputChar)) {
        // Get the operator
        const operatorEntry_t *pOp = getOperator(inputChar);
        if (pOp->bIncDepth) {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_INCREASE, INPUT_TYPE_OPERATOR);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_OPERATOR);
        }
        pNewListEntry->pFunEntry = (void *)pOp;

    } else if (charIsBracket(inputChar)) {
        if (inputChar == OPENING_BRACKET) {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_INCREASE, INPUT_TYPE_EMPTY);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_DECREASE, INPUT_TYPE_EMPTY);
        }
    } else if (charIsOther(inputChar)) {
        if (inputChar == ',') {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_EMPTY);
        } else {
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_CHAR,
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
    pNewListEntry->inputBase = pCalcCoreState->inputBase;

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

/**
 * @brief Function that converts a char to corresponding int (dec or bin)
 * @param c Char to be converted
 * @return 0 if not possible, but input function already checks for this.
 *   Otherwise returns the int in decimal base.
 */
int charToInt(char c) {
    if ((c >= '0') && (c <= '9')) {
        return (int)(c - '0');
    }
    if ((c >= 'a') && (c <= 'f')) {
        return (int)(c - 'a' + 10);
    }
    return 0;
}

/**
 * @brief Converts an input list containing chars, and converts all chars to
 * ints
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @param ppSolverListStart Pointer to pointer to start of list.
 * @return Status of the conversion.
 *
 * The conversion from char to int will reduce the length of the list, where
 * e.g. '1'->'2'->'3' will be converted to 123 (from 3 entries to 1).
 * @warning Floating and fixed point conversion not yet done.
 */
calc_funStatus_t copyAndConvertList(calcCoreState_t *pCalcCoreState,
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
            // until the entry is either NULL or not numerical
            inputFormat_t inputFormat = pCalcCoreState->inputFormat;
            pNewListEntry->entry.typeFlag =
                CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_INT,
                                   DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
            pNewListEntry->entry.subresult = 0;
            while (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
                   INPUT_TYPE_NUMBER) {
                // Aggregate the input based on the input base
                if (pCurrentListEntry->inputBase == inputBase_DEC) {
                    if ((inputFormat == INPUT_FMT_UINT)) {
                        SUBRESULT_UINT *pRes =
                            (SUBRESULT_UINT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 10;
                    } else if ((inputFormat == INPUT_FMT_SINT)) {
                        SUBRESULT_INT *pRes =
                            (SUBRESULT_INT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 10;
                    } else if ((inputFormat == INPUT_FMT_FLOAT)) {
                        // TODO
                    } else if ((inputFormat == INPUT_FMT_FIXED)) {
                        // TODO
                    }
                }
                if (pCurrentListEntry->inputBase == inputBase_HEX) {
                    if ((inputFormat == INPUT_FMT_UINT)) {
                        SUBRESULT_UINT *pRes =
                            (SUBRESULT_UINT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 16;
                    } else if ((inputFormat == INPUT_FMT_SINT)) {
                        SUBRESULT_INT *pRes =
                            (SUBRESULT_INT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 16;
                    } else if ((inputFormat == INPUT_FMT_FLOAT)) {
                        // TODO
                    } else if ((inputFormat == INPUT_FMT_FIXED)) {
                        // TODO
                    }
                }
                if (pCurrentListEntry->inputBase == inputBase_BIN) {
                    if ((inputFormat == INPUT_FMT_UINT)) {
                        SUBRESULT_UINT *pRes =
                            (SUBRESULT_UINT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 2;
                    } else if ((inputFormat == INPUT_FMT_SINT)) {
                        SUBRESULT_INT *pRes =
                            (SUBRESULT_INT *)&(pNewListEntry->entry.subresult);
                        *pRes = (*pRes) * 2;
                    } else if ((inputFormat == INPUT_FMT_FLOAT)) {
                        // TODO
                    } else if ((inputFormat == INPUT_FMT_FIXED)) {
                        // TODO
                    }
                }
                pNewListEntry->entry.subresult +=
                    charToInt(pCurrentListEntry->entry.c);
                // logger("Input: %c, output: %i\r\n",
                pCurrentListEntry = pCurrentListEntry->pNext;

                if (pCurrentListEntry == NULL) {
                    break;
                }
            }
        } else {
            // Not a number input, therefore move on to the next entry directly
            pCurrentListEntry = pCurrentListEntry->pNext;
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
int8_t readOutArgs(SUBRESULT_UINT *pArgs, int8_t numArgs,
                   inputListEntry_t *pStart) {
    // pArgs must have been allocated
    if (pStart == NULL) {
        return -1;
    }
    // Read out numArgs arguments from the list.
    uint8_t readArgs = 0;
    while (readArgs < numArgs) {
        if (pStart == NULL) {
            return -1;
        }
        if (GET_SUBRESULT_TYPE(pStart->entry.typeFlag) == SUBRESULT_TYPE_INT) {
            // Argument found.
            logger("Argument[%i] = %i\r\n", readArgs, pStart->entry.subresult);
            *pArgs++ = pStart->entry.subresult;
            readArgs++;
        }
        pStart = pStart->pNext;
    }
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
 */
int solveExpression(calcCoreState_t *pCalcCoreState,
                    inputListEntry_t **ppResult, inputListEntry_t *pExprStart,
                    inputListEntry_t *pExprEnd) {

    // Sanity check the input
    if (pExprStart == NULL) {
        logger("ERROR! Input pointer is NULL\r\n");
        return calc_solveStatus_INPUT_LIST_NULL;
    }

    inputListEntry_t *pStart = pExprStart;
    inputListEntry_t *pEnd = pExprEnd;
    bool outerExpression = false;
    logger("Solve expression\r\n");

    // Check if there is an outer expression
    if (GET_INPUT_TYPE(pStart->entry.typeFlag) != INPUT_TYPE_NUMBER) {
        logger("Expression starts with depth increase\r\n");
        // If the first entry isn't a number, it's either a bracket,
        // a depth increasing operator or a custom function.
        // Double check that the last entry is the same type.
        if (GET_INPUT_TYPE(pEnd->entry.typeFlag) == INPUT_TYPE_NUMBER) {
            logger("ERROR! Unmatched brackets\r\n");
            return calc_solveStatus_BRACKET_ERROR;
        }
        if (pStart->pNext == pEnd) {
            // Empty inside brackets, return error.
            logger("ERROR! Empty expression within brackets\r\n");
            return calc_solveStatus_BRACKET_ERROR;
        }
        // Move the start and end so that to remove the
        // operator/function/bracket, and check if the remaining list
        // is emtpy
        pStart = pExprStart->pNext;
        pEnd = pExprEnd->pPrevious;
        if (pStart == NULL) {
            logger("ERROR. \r\n");
            return calc_solveStatus_BRACKET_ERROR;
        }
        outerExpression = true;
    }
    // While the start of the buffers next entry isn't pointing at the end
    // continue solving.
    // Note: If only numbers entered, this will be skipped, as
    // pStart->pNext = pEnd.
    bool operatorFound = true;
    inputListEntry_t *pHigestOp = NULL;
    while (pStart->pNext != pEnd) {
        // Solve between pStart and pEnd.
        // Start by finding the highest priority operator
        // Priority is ascending, with 0 being the highest priority.
        inputListEntry_t *pCurrentListEntry = pStart;
        pHigestOp = NULL;
        uint8_t highestPriority = 255;
        while ((pCurrentListEntry != pEnd) && (pCurrentListEntry != NULL)) {
            if (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) ==
                INPUT_TYPE_OPERATOR) {
                // Get the operator solving priority
                if (pCurrentListEntry->pFunEntry == NULL) {
                    // Thas been as error, the function pointer for an operator
                    // should always exist!
                    logger("ERROR! Function pointer is NULL\r\n");
                    return calc_solveStatus_INPUT_LIST_ERROR;
                }
                uint8_t currentPriority =
                    ((operatorEntry_t *)(pCurrentListEntry->pFunEntry))
                        ->solvPrio;
                if (currentPriority < highestPriority) {
                    pHigestOp = pCurrentListEntry;
                    highestPriority = currentPriority;
                }
            }
            // logger("Current list entry: 0x%08x\r\n",
            // (uint32_t)pCurrentListEntry);
            pCurrentListEntry = pCurrentListEntry->pNext;
        }

        // Sanity check the result of the search.
        if (pHigestOp == NULL) {
            logger("Highest order operator not found!\n");
            operatorFound = false;
            break;
        }
        logger("Highest operator: %s\r\n",
               ((operatorEntry_t *)(pHigestOp->pFunEntry))->opString);
        // The operation that we should now perform is on
        // the current operator and the entries before and after.
        // Ensure that these exist, and are indeed numbers.
        inputListEntry_t *pPrevEntry = pHigestOp->pPrevious;
        inputListEntry_t *pNextEntry = pHigestOp->pNext;
        if ((pNextEntry == NULL) || (pPrevEntry == NULL)) {
            logger("ERROR: Pointer(s) before and after operators are NULL\n");
            return calc_solveStatus_OPERATOR_POINTER_ERROR;
        }
        if ((GET_INPUT_TYPE(pNextEntry->entry.typeFlag) != INPUT_TYPE_NUMBER) ||
            (GET_INPUT_TYPE(pPrevEntry->entry.typeFlag) != INPUT_TYPE_NUMBER)) {
            logger("ERROR: %c and %c surrounding %c are not numbers!\n",
                   pNextEntry->entry.c, pPrevEntry->entry.c,
                   pHigestOp->entry.c);
            return calc_solveStatus_OPERATOR_POINTER_ERROR;
        }
        if ((GET_SUBRESULT_TYPE(pNextEntry->entry.typeFlag) !=
             SUBRESULT_TYPE_INT) ||
            (GET_SUBRESULT_TYPE(pPrevEntry->entry.typeFlag) !=
             SUBRESULT_TYPE_INT)) {
            logger("ERROR: %c and %c are not resolved integers!\n",
                   pNextEntry->entry.c, pPrevEntry->entry.c);
            return calc_solveStatus_OPERATOR_POINTER_ERROR;
        }

        // Finally, we are ready to solve this subresult!
        // This will write the subresult to the operators
        // subresult field, set the subresult type to int and the
        // input type to number.
        logger("Solving %i %s %i\n", pPrevEntry->entry.subresult,
               ((operatorEntry_t *)(pHigestOp->pFunEntry))->opString,
               pNextEntry->entry.subresult);

        inputFormat_t inputFormat = pCalcCoreState->inputFormat;
        int8_t (*pFun)(SUBRESULT_UINT * pResult, inputFormat_t inputFormat,
                       int num_args, SUBRESULT_UINT *args) =
            (function_operator *)(((operatorEntry_t *)(pHigestOp->pFunEntry))
                                      ->pFun);
        SUBRESULT_UINT pSubresult =
            (SUBRESULT_UINT)(&(pHigestOp->entry.subresult));
        SUBRESULT_UINT pArgs[2];
        pArgs[0] = (SUBRESULT_UINT)(pPrevEntry->entry.subresult);
        pArgs[1] = (SUBRESULT_UINT)(pNextEntry->entry.subresult);
        int8_t calcStatus =
            (*pFun)(&(pHigestOp->entry.subresult), inputFormat, 2, pArgs);
        if (calcStatus < 0) {
            logger("ERROR: Calculation not solvable");
            return calc_solveStatus_CALC_NOT_SOLVABLE;
        }
        if (calcStatus > 0) {
            logger("Warning: calculation had some problems");
        }

        logger("Subresult = %i \r\n", pHigestOp->entry.subresult);
        pHigestOp->entry.typeFlag =
            CONSTRUCT_TYPEFLAG(inputFormat, SUBRESULT_TYPE_INT,
                               DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);

        // The subresult is now stored in the operators entry, so we have to
        // remove the two number on either side, and replace it with
        // the operator only, as it's been solved!
        pHigestOp->pPrevious = pPrevEntry->pPrevious;
        pHigestOp->pNext = pNextEntry->pNext;
        if (pPrevEntry->pPrevious != NULL) {
            ((inputListEntry_t *)(pPrevEntry->pPrevious))->pNext = pHigestOp;
        }
        if (pNextEntry->pNext != NULL) {
            ((inputListEntry_t *)(pNextEntry->pNext))->pPrevious = pHigestOp;
        }
        // Free the previous and next entries
        overloaded_free(pPrevEntry);
        overloaded_free(pNextEntry);
        // Start next round of looking on this entry.
        pStart = pHigestOp;
        pCalcCoreState->allocCounter -= 2;
        // It should just magically continue from here
    }
    // if applicable, solve the outer most operator as well.
    if (outerExpression) {
        logger("Need to solve outer function as well!\n");
        uint8_t numArgsInBuffer = countArgs(pExprStart, pExprEnd);
        logger("Number of args: %i\r\n", numArgsInBuffer);

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
                logger("Operator arguments is 0. Does not make sense\r\n");
                return calc_solveStatus_INVALID_NUM_ARGS;
            } else if (operatorNumArgs > 0) {
                // Fixed amount of arguments.
                if (numArgsInBuffer != operatorNumArgs) {
                    logger("Operator accepts %i arguments, but %i arguments "
                           "was given.\r\n",
                           operatorNumArgs, numArgsInBuffer);
                    return calc_solveStatus_INVALID_NUM_ARGS;
                }
            }
            // Allocate a temporary buffer to hold all arguments and calculate
            SUBRESULT_UINT *pArgs =
                malloc(numArgsInBuffer * sizeof(SUBRESULT_UINT));
            if (pArgs == NULL) {
                logger("Arguments could not be allocated!\r\n");
                return calc_solveStatus_ALLOCATION_ERROR;
            }
            readOutArgs(pArgs, numArgsInBuffer, pExprStart);
            inputFormat_t inputFormat = pCalcCoreState->inputFormat;
            int8_t (*pFun)(SUBRESULT_UINT * pResult, inputFormat_t inputFormat,
                           int num_args, SUBRESULT_UINT *args) =
                (function_operator
                     *)(((operatorEntry_t *)(pExprStart->pFunEntry))->pFun);
            SUBRESULT_UINT pSubresult =
                (SUBRESULT_UINT)(&(pExprStart->entry.subresult));
            int8_t calcStatus = (*pFun)(&(pExprStart->entry.subresult),
                                        inputFormat, numArgsInBuffer, pArgs);
            free(pArgs);
        } else {
            // If there is more that one arguments, but no operator,
            // that is an error
            if (numArgsInBuffer > 1) {
                return calc_solveStatus_ARGS_BUT_NO_OPERATOR;
            }
            // No operator, simply ignore it and
            // pass on the result.
            pExprStart->entry.subresult = pStart->entry.subresult;
            pExprStart->entry.typeFlag = pStart->entry.typeFlag;
        }

        pStart = pExprStart;
        pEnd = pExprEnd;

        // Free everything after pStart until the end.
        while (pStart->pNext != pExprEnd) {
            inputListEntry_t *pTmp = pStart->pNext;
            pStart->pNext = pTmp->pNext;
            if (pTmp != NULL) {
                overloaded_free(pTmp);
                pCalcCoreState->allocCounter--;
            } else {
                break;
            }
        }
        // Free the end and repoint.
        pStart->pNext = pExprEnd->pNext;
        if (pExprEnd->pNext != NULL) {
            ((inputListEntry_t *)(pExprEnd->pNext))->pPrevious = pStart;
        }
        overloaded_free(pExprEnd);
        pCalcCoreState->allocCounter--;
    }
    *ppResult = pStart;
    // logger("pStart->entry.subres = %i\r\n", pStart->entry.subresult);
    return calc_solveStatus_SUCCESS;
}

calc_funStatus_t calc_solver(calcCoreState_t *pCalcCoreState) {
    pCalcCoreState->solved = false;

    // Local variables to keep track while the solver is
    // at work. Should be copied to core state when done.
    SUBRESULT_UINT result = 0;
    bool solved = false;
    int depth = 0;
    bool overflow = false;

    // Copy the list and convert the numbers from
    // chars into actual ints (or floats if that's the case)

    inputListEntry_t *pSolverListStart = NULL;
    copyAndConvertList(pCalcCoreState, &pSolverListStart);

    // Loop through and find the deepest point of the buffer
    inputListEntry_t *pStart = pSolverListStart;
    inputListEntry_t *pEnd = NULL;

    // Do a NULL check on the start:
    if (pStart == NULL) {
        // No list to solve for. Simply return
        logger("ERROR: No input list\r\n");
        return calc_funStatus_INPUT_LIST_NULL;
    }

    // Loop until the pointers are back at start and end
    calc_funStatus_t returnStatus = calc_funStatus_SUCCESS;
    while (!solved) {
        // Find the deepest calculation
        if (findDeepestPoint(&pStart, &pEnd) < 0) {
            // There was an error in finding the bracket.
            logger("Could not find the deepest point. \r\n");
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

        if (solveExpression(pCalcCoreState, &pResult, pStart, pEnd) < 0) {
            logger("ERROR: Could not solve expression\r\n");
            returnStatus = calc_funStatus_SOLVE_INCOMPLETE;
            break;
        }
        if (pResult == NULL) {
            logger("No result written, but expression solver returned OK. \n");
            returnStatus = calc_funStatus_SOLVE_INCOMPLETE;
            break;
        }
        if ((pResult->pNext == NULL) && (pResult->pPrevious == NULL)) {
            logger("SOLVED! Result is %i\r\n", pResult->entry.subresult);
            pCalcCoreState->result = pResult->entry.subresult;
            solved = true;
            // Free the result buffer as well.
            overloaded_free(pResult);
            pCalcCoreState->allocCounter--;
            break;
        }
    }

    // Free the temporary list if calculation could not be done
    if (!solved) {
        while (pSolverListStart != NULL) {
            inputListEntry_t *pNext = pSolverListStart->pNext;
            if (pSolverListStart != NULL) {
                overloaded_free(pSolverListStart);
                pCalcCoreState->allocCounter--;
            } else {
                break;
            }
            pSolverListStart = pNext;
        }
    }
    return returnStatus;
}

calc_funStatus_t calc_printBuffer(calcCoreState_t *pCalcCoreState,
                                  char *pResString, uint16_t stringLen) {

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
        calc_funStatus_STRING_BUFFER_ERROR;
    }

    // Make a local variable of the string entry to interate on.
    char *pString = pResString;

    // Variable to keep track of the number of chars written to string
    // Add one as as we need a null terminator at the end.
    uint16_t numCharsWritten = 1;

    // Loop through all buffers
    uint8_t previousInputType = INPUT_TYPE_EMPTY;
    while (pCurrentListEntry != NULL) {
        // Depending on the input type, print different things.
        uint8_t currentInputType =
            GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag);

        if (currentInputType == INPUT_TYPE_NUMBER) {
            // If the previous input type wasn't a number,
            // then print the precursor. For hex it's 0x, for bin it's 0b
            if (previousInputType != currentInputType) {
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
            uint8_t tmpStrLen = strlen(pOperator->opString);

            if (numCharsWritten < stringLen - tmpStrLen) {
                numCharsWritten += sprintf(pString, pOperator->opString);
                pString += tmpStrLen;
            } else {
                return calc_funStatus_STRING_BUFFER_ERROR;
            }
            // If the operator increase depth, then print an opening bracket too
            if (GET_DEPTH_FLAG(pCurrentListEntry->entry.typeFlag) ==
                DEPTH_CHANGE_INCREASE) {
                if (numCharsWritten < stringLen) {
                    *pString++ = '(';
                    numCharsWritten++;
                } else {
                    return calc_funStatus_STRING_BUFFER_ERROR;
                }
            }
        } else if (currentInputType == INPUT_TYPE_EMPTY) {
            // This should only be brackets, but it's at least only one char.
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

/* --------------------------------------------
 * ------------- FUNCTION WRAPPERS ------------
 * ---These are exposed for testing purposes---
 * --------------------------------------------*/

int wrap_findDeepestPoint(inputListEntry_t **ppStart,
                          inputListEntry_t **ppEnd) {
    return findDeepestPoint(ppStart, ppEnd);
}