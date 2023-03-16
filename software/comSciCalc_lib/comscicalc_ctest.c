/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Computer Scientist Calculator (comscicalc) C library
 * 
 * This file contains functions for testing the comscicalc library 
 * in c. 
 * This should at some point be ported to Python. 
 *
 * 
 */

/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */

// comsci header file
#include "comscicalc.h"
#include "comscicalc_operators.h"
// Standard libs
#include <stdio.h>
#include <string.h>


/* -------------- TEST FUNCTION--------------- */


/* -------------- MAIN FUNCTION--------------- */


void main(){

	printf("Testing the comsci library funcitons\r\n");
	printf("\r\n");

	bool passed = true;

	/* -------------- Calculator core tests -------------- 	*/
	calcCoreState_t coreState;
	if(calc_coreInit(&coreState) != calc_funStatus_SUCCESS){
		printf("ERROR: Could not intialize calculator core state!\r\n");
		// If this is the case, we cannot proceed. 
		return;
	}
	coreState.inputBase = inputBase_DEC;

	// String to be tested
	char pTestString[] = "123+456\0";

	char *pTestStringIter = pTestString;
	uint8_t status = calc_funStatus_SUCCESS;

	while(*pTestStringIter != '\0'){
		status = calc_addInput(&coreState, *pTestStringIter);
		if(status != calc_funStatus_SUCCESS){
			printf("Could not add input \r\n");
			printf("Status = %d\n", status);
		}
		pTestStringIter++;
	}

	// Print results:
	char pResString[100] = {0};
	uint8_t state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  %s\r\n", pTestString);
	
	// Teardown buffers:
	if(calc_coreBufferTeardown(&coreState) != calc_funStatus_SUCCESS){
		printf("Buffer teardown unsuccessful! \r\n");
	}
	return; 

}