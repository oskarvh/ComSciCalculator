/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Computer Scientist Calculator (comscicalc) header file
 * 
 * This file contains function prototypes, type declarations and 
 * more to be used by the comscicalc library. 
 *  
 */

#ifndef COMSCICALC_H
#define COMSCICALC_H
/* -------------------------------------------
 * ----------------- DEFINES ----------------- 
 * -------------------------------------------*/
#define OPENING_BRACKET '('
#define CLOSING_BRACKET ')'
// Defines that dictate the sub result and
// result bit length

/* -------------------------------------------
 * ----------------- MACROS ------------------ 
 * -------------------------------------------*/

/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Operator functions
#include "comscicalc_operators.h"

// Standard library
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
// Enum for the possible type of input.
// TODO: add support for float and fixed point here
enum inputBase {
    inputBase_DEC		= 0,
    inputBase_HEX 		= 1,
    inputBase_BIN 		= 2,
    // Not assigned:
    inputBase_NONE 		= -1,
};

// status of calc_* functions
// TODO: add soft errors and hard errors here
enum calc_funStatus {
	calc_funStatus_SUCCESS 				= 0, 
	// Input list pointer is NULL
	calc_funStatus_INPUT_LIST_NULL 		= 1,
	// Pointer to core state is NULL
	calc_funStatus_CALC_CORE_STATE_NULL = 2,
	// Tried to add input in the middle of 
	// string buffer, where the input base didn't correspond
	// between input and existing entry. 
	calc_funStatus_INPUT_BASE_ERROR		= 3,
	// Unknown input
	calc_funStatus_UNKNOWN_INPUT		= 4,
	// Erronous input state. 
	calc_funStatus_ERROR                = 5,
	// Could not allocate a new buffer. 
	calc_funStatus_ALLOCATE_ERROR		= 6,
	// String buffer was broken
	calc_funStatus_STRING_BUFFER_ERROR	= 7,
	// Entry list was broken
	calc_funStatus_ENTRY_LIST_ERROR		= 8,
	// Teardown of list entries incomplete
	calc_funStatus_TEARDOWN_INCOMPLETE 	= 9,
	// Solver could not be completed
	calc_funStatus_SOLVE_INCOMPLETE 	= 10,
};

// Status of functions handling input lists
enum inputModStatus {
	inputModStatus_SUCCESS = 0,
	// Cursor value larger than list entries
	inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY = -2,
	// Input list pointer is NULL
	inputModStatus_INPUT_LIST_NULL 	= -1,
};

typedef uint8_t inputBase_t;

typedef char operators_t;
typedef int8_t calc_funStatus_t;
typedef int8_t inputModStatus_t;



// Struct to be used for operator or char entry
typedef struct inputType {
	// The character from the input
	char c;
	// Flags used for identifying what
	// type of input this is. 
	// Call me old school but I try to avoid
	// bitfields. 
	// bit 0-1: 0 = empty, 1 = number, 3 = operator, 3 = custom function
	// bit 2-3: 0 = keep depth, 1 = increase depth, 2 = decrease depth, 3 = reserved
	// bit 4: 0 = char input, 1 = subresult
	// Bit 5-6: Input is: 0 = unsigned int, 1 = signed int, 2 = floating point, 3 = fixed point
	// bit 7: reserved.
	typeFlag_t typeFlag;

	// Partial restult. Only used for when solving. 
	SUBRESULT_UINT subresult;
} inputType_t;

// Struct for the input linked list entry
typedef struct inputListEntry {
	// Pointer to previous entry in the list. 
	// If no previous instance available, this should be NULL
	void *pPrevious;

	// Pointer to the next entry in the list.
	// If no next entry available, this should be NULL
	void *pNext;

	// Character for this entry. 
	// bit 0-7: input character
	// bit 8-9: type (operator, number, function, bracket)
	inputType_t entry;

	// Input format in this entry
	// The entire string must be in the same entry. 
	inputBase_t inputBase;

	// Pointer to either operator or custom function
	// entry. NOTE: not to the actual function! 
	void *pFunEntry;
} inputListEntry_t;

// Struct to handle the calculator core state
// These settings are global and reflect the state 
// of the calculator settings based on user input
typedef struct calcCoreState {
	// Pointer to the starting point of the
	// input list. This should always exist. 
	inputListEntry_t *pListEntrypoint;

	// Position of the cursor from the last entry
	// i.e. 0 means cursor is at last position, 
	// 1 means that the position of the cursor is between 
	// last and second to last characters
	uint8_t cursorPosition;

	// The base of incoming characters. 
	// Note that this is different than the input entry
	// base, as this will be used when new entires are made. 
	inputBase_t inputBase;

	// The input format, signed or unsigned int, float or fixed point
	inputFormat_t inputFormat;

	// DEBUG: counter for the number of free's and malloc's
    uint8_t allocCounter;

    // Bool indicating if the current buffer has been 
    // solved or not. 
	bool solved;

	// Result of the current buffer
	SUBRESULT_UINT result;

} calcCoreState_t;




/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/
// Calculator core functions
calc_funStatus_t calc_coreInit(calcCoreState_t *pCalcCoreState);
calc_funStatus_t calc_coreBufferTeardown(calcCoreState_t *pCalcCoreState);
calc_funStatus_t calc_addInput(calcCoreState_t* pCalcCoreState, char inputChar);
calc_funStatus_t calc_removeInput(calcCoreState_t* pCalcCoreState);
calc_funStatus_t calc_printBuffer(calcCoreState_t* pCalcCoreState, char *pResString, uint16_t stringLen);
calc_funStatus_t calc_solver(calcCoreState_t* pCalcCoreState);


/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/


/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/
int wrap_findDeepestPoint(inputListEntry_t **ppStart, inputListEntry_t **ppEnd);

#endif