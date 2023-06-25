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
#include "comscicalc_common.h"
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
/* -------------------------------------------
 * ----------------- DEFINES -----------------
 * -------------------------------------------*/
//! Maximum number of operators in the operators list
#define NUM_OPERATORS 32

/**
 * @brief Enumeration for operator status
 *
 * These hold the value which the operator functions
 * (calc_xxx in this file) shall return with.
 */
enum functionStatus {
    //! Status if function was solvable
    function_solved = 0,
    //! Warning if function was overflowing
    function_overflow = 1,
    //! Error if the number of arguments is not compatible with the function.
    incorrect_args = -1,
    //! Issue with one of the args, e.g. divion by zero.
    error_args = -2,
    //!
    format_not_supported = -4,
};

/* -------------------------------------------
 * ---------------- VARIABLES ----------------
 * -------------------------------------------*/
//! Table of operators and functions.
const operatorEntry_t operators[NUM_OPERATORS];

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

/**
 * @brief Typedef for the operator funtion
 *
 * This typedef is used to typecast the function pointer
 * to the operator to the correct form.
 * @warning Must have the same format for all operator functions.
 */
typedef int8_t function_operator(SUBRESULT_INT *pResult,
                                 numberFormat_t numberFormat, int num_args,
                                 inputType_t *pArgs);

/**
 * @defgroup calc_functions Calculation functions.
 * @param pResult Pointer to where the result shall be written
 * @param numberFormat Number format
 * @param num_args Number of argument to the calculation function.
 * @param pArgs Pointer to an array of arguments. Must be num_args long.
 * @return Status of calculation.
 * @note The input format shall also dictate the type of calculation.
 *   For example, a float calculation differs from int calculations,
 *   and we rely heavily on typecasting here, so the return is only
 *   interesting from a memory point of view.
 * @{
 */
//! Function for handling addition.
int8_t calc_add(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs);
//! Function for handling subtraction.
int8_t calc_subtract(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                     int num_args, inputType_t *pArgs);
//! Function for handling normal multiplication.
int8_t calc_multiply(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                     int num_args, inputType_t *pArgs);
//! Function for handling normal division.
int8_t calc_divide(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                   int num_args, inputType_t *pArgs);
//! Function for handling bitwise leftshift. Adds zeros
int8_t calc_leftshift(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                      int num_args, inputType_t *pArgs);
//! Function for handling bitwise rightshift. Adds zeros
int8_t calc_rightshift(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                       int num_args, inputType_t *pArgs);
//! Function for calculating the sum of a variable amount of arguments.
int8_t calc_sum(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs);
//! Function for calculating bitwise AND
int8_t calc_and(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs);
//! Function for calculating bitwise NAND
int8_t calc_nand(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                 int num_args, inputType_t *pArgs);
//! Function for calculating bitwise OR
int8_t calc_or(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
               int num_args, inputType_t *pArgs);
//! Function for calculating bitwise XOR
int8_t calc_xor(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs);
//! Function for calculating bitwise NOT
int8_t calc_not(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs);
/**@}*/

#endif
