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
int8_t calc_add(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, SUBRESULT_INT *args) {

    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on
    SUBRESULT_INT a = args[0];
    SUBRESULT_INT b = args[1];
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a + b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        if (numberFormat.numBits == 32) {
            // Solve for float
        } else if (numberFormat.numBits == 64) {
            // Solve for double
        } else {
            // Format is not supporteds
            return format_not_supported;
        }
        // Solve for 32bit floats
        (*((SUBRESULT_INT *)pResult)) = a + b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_subtract(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                     int num_args, SUBRESULT_INT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = args[0];
    SUBRESULT_INT b = args[1];
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a - b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_INT *)pResult)) = a - b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_multiply(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                     int num_args, SUBRESULT_INT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = args[0];
    SUBRESULT_INT b = args[1];
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a * b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_INT *)pResult)) = a * b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_divide(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                   int num_args, SUBRESULT_INT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = args[0];
    SUBRESULT_INT b = args[1];
    if (b == 0) {
        return error_args;
    }
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a / b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit float
        (*((SUBRESULT_INT *)pResult)) = a / b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_and(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, SUBRESULT_INT *args) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = args[0];
    SUBRESULT_INT b = args[1];
    if (b == 0) {
        return error_args;
    }
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a & b;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_nand(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                 int num_args, SUBRESULT_INT *args) {
    return function_solved;
}

int8_t calc_or(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
               int num_args, SUBRESULT_INT *args) {
    return function_solved;
}

int8_t calc_xor(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, SUBRESULT_INT *args) {
    return function_solved;
}

int8_t calc_not(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, SUBRESULT_INT *args) {
    // Only expecting one variable arguments here
    if (num_args != 1) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = args[0];
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for 32bit unsigned or signed integer
        (*((SUBRESULT_INT *)pResult)) = ~a;
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Does not make sense for float. But I'll allow it.
        (*((SUBRESULT_INT *)pResult)) = ~a;
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

int8_t calc_leftshift(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                      int num_args, SUBRESULT_INT *args) {
    return function_solved;
}

int8_t calc_rightshift(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                       int num_args, SUBRESULT_INT *args) {
    return function_solved;
}

int8_t calc_sum(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, SUBRESULT_INT *args) {

    if (args == NULL) {
        return incorrect_args;
    }
    if (num_args < 1) {
        return incorrect_args;
    }
    // Make calculation based on format
    switch (numberFormat.formatBase) {
    case INPUT_FMT_INT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_INT *)pResult)) += (SUBRESULT_INT)args[i];
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit floats
        (*((SUBRESULT_INT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_INT *)pResult)) += (SUBRESULT_INT)args[i];
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FIXED:
        // TODO:Solve for fixed point.
        // TODO: add overflow detection
        return format_not_supported;
        break;
    }
    return function_solved;
}

/* -------------------------------------------
 * ------------- DEBUG FUNCTIONS -------------
 * -------------------------------------------*/
