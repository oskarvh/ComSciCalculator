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

/* -------------------------------------------
 * ----------------- DEFINES ----------------- 
 * -------------------------------------------*/

/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* -------------------------------------------
 * ------------ ENUMS AND STRUCTS ------------
 * -------------------------------------------*/
// Enum for the possible type of input.
typedef enum inputBase {
    inputBase_DEC		= 0,
    inputBase_HEX 		= 1,
    inputBase_BIN 		= 2,
    // Not assigned:
    inputBase_NONE 		= -1,
} inputBase_t;

// Enum for the supported operators
// bit 7: 1 = single input, 0 = multiple input
// bit 6-5: Not in use
// bit 4: 1 = bitwise, 0 = arithmetic
// bit 3-0: Operator
typedef enum operators {
	// Artihmetic operators, multiple input
    operators_ADD 		= 0x01,
    operators_SUBTRACT 	= 0x02,
    operators_MULTI 	= 0x03,
    operators_DIVIDE	= 0x04, 
    // Arithmetic operators, single input
    // Bitwise operators, multiple input
    operators_AND 		= 0x11,
    operators_NAND 		= 0x12,
    operators_OR 		= 0x13,
    operators_XOR 		= 0x14, 
    // Arithmetic operators, single input
    // EXAMPLE = 0x8x. 
    // Bitwise operators, single input
    operators_NOT 		= 0x91,
    // Not assigned:
    operators_NONE 		= 0x00,
} operators_t;

// status of calc_ functions
typedef enum calc_funStatus {
	calc_funStatus_SUCCESS 				= 0, 
	// Input list pointer is NULL
	calc_funStatus_INPUT_LIST_NULL 		= -1,
	// Pointer to core state is NULL
	calc_funStatus_CALC_CORE_STATE_NULL = -2,
} calc_funStatus_t;

// Status of functions handling input lists
typedef enum inputModStatus {
	inputModStatus_SUCCESS = 0,
	// Cursor is at/in string. NOT USED AT THE MOMENT
	// LIST_AT_STRING_ENTRY = 1, 
	// Cursor at end operator
	inputModStatus_LIST_AT_OPERATOR_ENTRY = 2,
	// Cursor at preceding operator
	inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY = 3,
	// Cursor value larger than list entries
	inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY = -2,
	// Input list pointer is NULL
	inputModStatus_INPUT_LIST_NULL 	= -1,
} inputModStatus_t;

// Struct used for the input string linked list
typedef struct inputStringEntry {
	// Pointer to previous entry. NULL if first element in list
	void *pPrevious;
	// Pointer to next entry. NULL if last element in list
	void * pNext;
	// Character in this entry.
	char c;
} inputStringEntry_t;

// Struct to be used with a custom function
typedef struct customFunc {
	// Pointer to custom function
	// NULL if no function defined. 
	void *pFunc; 
	// Number of arguments. 
	// I.e. how many of the following entries should go into 
	// this function
	uint8_t numArgs;
} customFunc_t;

// Struct for the input linked list entry
typedef struct inputListEntry {
	// Pointer to previous entry in the list. 
	// If no previous instance available, this should be NULL
	void *pPrevious;

	// Pointer to the next entry in the list.
	// If no next entry available, this should be NULL
	void *pNext;

	// Input format in this entry
	// The entire entry must be in the same base
	inputBase_t inputBase;

	// Pointer to input string linked list entry
	// NULL if no entry is made
	inputStringEntry_t *pInputStringEntry;

	// Pointer to last entry of string linked list
	// NULL if no entry is made
	inputStringEntry_t *pLastInputStringEntry;

	// Operator to next entry
	// This operator is acting between this entry and next entry
	operators_t opNext;

	// Operator on this entry
	// This operator is acting on this entry, e.g. NOT. 
	// These operators should only be single input operators(bit 7 = 1)
	operators_t opThis;

	// Custom function. 
	// If this is defined, then no inputstring should be defined, 
	// as the next N number of entries should be the 
	// input to this function. 
	customFunc_t customFunction;

	// Depth of this operator, i.e. how many brackets deep is 
	// this entry. 
	uint8_t depth;
} inputListEntry_t;

// Struct to handle the calculator core state
// These settings are global and reflect the state 
// of the calculator settings based on user input
typedef struct calcCoreState {
	// Position of the cursor from the last entry
	// i.e. 0 means cursor is at last position, 
	// 1 means that the position of the cursor is between 
	// last and second to last characters
	uint8_t cursorPosition;
} calcCoreState_t;

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/

// List of operator function pointers


/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/
calc_funStatus_t calc_addInput(inputListEntry_t *pInputList, calcCoreState_t* pCalcCoreState);
calc_funStatus_t calc_removeInput(inputListEntry_t *pInputList, calcCoreState_t* pCalcCoreState);
inputModStatus_t getInputListEntry(
	inputListEntry_t *pInputList, 
	uint8_t cursorPosition,
	inputListEntry_t **ppInputListAtCursor, 
	inputStringEntry_t **ppInputString
);