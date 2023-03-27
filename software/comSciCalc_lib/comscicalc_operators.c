/*
 * Copyright (c) 2023
 * Oskar von Heideken.
 *
 * Computer Scientist Calculator (comscicalc) C library
 *
 * This file contains operator functions for the comsci
 * calculator. To add a new core function in the operators
 * table, define a int32_t (int32_t, int32_t) function here
 * and comscicalc_operator.h.
 * Then add it to the "operators" table in this file.
 *
 * To add a custom function TBD.
 *
 */

/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */
// Operator functions
#include "comscicalc_operators.h"

// Standard library
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/* ------------- GLOBAL VARIABLES ------------ */
// List of operator function pointers
// Ensure that each inputChar and op field is unique!
const operatorEntry_t operators[NUM_OPERATORS] = {
    // Arithmetic operators, multiple input
    {.inputChar = '+',
     .opString = "+\0",
     .solvPrio = 3,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_add},
    {.inputChar = '-',
     .opString = "-\0",
     .solvPrio = 3,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_subtract},
    {.inputChar = '*',
     .opString = "*\0",
     .solvPrio = 0,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_multiply},
    {.inputChar = '/',
     .opString = "/\0",
     .solvPrio = 1,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_divide},
    {.inputChar = '<',
     .opString = "<<\0",
     .solvPrio = 2,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_leftshift},
    {.inputChar = '>',
     .opString = ">>\0",
     .solvPrio = 2,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_rightshift},
    {.inputChar = 's',
     .opString = "SUM\0",
     .solvPrio = 0,
     .bIncDepth = true,
     .numArgs = -1,
     .pFun = &calc_sum},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = false,
     .numArgs = 0,
     .pFun = NULL},

    // Bitwise operators, mulitple input
    {.inputChar = '&',
     .opString = "AND\0",
     .solvPrio = 0,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_and},
    {.inputChar = 'n',
     .opString = "NAND\0",
     .solvPrio = 0,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_nand},
    {.inputChar = '|',
     .opString = "OR\0",
     .solvPrio = 0,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_or},
    {.inputChar = '^',
     .opString = "XOR\0",
     .solvPrio = 0,
     .bIncDepth = false,
     .numArgs = 2,
     .pFun = &calc_xor},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},

    // Arithmetic operators, single input
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},

    // Bitwise operators, single input
    {.inputChar = '~',
     .opString = "NOT\0",
     .solvPrio = 0,
     .bIncDepth = true,
     .numArgs = 1,
     .pFun = &calc_not},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL},
    {.inputChar = 0,
     .opString = "\0",
     .solvPrio = 255,
     .bIncDepth = true,
     .numArgs = 0,
     .pFun = NULL}};

/* ------ CALCULATOR OPERATOR FUNCTIONS ------ */

// Calculator operator functions to be used in "operators" table
/************************************************************
 *  @brief Function to add two numbers together
 *
 * This function is utilizing the default compiler addition
 * to add two, and only two, arguments.
 *
 * @param   pResult     Pointer to the where the result of
 *                      addition should be placed.
 *                      Note that the type is generic, and
 *                      will be casted to the input format.
 * @param   inputFormat Flag indicating integer, float or
 *                      fixed point format. See INPUT_FMT_<>
 * @param   inputFormat Flag indicating integer, float or
 *                      fixed point format. See INPUT_FMT_<>
 ***********************************************************/
int8_t calc_add(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                int num_args, SUBRESULT_UINT *args) {

    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_UINT a = args[0];
    SUBRESULT_UINT b = args[1];
    printf("adding %i and %i\r\n", a, b);
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
        // Solve for N bit unsigned integer
        (*((SUBRESULT_UINT *)pResult)) = a + b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_SINT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a + b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit floats
        (*((SUBRESULT_FLOAT *)pResult)) = a + b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

int8_t calc_subtract(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                     int num_args, SUBRESULT_UINT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_UINT a = args[0];
    SUBRESULT_UINT b = args[1];
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
        // Solve for N bit unsigned integer
        (*((SUBRESULT_UINT *)pResult)) = a - b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_SINT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a - b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_FLOAT *)pResult)) = a - b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

int8_t calc_multiply(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                     int num_args, SUBRESULT_UINT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_UINT a = args[0];
    SUBRESULT_UINT b = args[1];
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
        // Solve for 32bit unsigned integer
        (*((SUBRESULT_UINT *)pResult)) = a * b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_SINT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a * b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_FLOAT *)pResult)) = a * b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

int8_t calc_divide(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                   int num_args, SUBRESULT_UINT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_UINT a = args[0];
    SUBRESULT_UINT b = args[1];
    if (b == 0) {
        return error_args;
    }
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
        // Solve for 32bit unsigned integer
        (*((SUBRESULT_UINT *)pResult)) = a / b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_SINT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a / b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_FLOAT *)pResult)) = a / b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

int8_t calc_and(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                int num_args, SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_nand(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                 int num_args, SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_or(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args,
               SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_xor(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                int num_args, SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_not(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                int num_args, SUBRESULT_UINT *args) {
    // Only expecting one variable arguments here
    if (num_args != 1) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_UINT a = args[0];
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
    case INPUT_FMT_SINT:
        // Solve for 32bit unsigned or signed integer
        (*((SUBRESULT_UINT *)pResult)) = ~a;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Does not make sense for float. But I'll allow it.
        (*((SUBRESULT_FLOAT *)pResult)) = ~a;
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

int8_t calc_leftshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                      int num_args, SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_rightshift(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                       int num_args, SUBRESULT_UINT *args) {
    return function_solved;
}

int8_t calc_sum(SUBRESULT_UINT *pResult, inputFormat_t inputFormat,
                int num_args, SUBRESULT_UINT *args) {

    if (args == NULL) {
        return incorrect_args;
    }
    if (num_args < 1) {
        return incorrect_args;
    }
    // Make calculation based on format
    switch (inputFormat) {
    case INPUT_FMT_UINT:
        // Solve for N bit unsigned integer
        (*((SUBRESULT_UINT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            printf("Summing number %i\r\n", i);
            (*((SUBRESULT_UINT *)pResult)) += (SUBRESULT_UINT)args[i];
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_SINT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_INT *)pResult)) += (SUBRESULT_INT)args[i];
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit floats
        (*((SUBRESULT_FLOAT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_FLOAT *)pResult)) += (SUBRESULT_FLOAT)args[i];
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        break;
    }
    return function_solved;
}

/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/