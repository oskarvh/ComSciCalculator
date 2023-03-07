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
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* -------------- TEST FUNCTION--------------- */
bool test1_getInputListEntry(){
	bool passed = true;
	/*
	1. Input an empty list (no entries at all), cursor value 0. 
	  - Returns SUCCESS with string null pointer and list pointer at pInputList
	*/
	// Create an empty list
	inputListEntry_t emptyList = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	uint8_t cursorPosition = 0;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_SUCCESS){
		passed = false;
		printf("getInputListEntry 1 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList){
		passed = false;
		printf("getInputListEntry 1 returned a pointer for pInputListAtCursor.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 1 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test2_getInputListEntry(){
	bool passed = true;
	/*
	2. Input an empty list (no entries at all), cursor value > 0.
	   - Returns CURSOR_VALUE_LARGER_THAN_LIST_ENTRY with string null 
	     pointer and list pointer at pInputList
	*/
	// Create an empty list
	inputListEntry_t emptyList = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY){
		passed = false;
		printf("getInputListEntry 2 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList){
		passed = false;
		printf("getInputListEntry 2 returned a pointer for pInputListAtCursor.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 2 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test3_getInputListEntry(){
	bool passed = true;
	/*	     
	3. Input a NULL pointer as input list
	  - Returns with INPUT_LIST_NULL. 
	*/
	// Create an empty list
	inputListEntry_t *emptyList = NULL;
	uint8_t cursorPosition = 0;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		emptyList, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_INPUT_LIST_NULL){
		passed = false;
		printf("getInputListEntry 3 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != emptyList){
		passed = false;
		printf("getInputListEntry 3 returned a pointer for pInputListAtCursor.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 3 returned a pointer for pInputString.\r\n");
	}
	return passed;
}


bool test4_getInputListEntry(){
	bool passed = true;
	/*	  
	4. Input a list with empty string but preceding operator, cursor value 0. 
	  - Example "NOT(", cursor = 0
	  - Returns SUCCESS with string null pointer and list pointer at pInputList
	*/
	// Create an empty list
	inputListEntry_t emptyList = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	uint8_t cursorPosition = 0;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_SUCCESS){
		passed = false;
		printf("getInputListEntry 4 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList){
		passed = false;
		printf("getInputListEntry 4 returned a pointer for pInputListAtCursor.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 4 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test5a_getInputListEntry(){
	bool passed = true;
	/*	  
	5. Input a list with empty string but preceding operator, cursor value > 0. 
	  - Example "NOT(", cursor = 1
	  - Returns LIST_AT_PRECEDING_OPERATOR_ENTRY, string entry NULL and list pointer at pInputList
	*/
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE,
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 5a did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList1){
		passed = false;
		printf("getInputListEntry 5a did not return the correct pointer.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 5a returned a pointer for pInputString.\r\n");
	}
	return passed;
	return false;
}

bool test5b_getInputListEntry(){
	bool passed = true;
	/*	  
	5. Input a list with empty string but preceding operator, cursor value > 0. 
	  - Example "123+NOT(", cursor = 1
	  - Returns LIST_AT_PRECEDING_OPERATOR_ENTRY, string entry NULL and list pointer at pInputList
	*/
	// Create the string list
	inputStringEntry_t stringEntry1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.c = '1'
	};
	inputStringEntry_t stringEntry2 = {
		.pPrevious = &stringEntry1, 
		.pNext = NULL, 
		.c = '2'
	};
	inputStringEntry_t stringEntry3 = {
		.pPrevious = &stringEntry3, 
		.pNext = NULL, 
		.c = '3'
	};
	stringEntry1.pNext = &stringEntry2;
	stringEntry2.pNext = &stringEntry3;
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = &stringEntry1, 
		.pLastInputStringEntry = &stringEntry3, 
		.opNext = operators_ADD,
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};

	inputListEntry_t emptyList2 = {
		.pPrevious = &emptyList1, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE,
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	emptyList1.pNext = &emptyList2;
	
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 5b did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList2){
		passed = false;
		printf("getInputListEntry 5b did not return the correct pointer.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 5b returned a pointer for pInputString.\r\n");
	}
	return passed;
	return false;
}


bool test6_getInputListEntry(){
	bool passed = true;
	/*
	6. Input a list with an operator but no string list, cursor 0
	  - Example "+", cursor 0
	  - Returns SUCCESS with string null pointer and list pointer at pInputList
	*/
	// Create an empty list
	inputListEntry_t emptyList = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_ADD, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	uint8_t cursorPosition = 0;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_SUCCESS){
		passed = false;
		printf("getInputListEntry 6 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList){
		passed = false;
		printf("getInputListEntry 6 returned a pointer for pInputListAtCursor.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 6 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test7_getInputListEntry(){
	bool passed = true;
	/*	  
	7. Input a list with an operator but no string list, cursor > 0
	  - Example "+", cursor 1
	  - Returns LIST_AT_OPERATOR_ENTRY with string null pointer and list pointer at pInputList->pPrevious
	*/
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_ADD, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	inputListEntry_t emptyList2 = {
		.pPrevious = &emptyList1, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	emptyList1.pNext = &emptyList2;
	
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 7 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList1){
		passed = false;
		printf("getInputListEntry 7 did not return the correct pointer.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 7 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test8_getInputListEntry(){
	bool passed = true;
	/*	  
	8. Input a list of operators and check the operator
	  - Example "+-*", cursor 1
	  - Returns LIST_AT_OPERATOR_ENTRY with string null pointer, and list pointer operator
	    pInputList->opNext = MINUS
	*/
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_ADD, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	inputListEntry_t emptyList2 = {
		.pPrevious = &emptyList1, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_SUBTRACT, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	inputListEntry_t emptyList3 = {
		.pPrevious = &emptyList2, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_MULTI, 
		.opThis = operators_NONE, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	emptyList1.pNext = &emptyList2;
	emptyList2.pNext = &emptyList3;
	
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 8 did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList2){
		passed = false;
		printf("getInputListEntry 8 did not return the correct pointer.\r\n");
	}

	if(pInputListAtCursor->opNext != operators_SUBTRACT){
		passed = false;
		printf("getInputListEntry 8 did not return correct operator.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 8 returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test9a_getInputListEntry(){
	bool passed = true;
	/*	  
	9a. Double preceding operators
	  - Example "NOT(NOT(", cursor 1
	  - Returns LIST_AT_PRECEDING_OPERATOR_ENTRY with string null pointer, and list pointer 
	  	at the second entry
	*/
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	inputListEntry_t emptyList2 = {
		.pPrevious = &emptyList1, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	emptyList1.pNext = &emptyList2;
	
	uint8_t cursorPosition = 1;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 9a did not return successfully.\r\n");
	}

	if(pInputListAtCursor != &emptyList2){
		passed = false;
		printf("getInputListEntry 9a did not return the correct pointer.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 9a returned a pointer for pInputString.\r\n");
	}
	return passed;
}

bool test9b_getInputListEntry(){
	bool passed = true;
	/*	  
	9b. Double preceding operators, start of list
	  - Example "NOT(NOT(", cursor 2
	  - Returns LIST_AT_PRECEDING_OPERATOR_ENTRY with string null pointer, and list pointer 
	  	at first entry
	*/
	// Create an empty list
	inputListEntry_t emptyList1 = {
		.pPrevious = NULL, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	inputListEntry_t emptyList2 = {
		.pPrevious = &emptyList1, 
		.pNext = NULL, 
		.inputBase = inputBase_NONE, 
		.pInputStringEntry = NULL, 
		.pLastInputStringEntry = NULL, 
		.opNext = operators_NONE, 
		.opThis = operators_NOT, 
		.customFunction.pFunc = NULL, 
		.customFunction.numArgs = 0,
		.depth = 0
	};
	emptyList1.pNext = &emptyList2;
	
	uint8_t cursorPosition = 2;
	inputListEntry_t *pInputListAtCursor = NULL;
	inputStringEntry_t *pInputString = NULL;

	// Run the function
	inputModStatus_t testRes = getInputListEntry(
		&emptyList1, 
		cursorPosition,
		&pInputListAtCursor, 
		&pInputString);

	// Check the output:
	if(testRes != inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY){
		passed = false;
		printf("getInputListEntry 9b did not return successfully.\r\n");
		printf("Status = %i\n", testRes);
	}

	if(pInputListAtCursor != &emptyList1){
		passed = false;
		printf("getInputListEntry 9b did not return the correct pointer.\r\n");
	}

	if(pInputString != NULL){
		passed = false;
		printf("getInputListEntry 9b returned a pointer for pInputString.\r\n");
	}
	return passed;
}

/* -------------- MAIN FUNCTION--------------- */


void main(){
	printf("Testing the comsci library funcitons\r\n");
	printf("\r\n");

	bool passed = true;

	/* getInputListEntry test cases: */
	printf("Testing function getInputListEntry.\r\n");
	
	passed = test1_getInputListEntry();
	
	passed = test2_getInputListEntry();

	passed = test3_getInputListEntry();

	passed = test4_getInputListEntry();

	passed = test5a_getInputListEntry();

	passed = test5b_getInputListEntry();

	passed = test6_getInputListEntry();

	passed = test7_getInputListEntry(); 
	
	passed = test8_getInputListEntry();
	
	passed = test9a_getInputListEntry();

	passed = test9b_getInputListEntry();	
	
	if(passed){
		printf("getInputListEntry tests passed!\r\n");
	}
	else {
		printf("getInputListEntry tests failed!\r\n");
	}
}