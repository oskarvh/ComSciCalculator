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

/* -------------------------------------------
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Standard library
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>


/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/


// Struct for storing operators
typedef struct operatorEntry {
	// Which input corresponds to this function
	char inputChar;
	// What string should be displayed for this operator?
	char opString[OPERATOR_STRING_MAX_LEN];
	// Solving priority
	uint8_t solvPrio;
	// Flag to be used if depth of the next entry shall be
	// increased. Used for functions with natual brackets. 
	bool bIncDepth;
    // Pointer to documentation for the function. 
    char *pDoc;
	// Function pointer to function. NULL if no function
	//int32_t (*pFun)(int32_t a, int32_t b);
    void *pFun;
    // Number of arguments. 
    // NOTE: for operators that does not increase depth, 
    // the number of arguments is always 2, and therefore this
    // parameter will be ignored. 
    uint8_t numArgs;
} operatorEntry_t;


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
int32_t calc_leftshift(int32_t a, int32_t b);
int32_t calc_rightshift(int32_t a, int32_t b);

int32_t calc_and(int32_t a, int32_t b);
int32_t calc_nand(int32_t a, int32_t b);
int32_t calc_or(int32_t a, int32_t b);
int32_t calc_xor(int32_t a, int32_t b);

int32_t calc_not(int32_t a);

// Custom functions
// int32_t customFunctionName(void* args);


/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/

#endif