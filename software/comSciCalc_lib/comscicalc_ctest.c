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

	// Add '123'
	uint8_t status = calc_addInput(&coreState, '1');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '2');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '3');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}
	

	// Go back one step and add a 4
	coreState.cursorPosition = 1;
	status = calc_addInput(&coreState, '4');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Go back to the beginning step and add a 5
	// THIS DOES NOT WORK! HOW WOULD THIS BE HANDLED?
	coreState.cursorPosition = 4;
	status = calc_addInput(&coreState, '5');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Go back to the beginning step and add a 5
	// THIS DOES NOT WORK! HOW WOULD THIS BE HANDLED?
	coreState.cursorPosition = 40;
	status = calc_addInput(&coreState, '6');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Print results:
	char pResString[100] = {0};
	uint8_t state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  651243 \r\n");

	
	if(calc_coreInit(&coreState) != calc_funStatus_SUCCESS){
		printf("ERROR: Could not intialize calculator core state!\r\n");
		// If this is the case, we cannot proceed. 
		return;
	}
	coreState.inputBase = inputBase_DEC;

	// Test operator insertion
	// Add '1234'
	status = calc_addInput(&coreState, '1');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '2');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '3');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '4');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Add plus at the end -1
	coreState.cursorPosition = 1;
	status = calc_addInput(&coreState, '+');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Add a 5. 
	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '5');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 

	memset(pResString, 0, 100);
	state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  123+45 \r\n");

	// Teardown buffers:
	if(calc_coreBufferTeardown(&coreState) != calc_funStatus_SUCCESS){
		printf("Buffer teardown unsuccessful! \r\n");
	}

	if(calc_coreInit(&coreState) != calc_funStatus_SUCCESS){
		printf("ERROR: Could not intialize calculator core state!\r\n");
		// If this is the case, we cannot proceed. 
		return;
	}
	coreState.inputBase = inputBase_DEC;

	// Test operator insertion
	status = calc_addInput(&coreState, '1');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '2');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Add plus at the start
	coreState.cursorPosition = 2;
	status = calc_addInput(&coreState, '+');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}
	// Add plus at the start
	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '+');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '3');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 


	memset(pResString, 0, 100);
	state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  +12+3 \r\n");

	// Teardown buffers:
	if(calc_coreBufferTeardown(&coreState) != calc_funStatus_SUCCESS){
		printf("Buffer teardown unsuccessful! \r\n");
	}

	if(calc_coreInit(&coreState) != calc_funStatus_SUCCESS){
		printf("ERROR: Could not intialize calculator core state!\r\n");
		// If this is the case, we cannot proceed. 
		return;
	}
	coreState.inputBase = inputBase_DEC;

	coreState.cursorPosition = 0;
	// Test operator insertion
	status = calc_addInput(&coreState, '1');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '2');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Add plus at the start
	coreState.cursorPosition = 1;
	status = calc_addInput(&coreState, 'N');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}
	// Add plus at the start
	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '+');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	status = calc_addInput(&coreState, '3');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 


	memset(pResString, 0, 100);
	state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  1NAND(2+3 \r\n");

	// Teardown buffers:
	if(calc_coreBufferTeardown(&coreState) != calc_funStatus_SUCCESS){
		printf("Buffer teardown unsuccessful! \r\n");
	}

	if(calc_coreInit(&coreState) != calc_funStatus_SUCCESS){
		printf("ERROR: Could not intialize calculator core state!\r\n");
		// If this is the case, we cannot proceed. 
		return;
	}
	coreState.inputBase = inputBase_DEC;

	coreState.cursorPosition = 0;
	// Test operator insertion
	status = calc_addInput(&coreState, '+');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '*');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}



	// Add plus at the start
	coreState.cursorPosition = 1;
	// Problem is: In this case there are three entries, as a new operator always
	// spawns a new empty entry when added last. 
	status = calc_addInput(&coreState, '-');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}
	
	// Add plus at the start
	coreState.cursorPosition = 3;
	status = calc_addInput(&coreState, '-');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}

	coreState.cursorPosition = 0;
	status = calc_addInput(&coreState, '/');
	if(status != calc_funStatus_SUCCESS){
		printf("Could not add input \r\n");
		printf("Status = %d\n", status);
	}
	
	// -+-*/
	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 


	

	memset(pResString, 0, 100);
	state = calc_printBuffer(&coreState, pResString, 100);
	printf("In buffer: %s\n", pResString);
	printf("Expected:  -+-*/ \r\n\r\n");

	// Teardown buffers:
	if(calc_coreBufferTeardown(&coreState) != calc_funStatus_SUCCESS){
		printf("Buffer teardown unsuccessful! \r\n");
	}
}