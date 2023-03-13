/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Computer Scientist Calculator (comscicalc) header file
 * 
 * This file contains function prototypes for calculator
 * operators. 
 *  
 */
#ifndef COMSCICALC_OPERATORS_H
#define COMSCICALC_OPERATORS_H
/* -------------------------------------------
 * ----------------- DEFINES ----------------- 
 * -------------------------------------------*/
#define NUM_OPERATORS 32 // Max number of operators
#define OPERATOR_STRING_MAX_LEN 10 // Max length of the operator string

/* -------------------------------------------
 * ----------------- MACROS ------------------ 
 * -------------------------------------------*/
#define CONSTRUCT_OPERATOR(OP, BITWISE, SINGLE_INPUT) ( (OP&0xF) | (BITWISE<<4) | (SINGLE_INPUT<<5))
#define OPERATOR_IS_BITWISE(OP_ID) ( (OP_ID >> 4) & 0x01)
/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Standard library
#include <stdint.h>
#include <stdbool.h>


/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/

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

// Struct for storing operators
typedef struct operatorEntry {
	// Which input corresponds to this function
	char inputChar;
	// What string should be displayed for this operator?
	char opString[OPERATOR_STRING_MAX_LEN];
	// Operator flag
	char op;
	// Flag to be used if depth of the next entry shall be
	// increased. Used for functions with natual brackets. 
	bool bIncDepth;
	// Function pointer to function. NULL if no function
	int32_t (*pFun)(int32_t a, int32_t b);
} operatorEntry_t;

// Struct to be used with a custom function
typedef struct customFunc {
    // Which input corresponds to this function
    char inputChar;
    // What string should be displayed for this operator?
    char opString[OPERATOR_STRING_MAX_LEN];
    // Pointer to custom function
    // NULL if no function defined. 
    int32_t (*pFunc)(void* args); 
    // Number of arguments. 
    // I.e. how many of the following entries should go into 
    // this function
    uint8_t numArgs;
    
} customFunc_t;

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/

const operatorEntry_t operators[NUM_OPERATORS];

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

// Calculator operator functions to be used in "operators" table
int32_t calc_add(int32_t a, int32_t b);
int32_t calc_subtract(int32_t a, int32_t b);
int32_t calc_multiply(int32_t a, int32_t b);
int32_t calc_divide(int32_t a, int32_t b);

int32_t calc_and(int32_t a, int32_t b);
int32_t calc_nand(int32_t a, int32_t b);
int32_t calc_or(int32_t a, int32_t b);
int32_t calc_xor(int32_t a, int32_t b);

int32_t calc_not(int32_t a, int32_t b);

// Custom functions
// int32_t customFunctionName(void* args);


/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/

#endif