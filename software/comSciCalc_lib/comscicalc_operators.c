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

int32_t calc_not(int32_t a){
	return ~a;
}

int32_t calc_leftshift(int32_t a, int32_t b){
	return a<<b;
}

int32_t calc_rightshift(int32_t a, int32_t b){
	return a>>b;
}

/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/