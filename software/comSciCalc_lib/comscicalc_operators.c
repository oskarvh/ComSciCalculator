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
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* ------------- GLOBAL VARIABLES ------------ */
// List of operator function pointers
// Ensure that each inputChar and op field is unique! 
const operatorEntry_t operators[NUM_OPERATORS] = {
	// Arithmetic operators, multiple input
	{.inputChar = '+', .opString = "+\0",  .solvPrio = 3, .bIncDepth = false, .pFun = &calc_add},
	{.inputChar = '-', .opString = "-\0",  .solvPrio = 3, .bIncDepth = false, .pFun = &calc_subtract},
	{.inputChar = '*', .opString = "*\0",  .solvPrio = 0, .bIncDepth = false, .pFun = &calc_multiply},
	{.inputChar = '/', .opString = "/\0",  .solvPrio = 1, .bIncDepth = false, .pFun = &calc_divide},
	{.inputChar = '<', .opString = "<<\0", .solvPrio = 2, .bIncDepth = false, .pFun = &calc_leftshift},
	{.inputChar = '>', .opString = ">>\0", .solvPrio = 2, .bIncDepth = false, .pFun = &calc_rightshift},
	{.inputChar = 0,   .opString = "\0",   .solvPrio = 255, .bIncDepth = false, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0",   .solvPrio = 255, .bIncDepth = false, .pFun = NULL},
	
	// Bitwise operators, mulitple input
	{.inputChar = '&', .opString = "AND\0",  .solvPrio = 0, .bIncDepth = false, .pFun = &calc_and},
	{.inputChar = 'n', .opString = "NAND\0", .solvPrio = 0, .bIncDepth = false, .pFun = &calc_nand},
	{.inputChar = '|', .opString = "OR\0",   .solvPrio = 0, .bIncDepth = false, .pFun = &calc_or},
	{.inputChar = '^', .opString = "XOR\0",  .solvPrio = 0, .bIncDepth = false, .pFun = &calc_xor},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	
	// Arithmetic operators, single input
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	
	// Bitwise operators, single input
	{.inputChar = '~', .opString = "NOT\0", .solvPrio = 0, .bIncDepth = true, .pFun = &calc_not},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .solvPrio = 255, .bIncDepth = true, .pFun = NULL}

};

int test(uint8_t num_args, ...){
	va_list valist;
	va_start(valist, num_args);

	// Only expecting two variable arguments here
	printf("input format: %i, Num args: %i\r\n");
	if(num_args != 2){
		return incorrect_args;
	}
	
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	uint32_t b = va_arg(valist, uint32_t);
	printf("Adding %i and %i\r\n");
	va_end(valist);
}

/* ------ CALCULATOR OPERATOR FUNCTIONS ------ */

int8_t calc_add(uint32_t *pResult, inputFormat_t inputFormat, int num_args, ...){
	va_list valist;
	va_start(valist, num_args);

	// Only expecting two variable arguments here
	if(num_args != 2){
		return incorrect_args;
	}
	
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	uint32_t b = va_arg(valist, uint32_t);
	va_end(valist);
	// Make calculation based on format
	switch(inputFormat){
		case INPUT_FMT_UINT:
			// Solve for N bit unsigned integer
			(*((SUBRESULT_UINT*)pResult)) = a+b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_SINT:
			// Solve for N bit signed integer
			(*((SUBRESULT_INT*)pResult)) = a+b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FLOAT:
			// Solve for 32bit floats
			(*((SUBRESULT_FLOAT*)pResult)) = a+b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FIXED:
			// TODO:Solve for fixed point. 
			// TODO: add overflow detection
		break;
	}
	return function_solved;
}

int8_t calc_subtract(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	va_list valist;
	va_start(valist, num_args);

	// Only expecting two variable arguments here
	if(num_args != 2){
		return incorrect_args;
	}
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	uint32_t b = va_arg(valist, uint32_t);
	va_end(valist);
	// Make calculation based on format
	switch(inputFormat){
		case INPUT_FMT_UINT:
			// Solve for N bit unsigned integer
			(*((SUBRESULT_UINT*)pResult)) = a-b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_SINT:
			// Solve for N bit signed integer
			(*((SUBRESULT_INT*)pResult)) = a-b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FLOAT:
			// Solve for 32bit float
			(*((SUBRESULT_FLOAT*)pResult)) = a-b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FIXED:
			// TODO:Solve for fixed point. 
			// TODO: add overflow detection
		break;
	}
	return function_solved;
}

int8_t calc_multiply(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	va_list valist;
	va_start(valist, num_args);

	// Only expecting two variable arguments here
	if(num_args != 2){
		return incorrect_args;
	}
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	uint32_t b = va_arg(valist, uint32_t);
	va_end(valist);
	// Make calculation based on format
	switch(inputFormat){
		case INPUT_FMT_UINT:
			// Solve for 32bit unsigned integer
			(*((SUBRESULT_UINT*)pResult)) = a*b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_SINT:
			// Solve for 32 bit signed integer
			(*((SUBRESULT_INT*)pResult)) = a*b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FLOAT:
			// Solve for 32bit float
			(*((SUBRESULT_FLOAT*)pResult)) = a*b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FIXED:
			// TODO:Solve for fixed point. 
			// TODO: add overflow detection
		break;
	}
	return function_solved;
}

int8_t calc_divide(void *pResult, inputFormat_t inputFormat, int num_args, ...){
		va_list valist;
	va_start(valist, num_args);

	// Only expecting two variable arguments here
	if(num_args != 2){
		return incorrect_args;
	}
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	uint32_t b = va_arg(valist, uint32_t);
	va_end(valist);
	// Make calculation based on format
	switch(inputFormat){
		case INPUT_FMT_UINT:
			// Solve for 32bit unsigned integer
			(*((SUBRESULT_UINT*)pResult)) = a/b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_SINT:
			// Solve for 32 bit signed integer
			(*((SUBRESULT_INT*)pResult)) = a/b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FLOAT:
			// Solve for 32bit float
			(*((SUBRESULT_FLOAT*)pResult)) = a/b;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FIXED:
			// TODO:Solve for fixed point. 
			// TODO: add overflow detection
		break;
	}
	return function_solved;
}

int8_t calc_and(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

int8_t calc_nand(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

int8_t calc_or(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

int8_t calc_xor(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

int8_t calc_not(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	va_list valist;
	va_start(valist, num_args);

	// Only expecting one variable arguments here
	if(num_args != 1){
		return incorrect_args;
	}
	// Read out the args as uint32_t. Will be casted later on
	uint32_t a = va_arg(valist, uint32_t);
	va_end(valist);
	// Make calculation based on format
	switch(inputFormat){
		case INPUT_FMT_UINT:
		case INPUT_FMT_SINT:
			// Solve for 32bit unsigned or signed integer
			(*((SUBRESULT_UINT*)pResult)) = ~a;
			// TODO: add overflow detection
		break;
		case INPUT_FMT_FLOAT:
			// Does not make sense for float. But I'll allow it. 
			(*((SUBRESULT_FLOAT*)pResult)) = ~a;
		break;
		case INPUT_FMT_FIXED:
			// TODO:Solve for fixed point. 
			// TODO: add overflow detection
		break;
	}
	return function_solved;
}

int8_t calc_leftshift(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

int8_t calc_rightshift(void *pResult, inputFormat_t inputFormat, int num_args, ...){
	return function_solved;
}

/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/