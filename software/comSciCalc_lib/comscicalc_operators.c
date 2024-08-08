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

// Debug:
#include "uart_logger.h"

// Standard library
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

SUBRESULT_INT promoteOrder(SUBRESULT_INT subresult, uint8_t currentOrder,
                           uint8_t higherOrder, numberFormat_t numberFormat) {
    SUBRESULT_INT result = subresult;
    if (higherOrder == INPUT_FMT_FLOAT) {
        if (currentOrder == INPUT_FMT_INT) {
            if (numberFormat.numBits == 32) {
                // Convert to float
                float res = subresult * 1.0;
                return (SUBRESULT_INT)res;
            } else if (numberFormat.numBits == 64) {
                // Convert to double
                double res = subresult * 1.0;
                return (SUBRESULT_INT)res;
            }
            // No else here, only 32 and 64 bits are supported.
        } else if (currentOrder == INPUT_FMT_FIXED) {
            // Convert the integer part and decimal parts
            // by themselves
            uint16_t numBitsDecimal = numberFormat.fixedPointDecimalPlace;

            SUBRESULT_INT intpart = subresult >> numBitsDecimal;
            SUBRESULT_INT decPart =
                subresult & ((1 << (numBitsDecimal + 1)) - 1);
            // Note: I don't like the dividing here, but due to the
            // lack of a better alternative, this is the way it's done.
            if (numberFormat.numBits == 32) {
                // Convert to float
                float resInt = intpart * 1.0;
                float resDec = decPart * 1.0;
                while (decPart > 1.0) {
                    decPart /= 10.0;
                }
                return (SUBRESULT_INT)(resInt + resDec);
            } else if (numberFormat.numBits == 64) {
                // Convert to double
                double resInt = intpart * 1.0;
                double resDec = decPart * 1.0;
                while (decPart > 1.0) {
                    decPart /= 10.0;
                }
                return (SUBRESULT_INT)(resInt + resDec);
            }
        } else {
            // Already float, nothing to do
            return subresult;
        }
    }
    if (higherOrder == INPUT_FMT_FIXED) {
        if (currentOrder == INPUT_FMT_INT) {
            // Simply convert the integer part.

        } else if (currentOrder == INPUT_FMT_FIXED) {
            // Nothing to do here, simply return
            return subresult;
        } else {
            // Format should not be float at any time.
            // so this is an unhandled error.
        }
    }
    if (higherOrder == INPUT_FMT_INT) {
        // Lowest order, do nothing.
        // Cannot be here if all input is not already int.
        return subresult;
    }
}

/**
 * @brief Process the input arguments for the calculator operation.
 *
 * This function find the highest order input format (int,
 * fixed or float, in that order), and promotes all the
 * entries in the arguments to that format.
 * This is so that calculation can be consistent.
 * @param pArgs Pointer to arguments
 * @param numArgs Number of arguments
 * @param numberFormat Global number format.
 * @return The format that all arguments have been promoted to.
 */
uint8_t processInputArgs(inputType_t *pArgs, uint8_t numArgs,
                         numberFormat_t numberFormat) {
    // First, go through all arguments and find the higest order
    // format.
    uint8_t higestOrderFormat = INPUT_FMT_INT;
    for (uint8_t i = 0; i < numArgs; i++) {
        uint8_t currentFormat = GET_FMT_TYPE(pArgs[i].typeFlag);
        if (currentFormat > higestOrderFormat) {
            higestOrderFormat = currentFormat;
        }
    }

    // Go through and check if any argument has a lower order,
    // in which case promote that to the higher order.
    for (uint8_t i = 0; i < numArgs; i++) {
        uint8_t currentFormat = GET_FMT_TYPE(pArgs[i].typeFlag);
        if (currentFormat < higestOrderFormat) {
            // Promote format to the higer order
            pArgs[i].subresult = promoteOrder(pArgs[i].subresult, currentFormat,
                                              higestOrderFormat, numberFormat);
        }
    }
}

/* ------ CALCULATOR OPERATOR FUNCTIONS ------ */

// Calculator operator functions to be used in "operators" table
int8_t calc_add(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs) {

    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }
    uint8_t inputFormat = processInputArgs(pArgs, num_args, numberFormat);
    // Read out the args as uint32_t. Will be casted later on

    // Make calculation based on format
    if (numberFormat.inputFormat == INPUT_FMT_INT) {
        // Solve for N bit signed integer
        SUBRESULT_INT a = pArgs[0].subresult;
        SUBRESULT_INT b = pArgs[1].subresult;
        (*((SUBRESULT_INT *)pResult)) = a + b;
        // TODO: add overflow detection
    } else if (numberFormat.inputFormat == INPUT_FMT_FLOAT) {
        if (numberFormat.numBits == 32) {
            // Solve for float
            float a, b;
            memcpy(&a, &(pArgs[0].subresult), sizeof(float));
            memcpy(&b, &(pArgs[1].subresult), sizeof(float));
            float result = a + b;
            memcpy(pResult, &result, sizeof(float));
            logger(LOGGER_LEVEL_INFO, "Solved %f + %f to be %f\r\n", a, b,
                   result);
        } else if (numberFormat.numBits == 64) {
            // Solve for double
            double a, b;
            memcpy(&a, &(pArgs[0].subresult), sizeof(double));
            memcpy(&b, &(pArgs[1].subresult), sizeof(double));
            double result = a + b;
            memcpy(pResult, &result, sizeof(double));
            logger(LOGGER_LEVEL_INFO, "Solved %d + %d to be %d\r\n", a, b,
                   result);
        } else {
            // Format is not supporteds
            return format_not_supported;
        }
    } else if (numberFormat.inputFormat == INPUT_FMT_FIXED) {
        // Normal addition should be OK here
        SUBRESULT_INT a = pArgs[0].subresult;
        SUBRESULT_INT b = pArgs[1].subresult;
        (*((SUBRESULT_INT *)pResult)) = a + b;
        // TODO: add overflow detection
    }
    return function_solved;
}

