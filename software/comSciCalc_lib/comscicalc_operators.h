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
//! Maximum number of operators in the operators list
#define NUM_OPERATORS 32 
//! Maximum length of the operator string
#define OPERATOR_STRING_MAX_LEN 10 
/**
 * @defgroup entryFlagDefs Defines for entry flags
 * @{
 */
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
/**@}*/
/* -------------------------------------------
 * ----------------- MACROS ------------------ 
 * -------------------------------------------*/
/**
 * @defgroup entryFlagMacros Macros for entry flags
 * @{
 */
#define CONSTRUCT_TYPEFLAG(inputFormat, subResType, depthFlag, inputType) (inputFormat << 5 | subResType << 4 | depthFlag << 2 | inputType)
#define GET_DEPTH_FLAG(typeFlag) ((typeFlag>>2)&0x3)
#define GET_INPUT_TYPE(typeFlag) ((typeFlag)&0x3)
#define GET_SUBRESULT_TYPE(typeFlag) ((typeFlag>>4)&0x1)
#define GET_FMT_TYPE(typeFlag) ((typeFlag>>5)&0x3)
/**@}*/



/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
//! Typedef to hold the typeFlag 
typedef uint8_t typeFlag_t;

//! Typedef to hold the input format
typedef uint8_t inputFormat_t;

/**
 * @defgroup subresTypedefs Typedefs for sub results. 
 * @note These define the type for which the result and 
 *   subresults are calculated and displayed with. 
 * @warning All of these must have the same bit length!
 * @{
 */
typedef uint32_t SUBRESULT_UINT;
typedef int32_t SUBRESULT_INT;
typedef double SUBRESULT_FLOAT;
typedef uint32_t SUBRESULT_FIXED;
/**@}*/

/**
 * @brief Typedef for the operator funtion
 * 
 * This typedef is used to typecast the function pointer
 * to the operator to the correct form. 
 * @warning Must have the same format for all operator functions. 
 */
typedef int8_t function_operator(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);

/**
 * @brief Enumeration for operator status
 * 
 * These hold the value which the operator functions 
 * (calc_xxx in this file) shall return with. 
 */
enum functionStatus {
    //! Status if function was solvable
    function_solved		= 0,
    //! Warning if function was overflowing
    function_overflow   = 1, 
    //! Error if the number of arguments is not compatible with the function. 
    incorrect_args 		= -1,
    //! Issue with one of the args, e.g. divion by zero. 
    error_args          = -2,
};



/**
 * @brief Struct for holding operator entry. 
 * 
 * This struct defines the data and metadata needed for the 
 * operator entry list. 
 */
typedef struct operatorEntry {
    /**
	 * @param inputChar Char to which this enties function is associated with. 
	 * 
     * This is what the user enters to get the functionality of this entry. 
	 */
	char inputChar;
	/**
	 * @param opString String associated with this function. 
     * 
     * The user only enters one char to get this function, 
     * but there might be a case where another string should be displayed
     * to the user, for readibility reasons.  
	 */
	char opString[OPERATOR_STRING_MAX_LEN];
	/**
	 * @param solvPrio Priority of solving. 
     * 
     * The priority of which to solve this. E.g. * has higher prio 
     * than + in 123+456*789. Therefore * has higher priority. 
     * @note Priority is ascending, so 0 has the highest priority, 
     *   255 has the lowest. 
	 */
	uint8_t solvPrio;
	/**
	 * @param bIncDepth Flag to increase depth
     * 
     * True if this function increases the depth, i.e. 
     * a bracket is needed after the function. E.g. sin is 
     * a depth increasing, since sin had to be followed by a bracket:
     * sin(x). 
	 */
	bool bIncDepth;
    /**
	 * @param pDoc Pointer to documentation string. 
     * 
     * Points to a \0-terminated string which describes the function. 
     * This will be displayed in the help section in the calculator.  
	 */
    char *pDoc;
	/**
	 * @param pFun Pointer to the calculator function
     * 
     * Points to the function associated with this input. 
     * @note This will be typecasted to the correct format. 
	 */
    void *pFun;
    // Number of arguments. 
    // -1: variable arguments, give input as pointers. 
    // 0 : reserved(use for variable maybe?)
    // >0: number of arguments accepted. 
    // NOTE: for non-depth increasing operators, 
    // this must be 2. All others are ignored. 
    /**
	 * @param numArgs Number of arguments for the calculation function. 
     * 
     * -1 is variable arguments. 
     * 0 is reserved. 
     * >0 means fixed set of arguments.  
     * @note for non-depth increasing operators, 
     *   this must be 2. All others are ignored. 
	 */
    int8_t numArgs;
} operatorEntry_t;


/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/
//! Table of operators and functions. 
const operatorEntry_t operators[NUM_OPERATORS];

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

/**
 * @defgroup calc_functions Calculation functions. 
 * @param pResult Pointer to where the result shall be written
 * @param inputFormat Flag to indicate the format. 
 * @param num_args Number of argument to the calculation function. 
 * @param args Pointer to an array of arguments. Must be num_args long. 
 * @return Status of calculation. 
 * @note The input format shall also dictate the type of calculation. 
 *   For example, a float calculation differs from int calculations, 
 *   and we rely heavily on typecasting here, so the return is only 
 *   interesting from a memory point of view. 
 * @{
 */
//! Function for handling addition. 
int8_t calc_add(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for handling subtraction. 
int8_t calc_subtract(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for handling normal multiplication. 
int8_t calc_multiply(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for handling normal division.
int8_t calc_divide(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for handling bitwise leftshift. Adds zeros
int8_t calc_leftshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for handling bitwise rightshift. Adds zeros
int8_t calc_rightshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating the sum of a variable amount of arguments. 
int8_t calc_sum(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating bitwise AND
int8_t calc_and(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating bitwise NAND
int8_t calc_nand(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating bitwise OR
int8_t calc_or(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating bitwise XOR
int8_t calc_xor(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
//! Function for calculating bitwise NOT
int8_t calc_not(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
/**@}*/

#endif