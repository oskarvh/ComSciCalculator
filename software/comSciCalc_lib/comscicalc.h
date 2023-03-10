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
#define OPERATOR_STRING_MAX_LEN 10 // Max length of the operator string
#define CONSTRUCT_OPERATOR(OP, BITWISE, SINGLE_INPUT) ( (OP&0xF) | (BITWISE<<4) | (SINGLE_INPUT<<5))
#define OPERATOR_IS_BITWISE(OP_ID) ( (OP_ID >> 4) & 0x01)
#define OPENING_BRACKET '('
#define CLOSING_BRACKET ')'
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

// Enum for the supported operators
// bit 7-6: Reserved
// bit 5: 1 = single input, 0 = multiple input
// bit 4: 1 = bitwise, 0 = arithmetic
// bit 3-0: Operator (0 is reserved, must be 1-15)
enum op_id {
	// Artihmetic operators, multiple input
    operators_ADD 		= CONSTRUCT_OPERATOR(1, 0, 0),
    operators_SUBTRACT 	= CONSTRUCT_OPERATOR(2, 0, 0),
    operators_MULTI 	= CONSTRUCT_OPERATOR(3, 0, 0),
    operators_DIVIDE	= CONSTRUCT_OPERATOR(4, 0, 0), 

    // Arithmetic operators, single input
    // Bitwise operators, multiple input
    operators_AND 		= CONSTRUCT_OPERATOR(1, 1, 0),
    operators_NAND 		= CONSTRUCT_OPERATOR(2, 1, 0),
    operators_OR 		= CONSTRUCT_OPERATOR(3, 1, 0),
    operators_XOR 		= CONSTRUCT_OPERATOR(4, 1, 0), 

    // Arithmetic operators, single input
    // EXAMPLE = (operators_t)CONSTRUCT_OPERATOR(n, 0, 1),
    // This could be SIN, COS etc.  
    // Bitwise operators, single input
    operators_NOT 		= CONSTRUCT_OPERATOR(1, 1, 1),

    // Not assigned:
    operators_NONE 		= 0x00
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
typedef uint8_t operators_t;
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

// Struct to be used with a custom function
typedef struct customFunc {
	// Pointer to custom function
	// NULL if no function defined. 

	int32_t (*pFunc)(void* args); 
	// Number of arguments. 
	// I.e. how many of the following entries should go into 
	// this function
	uint8_t numArgs;

	uint8_t padding0;
	uint16_t padding1;
} customFunc_t;

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
	customFunc_t customFunction;

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
	inputListEntry_t *pInputList, 
	uint8_t cursorPosition,
	inputListEntry_t **ppInputListAtCursor, 
	inputStringEntry_t **ppInputString
);
