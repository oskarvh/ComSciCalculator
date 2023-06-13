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

#ifndef COMSCICALC_H
#define COMSCICALC_H
/* -------------------------------------------
 * ----------------- DEFINES -----------------
 * -------------------------------------------*/
#define OPENING_BRACKET '('
#define CLOSING_BRACKET ')'

/* -------------------------------------------
 * ----------------- MACROS ------------------
 * -------------------------------------------*/

/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Operator functions
#include "comscicalc_operators.h"

// Standard library
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
//! Typedef for calculator function status
typedef int8_t calc_funStatus_t;
//! Typedef for input entry status
typedef int8_t inputModStatus_t;

/**
 *  @brief Enumeration of input base (dec, hex, bin or none)
 */
enum inputBase {
    //! Base 10, i.e. decimal.
    inputBase_DEC = 0,
    //! Base 16, i.e. hexadecimal.
    inputBase_HEX = 1,
    //! Base 2, i.e. binary.
    inputBase_BIN = 2,
    //! Base not defined.
    inputBase_NONE = -1,
};

/**
 * @brief Common status enumeration for calculator core
 * functions.
 */
enum calc_funStatus {
    //! Success:Function returned with success
    calc_funStatus_SUCCESS = 0,
    //! Error: Pointer to list entry was NULL
    calc_funStatus_INPUT_LIST_NULL = 1,
    //! Error: Pointer to calculator core was NULL.
    calc_funStatus_CALC_CORE_STATE_NULL = 2,
    //!  Error: Base was NONE, but tried to use it.
    calc_funStatus_INPUT_BASE_ERROR = 3,
    //! Error: Unrecognized input character.
    calc_funStatus_UNKNOWN_INPUT = 4,
    //! Error: Malloc not possible.
    calc_funStatus_ALLOCATE_ERROR = 6,
    //! Error: Pointer to string buffer was NULL.
    calc_funStatus_STRING_BUFFER_ERROR = 7,
    //! Error: Entry list is not healthy.
    calc_funStatus_ENTRY_LIST_ERROR = 8,
    //! Error: Teardown of calculator core incomplete. Memory leak exist.
    calc_funStatus_TEARDOWN_INCOMPLETE = 9,
    //! Warning: Could not solve expression.
    calc_funStatus_SOLVE_INCOMPLETE = 10,
};

/**
 * @brief Status of the solve expression function.
 */
enum calc_solveStatus {
    //! Success:Function returned with success.
    calc_solveStatus_SUCCESS = 0,
    //! Error: Pointer to list entry was NULL.
    calc_solveStatus_INPUT_LIST_NULL = -1,
    //! Error: Bracket mismatch.
    calc_solveStatus_BRACKET_ERROR = -2,
    //!  Error: Error with input list.
    calc_solveStatus_INPUT_LIST_ERROR = -3,
    //! Error: Operator pointer was NULL.
    calc_solveStatus_OPERATOR_POINTER_ERROR = -4,
    //! Error: Calculation could not be made.
    calc_solveStatus_CALC_NOT_SOLVABLE = -5,
    //! Error: Invalid number of arguments.
    calc_solveStatus_INVALID_NUM_ARGS = -6,
    //! Error: Malloc not possible.
    calc_solveStatus_ALLOCATION_ERROR = -7,
    //! Error: There was arguments, but no operator
    calc_solveStatus_ARGS_BUT_NO_OPERATOR = -8,
    //! Error: Invalid arguements.
    calc_solveStatus_INVALID_ARGS = -9,
};

/**
 * @brief Status for input modification functions.
 */
enum inputModStatus {
    //! Function returned with success.
    inputModStatus_SUCCESS = 0,
    //! Error: Cursor is larger than number of list entries.
    inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY = -2,
    //! Error: Pointer to list entry was NULL.
    inputModStatus_INPUT_LIST_NULL = -1,
};

/**
 * @brief Struct for holding input data and sub result data.
 *
 * This struct holds the data entered by the user, along with
 * storing sub results used while solving the expression.
 */
typedef struct inputType {
    /**
     * @param c Input character.
     */
    char c;
    /**
     * @param typeFlag Flag holding metadata of the entry
     *
     * Call me old school but I try to avoid bitfields.
     * bit 0-1: 0 = empty, 1 = number, 3 = operator, 3 = custom function.
     * bit 2-3: 0 = keep depth, 1 = increase depth, 2 = decrease depth, 3 =
     * reserved.
     * bit 4: 0 = char input, 1 = subresult.
     * Bit 5-6: Input is: 0 = int, 1 = floating point
     * 2 = fixed point, 3 = reserved
     * bit 7: 1 = signed, 0 = unsigned.
     */
    typeFlag_t typeFlag;

    //! Partial restult. Only used for when solving.
    SUBRESULT_INT subresult;
} inputType_t;

