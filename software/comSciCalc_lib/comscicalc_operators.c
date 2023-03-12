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
/* ------------- GLOBAL VARIABLES ------------ */
// List of operator function pointers
// Ensure that each inputChar and op field is unique! 
const operatorEntry_t operators[NUM_OPERATORS] = {
	// Arithmetic operators, multiple input
	{.inputChar = '+', .opString = "+\0", .op = operators_ADD,      .bIncDepth = false, .pFun = &calc_add},
	{.inputChar = '-', .opString = "-\0", .op = operators_SUBTRACT, .bIncDepth = false, .pFun = &calc_subtract},
	{.inputChar = '*', .opString = "*\0", .op = operators_MULTI,    .bIncDepth = false, .pFun = &calc_multiply},
	{.inputChar = '/', .opString = "/\0", .op = operators_DIVIDE,   .bIncDepth = false, .pFun = &calc_divide},
	{.inputChar = 0,   .opString = "\0",  .op = operators_NONE, .bIncDepth = false, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0",  .op = operators_NONE, .bIncDepth = false, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0",  .op = operators_NONE, .bIncDepth = false, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0",  .op = operators_NONE, .bIncDepth = false, .pFun = NULL},
	
	// Bitwise operators, mulitple input
	{.inputChar = '&', .opString = "AND\0",  .op = operators_AND,  .bIncDepth = true, .pFun = &calc_and},
	{.inputChar = 'N', .opString = "NAND\0", .op = operators_NAND, .bIncDepth = true, .pFun = &calc_nand},
	{.inputChar = '|', .opString = "OR\0",   .op = operators_OR,   .bIncDepth = true, .pFun = &calc_or},
	{.inputChar = '^', .opString = "XOR\0",  .op = operators_XOR,  .bIncDepth = true, .pFun = &calc_xor},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	
	// Arithmetic operators, single input
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	
	// Bitwise operators, single input
	{.inputChar = '~', .opString = "NOT\0", .op = operators_NOT, .bIncDepth = true, .pFun = &calc_not},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL},
	{.inputChar = 0,   .opString = "\0", .op = operators_NONE, .bIncDepth = true, .pFun = NULL}

};


/* ------ CALCULATOR OPERATOR FUNCTIONS ------ */

// Note: These can be overloaded, but the return is always 32 bits, 
// with two 32 bit inputs. This might come to change in the future if
// any major roadblocks become apparent. 
int32_t calc_add(int32_t a, int32_t b){
	return a+b;
}

int32_t calc_subtract(int32_t a, int32_t b){
	return a-b;
}

int32_t calc_multiply(int32_t a, int32_t b){
	return a*b;
}

int32_t calc_divide(int32_t a, int32_t b){
	return 0; //TODO: 
}

int32_t calc_and(int32_t a, int32_t b){
	return a&b;
}

int32_t calc_nand(int32_t a, int32_t b){
	return ~(a&b);
}

int32_t calc_or(int32_t a, int32_t b){
	return a|b;
}

int32_t calc_xor(int32_t a, int32_t b){
	return a^b;
}

int32_t calc_not(int32_t a, int32_t b){
	return ~a;
}

/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/