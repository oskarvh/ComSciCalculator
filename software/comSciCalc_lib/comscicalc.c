/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Computer Scientist Calculator (comscicalc) C library
 * 
 * This file contains functions to run the computer scientist calculator
 * The main kernel takes a character input, and based on the input settings
 * calculates the result dynamically. 
 * 
 * Input options:
 * - Decimal, hexadecimal or binary input mode (inputBase_t inputBase)
 * - Signed or unsigned entry (bool inputSigned)
 * - IEEE 754 float input mode (bool inputFloat)
 * 
 * Output options: 
 * - All bases (hex, dec and bin) is always available
 * - Signed or unsigned (bool outputSigned)
 * - IEEE 754 float (bool outputFloat)
 *
 *
 * ------------------- DEV LOGS -------------------
 * March 6, 2023
 * For preceding operators, such as NOT, have a single operand, 
 * but can still have a long list of internal calculations. 
 * This is kind of the same as brackets/parenthesis. 
 * There are situations where these are stacked, e.g.
 * NOT(NOT(...)) or ((12+4)*2). 
 * These situations require a lot of extra edge case handling
 * if handled in a single list entry. 
 * After protoyping in Python, I found that if the brackets
 * spawn a new list entry with an increase in depth of 1 per new entry, 
 * it becomes a lot easier to work with. 
 * Since there are a lot of commonalities with preceding/single entry operators, 
 * I think it makes sense to do the same. However, there is a point in keeping the
 * opThis (i.e. preceding operator) for printing purposes. 
 */

/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */
// comsci header file
#include "comscicalc.h"

/* -------- CALCULATOR CORE FUNCTIONS -------- */



/* Function to find the input list entry and input character entry
   Args: 
   - pInputList: Pointer to first entry of the input list
   - cursorPosition: Position of the cursor from the end.
   - ppInputListAtCursor: Pointer to pointer to inputList entry
   - ppInputString: Pointer to pointer to string entry
   Returns:
   - state of the function
*/
// TODO: Should be static. But for tests, keep it non-static
inputModStatus_t getInputListEntry(
		inputListEntry_t *pInputList, 
		uint8_t cursorPosition,
		inputListEntry_t **ppInputListAtCursor, 
		inputStringEntry_t **ppInputString)
{
	// Check pointer to input list
	if(pInputList == NULL){
		return inputModStatus_INPUT_LIST_NULL;
	}

	// Based on the first entry, get the last entry of the list. 
	// Should be reasonably fast, since the number of list entries shouldn't
	// be insane for most cases. 
	inputListEntry_t *pListEntry = pInputList;
	while(pListEntry->pNext != NULL){
		pListEntry = pListEntry->pNext;
	}

	// Going backwards, find the entry of the string. 
	inputStringEntry_t *pStringEntry = pListEntry->pLastInputStringEntry;
	// If the cursor position is 0, we should return with success by default. 
	inputModStatus_t state = inputModStatus_SUCCESS;
	for(uint8_t i = 0 ; i < cursorPosition ; i++){
		// If the string entry is NULL, that means that the current
		// string entry is exhausted, and we need to go to the previous
		// input list entry. 
		// However, the previous entry cannot be a string entry, 
		// it has to be an operator entry, either the previous nextOp
		// or this entries preceding operator (e.g. NOT). 
		if(pStringEntry == NULL){
			// Move on to previous entry, with the cursor at the  
			// previous list entries operator, or at this operator
			if(pListEntry->opThis != operators_NONE){
				// If the previous state was a preceding operator entry, and
				// the current one has a preceeding operator, that means that 
				// there were two preceding operators in a row. 
				if(state == inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY){
					state = inputModStatus_LIST_AT_OPERATOR_ENTRY;
					pListEntry = pListEntry->pPrevious;
					pStringEntry = pListEntry->pLastInputStringEntry;
				}
				else {
					// If there is no previous list entry, and the previous operator
					// was not a preceding operator, then this list entry has an empty
					// string buffer with only one operator. 
					state = inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY;
					// We need to stay at the current list entry, but acknowledge that a previous
					// list entry has been observed. 
				}
			}
			else {
				// Go to the previous list entry, if it exists. 
				if(pListEntry->pPrevious != NULL){
					// Cursor at previous operator, 				
					pListEntry = pListEntry->pPrevious;
					// Check if there is an operator, 
					// If yes, then set state to be at operator entry, 
					// Otherwise the previous entry didn't have an operator, 
					// which is the case of e.g. parenthesis
					if(pListEntry->opNext != operators_NONE){
						state = inputModStatus_LIST_AT_OPERATOR_ENTRY;
					}
					else {
						state = inputModStatus_SUCCESS;
					}
					pStringEntry = pListEntry->pLastInputStringEntry;
				}
				else {
					// If there is no previous list entry, hence we've
					// reaced the start of the list. 
					// Break and return with the state and results
					state = inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY;
					break;
				}
			}
		}
		else {
			// There is a string input, move to the previous one.
			pStringEntry = pStringEntry->pPrevious;
			// Currently at a list entry. 
			state = inputModStatus_SUCCESS;
		}
	}

	// Set the return values and return the state. 
	*ppInputListAtCursor = pListEntry;
	*ppInputString = pStringEntry;

	return state;
}