int8_t calc_subtract(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                     int num_args, inputType_t *pArgs) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = pArgs[0].subresult;
    SUBRESULT_INT b = pArgs[1].subresult;
    // Make calculation based on format
    switch (numberFormat.inputFormat) {
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
                     int num_args, inputType_t *pArgs) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as SUBRESULT_INT. Will be casted later on
    SUBRESULT_INT a = pArgs[0].subresult;
    SUBRESULT_INT b = pArgs[1].subresult;
    // Make calculation based on format
    switch (numberFormat.inputFormat) {
    case INPUT_FMT_INT:
        // Solve for 32 bit signed integer
        (*((SUBRESULT_INT *)pResult)) = a * b;
        if (a != 0 && *pResult / a != b) {
            // overflow handling
            logger(LOGGER_LEVEL_ERROR, "MULTIPLICATION OVERFLOW");
            return function_overflow;
        }
        // TODO: return overflow detection
        break;
    case INPUT_FMT_FLOAT:
        if(numberFormat.numBits == 32){
            float f_a, f_b, f_res;
            memcpy(&f_a, &a, sizeof(float));
            memcpy(&f_b, &b, sizeof(float));
            f_res = f_a*f_b;
            memcpy(pResult, &f_res, sizeof(float));
            if (f_a != 0 && f_res / f_a != f_b) {
                // overflow handling
                logger(LOGGER_LEVEL_ERROR, "MULTIPLICATION OVERFLOW");
                return function_overflow;  
            }
        } else if(numberFormat.numBits == 64){
            double f_a, f_b, f_res;
            memcpy(&f_a, &a, sizeof(double));
            memcpy(&f_b, &b, sizeof(double));
            f_res = f_a*f_b;
            memcpy(pResult, &f_res, sizeof(double));
            if (f_a != 0 && f_res / f_a != f_b) {
                // overflow handling
                logger(LOGGER_LEVEL_ERROR, "MULTIPLICATION OVERFLOW");
                return function_overflow;
            }
        } else {
            logger(LOGGER_LEVEL_ERROR, "FLOAT only supports 32 or 64 bits!\r\n");
            return format_not_supported;
        }
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
                   int num_args, inputType_t *pArgs) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = pArgs[0].subresult;
    SUBRESULT_INT b = pArgs[1].subresult;
    if (b == 0) {
        return error_args;
    }
    // Make calculation based on format
    switch (numberFormat.inputFormat) {
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
                int num_args, inputType_t *pArgs) {
    // Only expecting two variable arguments here
    if (num_args != 2) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = pArgs[0].subresult;
    SUBRESULT_INT b = pArgs[1].subresult;
    if (b == 0) {
        return error_args;
    }
    switch (numberFormat.inputFormat) {
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
                 int num_args, inputType_t *pArgs) {
    return function_solved;
}

int8_t calc_or(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
               int num_args, inputType_t *pArgs) {
    return function_solved;
}

int8_t calc_xor(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs) {
    return function_solved;
}

int8_t calc_not(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs) {
    // Only expecting one variable arguments here
    if (num_args != 1) {
        return incorrect_args;
    }

    // Read out the args as uint32_t. Will be casted later on

    SUBRESULT_INT a = pArgs[0].subresult;
    // Make calculation based on format
    switch (numberFormat.inputFormat) {
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
                      int num_args, inputType_t *pArgs) {
    return function_solved;
}

int8_t calc_rightshift(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                       int num_args, inputType_t *pArgs) {
    return function_solved;
}

int8_t calc_sum(SUBRESULT_INT *pResult, numberFormat_t numberFormat,
                int num_args, inputType_t *pArgs) {

    if (pArgs == NULL) {
        return incorrect_args;
    }
    if (num_args < 1) {
        return incorrect_args;
    }
    // Make calculation based on format
    switch (numberFormat.inputFormat) {
    case INPUT_FMT_INT:
        // Solve for N bit signed integer
        (*((SUBRESULT_INT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_INT *)pResult)) += (SUBRESULT_INT)pArgs[i].subresult;
        }
        // TODO: add overflow detection
        break;
    case INPUT_FMT_FLOAT:
        // Solve for 32bit floats
        (*((SUBRESULT_INT *)pResult)) = 0;
        for (int i = 0; i < num_args; i++) {
            (*((SUBRESULT_INT *)pResult)) += (SUBRESULT_INT)pArgs[i].subresult;
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
