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

/* -------------------------------------------
 * ----------------- DEFINES ----------------- 
 * -------------------------------------------*/
#define NUM_OPERATORS 32 // Max number of operators
/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Standard library
#include <stdint.h>


/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/

// Struct for storing operators
typedef struct operatorEntry {
	// Which input corresponds to this function
	char inputChar;
	// What string should be displayed for this operator?
	char opString[OPERATOR_STRING_MAX_LEN];
	// Operator flag
	operators_t op;
	// Flag to be used if depth of the next entry shall be
	// increased. Used for functions with natual brackets. 
	bool bIncDepth;
	// Function pointer to function. NULL if no function
	int32_t (*pFun)(int32_t a, int32_t b);
} operatorEntry_t;

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/

operatorEntry_t operators[NUM_OPERATORS];

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