/* Function to add a character to the list
   Args: 
   - pInputList: Pointer to first entry of the input list
   - pCalcCoreState: pointer to calculator core state
   Returns:
   - state of the function

   Function test cases:
   1. pInputList = NULL
      - Returns: INPUT_LIST_NULL
   2. pCalcCoreState = NULL
      - Returns: CALC_CORE_STATE_NULL
   3. Add character to end
   4. Add operator at end
   5. Add character at cursor > 0
   6. Add operator at cursor > 0

   n. test1
      - Example
      - Returns 
*/
calc_funStatus_t calc_addInput(inputListEntry_t *pInputList, calcCoreState_t* pCalcCoreState){

	// Check pointer to input list
	if(pInputList == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}

	// Get the current list and string entries based on the cursor. 
	inputListEntry_t *pCurrentListEntry;
	inputStringEntry_t *pCurrentStringEntry;
	inputModStatus_t listState = getInputListEntry(
		pInputList, 
		pCalcCoreState->cursorPosition,
		&pCurrentListEntry, 
		&pCurrentStringEntry
	);
	switch(listState){
		case inputModStatus_SUCCESS:
			// Everything went well, we can add input into list, or add another operator
			// If adding a char, insert that into the string buffer
			// If an operator, then create a new list entry splitting the list. 
			// If a preceding operator, then create a new list entry for that. 
			break;
		case inputModStatus_LIST_AT_OPERATOR_ENTRY:
			// Everything went well, we can add input into list
			// Char shall be added at the end of the string buffefr
			// Operator shall replace the current operator, and the current
			// operator shall be inserted in a new list entry. 
			break;
		case inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY:
			// Everything went well, list at preceding operator
			// If the cursor is at a preceding operator, any inputs shall
			// be added to the previous list entry. 
			break; 
		case inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY:
			// This is OK, simply add to the start, but cursor might need to be updated.
			// Add input to start of list. Need to check if there is a preceding operator. 
			break; 
		default:
			// Unknown return, throw an error. 
			break;
	}

	// Return success
	return calc_funStatus_SUCCESS;
}

/* Function to remove a character to the list. Only backspace possible. 
   Args: 
   - pInputList: Pointer to first entry of the input list
   - pCalcCoreState: pointer to calculator core state
   Returns:
   - state of the function

   Function test cases:
   1. pInputList = NULL
      - Returns: INPUT_LIST_NULL
   2. pCalcCoreState = NULL
      - Returns: CALC_CORE_STATE_NULL

   n. test1
      - Example
      - Returns 
*/
calc_funStatus_t calc_removeInput(inputListEntry_t *pInputList, calcCoreState_t* pCalcCoreState){
	// Check pointer to input list
	if(pInputList == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}

	// Get the current list and string entries based on the cursor. 
	inputListEntry_t *pCurrentListEntry;
	inputStringEntry_t *pCurrentStringEntry;
	inputModStatus_t listState = getInputListEntry(
		pInputList, 
		pCalcCoreState->cursorPosition,
		&pCurrentListEntry, 
		&pCurrentStringEntry
	);
	switch(listState){
		case inputModStatus_SUCCESS:
			// Everything went well, remove string if not start of string buffer. 
			// If at the start of the string, either remove preceding operator or previous
			// operator. 
			break;
		case inputModStatus_LIST_AT_OPERATOR_ENTRY:
			// Remove last string input of previous entry, or if string input empty remove
			// previous operator. If preceding operator exists without end operator, then remove
			// preceding operator. 
			break;
		case inputModStatus_LIST_AT_PRECEDING_OPERATOR_ENTRY:
			// At a preceding operator. Pretty much the same procedure as success. Might merge with that one. 
			break; 
		case inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY:
			// This is OK, no nothing, but cursor might need to be updated.
			// However, this is at the start of the buffer, so nothing to remove. 
			break; 
		default:
			// Unknown return, throw an error. 
			break;
	}
}
