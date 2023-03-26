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
 * ----------------- HEADERS -----------------
 * -------------------------------------------*/
// Standard library
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

/* -------------------------------------------
 * ----------------- DEFINES ----------------- 
 * -------------------------------------------*/
#define NUM_OPERATORS 32 // Max number of operators
#define OPERATOR_STRING_MAX_LEN 10 // Max length of the operator string
//#define SUBRESULT_UINT uint32_t
//#define SUBRESULT_INT int32_t
//#define SUBRESULT_FLOAT double
//#define SUBRESULT_FIXED uint32_t


#define INPUT_TYPE_EMPTY 0
#define INPUT_TYPE_NUMBER 1
#define INPUT_TYPE_OPERATOR 2
#define INPUT_TYPE_RESERVED 3
#define DEPTH_CHANGE_KEEP 0
#define DEPTH_CHANGE_INCREASE 1
#define DEPTH_CHANGE_DECREASE 2
#define DEPTH_CHANGE_RESERVED 3
#define SUBRESULT_TYPE_CHAR 0
#define SUBRESULT_TYPE_INT 1
#define INPUT_FMT_UINT 0
#define INPUT_FMT_SINT 1
#define INPUT_FMT_FLOAT 2
#define INPUT_FMT_FIXED 3

/* -------------------------------------------
 * ----------------- MACROS ------------------ 
 * -------------------------------------------*/
#define CONSTRUCT_TYPEFLAG(inputFormat, subResType, depthFlag, inputType) (inputFormat << 5 | subResType << 4 | depthFlag << 2 | inputType)
#define GET_DEPTH_FLAG(typeFlag) ((typeFlag>>2)&0x3)
#define GET_INPUT_TYPE(typeFlag) ((typeFlag)&0x3)
#define GET_SUBRESULT_TYPE(typeFlag) ((typeFlag>>4)&0x1)
#define GET_FMT_TYPE(typeFlag) ((typeFlag>>5)&0x3)




/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
// Type for the character flags. 
typedef uint8_t typeFlag_t;
typedef uint8_t inputFormat_t;
// Typedefs for subresult types. 
// NOTE: ALL THESE MUST BE THE SAME BIT LENGTH!
typedef uint32_t SUBRESULT_UINT;
typedef int32_t SUBRESULT_INT;
typedef double SUBRESULT_FLOAT;
typedef uint32_t SUBRESULT_FIXED;

enum functionStatus {
    // Status if function was solvable
    function_solved		= 0,
    // Warning if function was overflowing
    function_overflow   = 1, 
    // Error if the number of arguments
    // is not compatible with the function. 
    incorrect_args 		= -1,
};

typedef int8_t function_operator(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);

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
    // -1: variable arguments, give input as pointers. 
    // 0 : reserved(use for variable maybe?)
    // >0: number of arguments accepted. 
    // NOTE: for non-depth increasing operators, 
    // this must be 2. All others are ignored. 
    int8_t numArgs;
} operatorEntry_t;


/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/
const operatorEntry_t operators[NUM_OPERATORS];

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

// Calculator operator functions to be used in "operators" table
int8_t calc_add(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_subtract(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_multiply(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_divide(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_leftshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_rightshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_sum(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);

int8_t calc_and(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_nand(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_or(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
int8_t calc_xor(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);

int8_t calc_not(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);



/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/

#endif