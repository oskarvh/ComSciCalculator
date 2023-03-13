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

/* -------------------------------------------
 * ----------------- MACROS ------------------ 
 * -------------------------------------------*/

/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
// Enum for the possible type of input.
enum inputBase {
    inputBase_DEC		= 0,
    inputBase_HEX 		= 1,
    inputBase_BIN 		= 2,
    // Not assigned:
    inputBase_NONE 		= -1,
};

// status of calc_* functions
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

// Struct used for the input string linked list
typedef struct inputStringEntry {
	// Pointer to previous entry. NULL if first element in list
	void *pPrevious;
	// Pointer to next entry. NULL if last element in list
	void * pNext;
	// Character in this entry.
	char c;

	uint8_t padding0;
	uint16_t padding1;
} inputStringEntry_t;

// Struct for the input linked list entry
typedef struct inputListEntry {
	// Pointer to previous entry in the list. 
	// If no previous instance available, this should be NULL
	void *pPrevious;

	// Pointer to the next entry in the list.
	// If no next entry available, this should be NULL
	void *pNext;

	// Pointer to input string linked list entry
	// NULL if no entry is made
	inputStringEntry_t *pInputStringEntry;

	// Pointer to last entry of string linked list
	// NULL if no entry is made
	inputStringEntry_t *pLastInputStringEntry;

	// Custom function. 
	// If this is defined, then no inputstring should be defined, 
	// as the next N number of entries should be the 
	// input to this function. 
	void *pCustomFunction;

	// Operator to next entry
	// This operator is acting between this entry and next entry
	operators_t op;

	// Input format in this entry
	// The entire entry must be in the same base
	inputBase_t inputBase;

	// Depth of this operator, i.e. how many brackets deep is 
	// this entry. 
	uint8_t depth;

	uint8_t padding;
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

	uint16_t padding;
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

// Calculator operator functions
// NOTE: all operators must have 
int32_t calc_add(int32_t a, int32_t b);
int32_t calc_subtract(int32_t a, int32_t b);
int32_t calc_multiply(int32_t a, int32_t b);
int32_t calc_divide(int32_t a, int32_t b);

int32_t calc_and(int32_t a, int32_t b);
int32_t calc_nand(int32_t a, int32_t b);
int32_t calc_or(int32_t a, int32_t b);
int32_t calc_xor(int32_t a, int32_t b);

int32_t calc_not(int32_t a, int32_t b);

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/



/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/
inputModStatus_t getInputListEntryWrapper(
	calcCoreState_t *calcCoreState,
	inputListEntry_t **ppInputListAtCursor, 
	inputStringEntry_t **ppInputString
);
#endif