/**
 * @brief Struct for input entry.
 *
 * An input list entry is one entry in a doubly
 * linked list, responsible for:
 * 1. Holding the input from the user, and
 * 2. Temporary holding the converted input while solving.
 */
typedef struct inputListEntry {
    /**
     * @param pPrevious Pointer to previous entry in the list.
     * @note If no previous instance available, this shall be NULL
     */
    void *pPrevious;

    /**
     * @param pNext Pointer to next entry in the list.
     * @note If this is the last entry, this shall be NULL
     */
    void *pNext;

    /**
     * @param entry Struct that holds data
     * @note This holds characters coming from the user, as well as
     * sub-restuls used while solving.
     */
    inputType_t entry;

    /**
     * @param inputBase Input base for this entry
     * @note This is the base for this entry only, can be
     * different from system wide input base, if base has been
     * changed after input was entered.
     */
    inputBase_t inputBase;

    /**
     * @param pFunEntry Pointer to function.
     * @note This is a pointer to an operator entry, not to
     * the function directly. If not an operator, this can be NULL.
     */
    void *pFunEntry;
} inputListEntry_t;

/**
 * @brief Struct holding the calculator core state.
 */
typedef struct calcCoreState {
    /**
     * @param pListEntrypoint Entry point of the input list.
     */
    inputListEntry_t *pListEntrypoint;

    /**
     * @param cursorPosition Position of the cursor
     * @note 0 is the last (rightmost) position.
     */
    uint8_t cursorPosition;

    /**
     * @param allocCounter Counter of malloc's and free's.
     */
    uint8_t allocCounter;

    /**
     * @param solved True if list has been (or can be) solved. False if not.
     */
    bool solved;

    /**
     * @param result Result of current buffer if #solved is true
     */
    SUBRESULT_INT result;

    /**
     * @param numberFormat The current number format.
     */
    numberFormat_t numberFormat;

} calcCoreState_t;

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/
/**
 * @brief Logger function to abstract away printf's
 * @param msg Variable args to send to printf
 * This is just a very simple logger that uses a define
 * to either print or not.
 */
void logger(char *msg, ...);
/**
 * @brief Initalize calculator core
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of the initializaition.
 */
calc_funStatus_t calc_coreInit(calcCoreState_t *pCalcCoreState);
/**
 * @brief Tear down calculator core
 *
 * This free's all the allocated entries in the list
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of the teardown.
 * @note This only frees the lists pointed to by the core state.
 *   Not any temporary lists created within.
 */
calc_funStatus_t calc_coreBufferTeardown(calcCoreState_t *pCalcCoreState);
/**
 * @brief Add input character at cursor value
 *
 * Adds numeric or operator entry to the list
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @param inputChar Input character coming from the user input.
 * @return Status of the list addition.
 */
calc_funStatus_t calc_addInput(calcCoreState_t *pCalcCoreState, char inputChar);
/**
 * @brief Remove character at cursor value
 *
 * Removes numeric or operator entry at cursor from the list.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of the list deletion.
 */
calc_funStatus_t calc_removeInput(calcCoreState_t *pCalcCoreState);
/**
 * @brief Prints the buffer in a readable format to a string.
 *
 * Prints the content of the input buffer.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @param pResString Pointer to a location which to write the string.
 * @param stringLen Maximum length of the #pResString.
 * @param pSyntaxIssuePos Pointer to a variable to collect where the
 * first syntax issue is located. -1 if no syntax issues
 * @return Status of printing to buffer.
 */
calc_funStatus_t calc_printBuffer(calcCoreState_t *pCalcCoreState,
                                  char *pResString, uint16_t stringLen,
                                  int16_t *pSyntaxIssuePos);
/**
 * @brief Tries to solve the current buffer.
 *
 * This attempts to solve the current buffer and reflect the result
 * in the #pCalcCoreState.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Status of solving the buffer.
 */
calc_funStatus_t calc_solver(calcCoreState_t *pCalcCoreState);

/**
 * @brief Get the offset from the end of the buffer.
 *
 * This function returns the number of characters between the cursor
 * location and the end of the input text, i.e. cursor = 0.
 * This is not the same as counting the number of inputs, as some
 * inputs such as SUM has more chars than 1 representing the
 * operator.
 * @param pCalcCoreState Pointer to an allocated core state variable.
 * @return Number of chars from the end the cursor is at.
 */
uint8_t calc_getCursorLocation(calcCoreState_t *pCalcCoreState);

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/

/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/

#endif
