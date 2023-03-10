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

// Standard libs
#include <stdio.h>


/* -------------- TEST FUNCTION--------------- */


/* -------------- MAIN FUNCTION--------------- */


void main(){
	printf("Testing the comsci library funcitons\r\n");
	printf("\r\n");

	bool passed = true;

	//return; 
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

	// Print results:
	printf("In buffer: ");
	inputListEntry_t *pListEntry = coreState.pListEntrypoint;
	while(pListEntry != NULL){
		inputStringEntry_t *pInputStringEntry = (inputStringEntry_t *)pListEntry->pInputStringEntry;
		while(pInputStringEntry != NULL){
			printf("%c", pInputStringEntry->c);
			pInputStringEntry = pInputStringEntry->pNext;
		}
		pListEntry = pListEntry->pNext;
		printf("\r\n");
		
	}


	// Teardown buffers:
	calc_coreBufferTeardown(&coreState);


}