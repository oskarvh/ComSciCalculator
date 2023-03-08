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
 * Although, it's a bit confusing having both preceding operator and normal operator
 * in one entry. 
 * 
 * Conclusion is that there shall only be one operator per entry. 
 * so a NOT() operator shall always have a next list with one higher depth. 
 * Same goes for parenthesis. Do this and all tests of getInputListEntry passes
 */



/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */
// comsci header file
#include "comscicalc.h"

// Operator functions
#include "comscicalc_operators.h"

// Standard library

/* ------------- GLOBAL VARIABLES ------------ */

/* ---- CALCULATOR CORE HELPER FUNCTIONS ----- */

/* Function to check if character is numerical. 
   Args: 
   - base: which base is currently active. 
   - c: incoming character. 
   Returns:
   - True if c is numerical input, false otherwise. 
   - False if input base ins't defined as well. 
   Note: should not be used to determine if input is operator, as 
   it will return false for all non-numerical types in the active base
*/
static bool charIsNumerical(inputBase_t base, char c){
	switch(base){
		case inputBase_DEC:
			if('0' <= c <= '9'){
				return true;
			}
			break;
    	case inputBase_HEX:
    		if( ('0' <= c <= '9') || ('a' <= c <= 'f') ){
				return true;
			}
			break;
    	case inputBase_BIN:
    		if('0' <= c <= '1'){
				return true;
			}
			break;
    	default:
    		break;
	}
	return false;
}

/* Function to check if character is operator
   Args: 
   - c: incoming character. 
   Returns:
   - True if char is in operator database, false otherwise
*/
static bool charIsOperator(char c){
	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 
	for(uint8_t i = 0 ; i < NUM_OPERATORS ; i++){
		if(c == operators[i].inputChar){
			// Operator entry found!
			return true;
		}
	}
	return false;
}

/* Function to get the operator entry based on input character. 
   Args: 
   - c: incoming character. 
   Returns:
   - pointer to operator if successful, otherwise NULL
   Note: should not be used to detrmine if input is operator, as 
   it will return false for all non-numerical types
*/
static operatorEntry_t *getOperator(char c){
	// Loop through the operator array and check if the operator is in there. 
	// Not a nice way to do it, but the array is fairly small. 
	for(uint8_t i = 0 ; i < NUM_OPERATORS ; i++){
		if(c == operators[i].inputChar){
			// Operator entry found!
			return &operators[i];
		}
	}
	return NULL;
}

/* -------- CALCULATOR CORE FUNCTIONS -------- */

/* Function to find the input list entry and input character entry
   Args: 
   - pInputList: Pointer to first entry of the input list
   - cursorPosition: Position of the cursor from the end.
   - ppInputListAtCursor: Pointer to pointer to inputList entry
   - ppInputString: Pointer to pointer to string entry
   Returns:
   - state of the function
   State: 
   - Function tested successfully
*/
static inputModStatus_t getInputListEntry(
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
			// Go to the previous list entry, if it exists. 
			if(pListEntry->pPrevious != NULL){
				// Cursor at previous operator, 				
				pListEntry = pListEntry->pPrevious;
				// Check if there is an operator, 
				// If yes, then set state to be at operator entry, 
				// Otherwise the previous entry didn't have an operator, 
				// which is the case of e.g. parenthesis
				if(pListEntry->op != operators_NONE){
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

/* Function to add a character or operator to the entry list
   Args: 
   - pInputList: Pointer to first entry of the input list
   - pCalcCoreState: pointer to calculator core state
   Returns:
   - state of the function
   State:
   - Untested, Unfinished

   Function test cases:
   1. pInputList = NULL
      - Returns: INPUT_LIST_NULL
   2. pCalcCoreState = NULL
      - Returns: CALC_CORE_STATE_NULL
   3. Add character to end (permutations: empty entires, non-emtpy entries)
   4. Add operator at end
   5. Add '(' at end
   6. Add ')' at end
   7. Add character in middle of string buffer 
   8. Add operator in middle of string buffer
   9. Add '(' in middle of string buffer
   10.Add ')' in middle of string buffer
   11.Add character at entry without string (e.g. parentheses)
   12.Add operator at entry without string (e.g. parentheses)
   13.Add '(' at entry without string (e.g. parentheses)
   14.Add ')' at entry without string (e.g. parentheses)

   n. test1
      - Example
      - Returns 

*/
calc_funStatus_t calc_addInput(
	inputListEntry_t *pInputList, 
	calcCoreState_t* pCalcCoreState,
	char inputChar
	)
{
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
			// Check if adding a new entry, in which case use the incoming base
			// for the entry. If already in an existing list, then we either convert that
			// entry or throw an error. 
			if(pCurrentStringEntry == NULL){
				// No entry in string buffer, add new with incoming base. 
				if(charIsNumerical(pCalcCoreState->inputBase,inputChar)){
					// Allocate new string entry and populate with character
					inputStringEntry_t *newStringEntry = malloc(sizeof(inputStringEntry_t));
					newStringEntry->c = inputChar;
					newStringEntry->pPrevious = NULL;
					newStringEntry->pNext = NULL;
					// TBD: How to handle the case where this is a parentheses or 
					// operator only entry? 
				}
				else if(charIsOperator(inputChar)){
					// Character is operator, add this to the current entry 
					// and allocate a new entry and add to list. 
					// Check if existing operator exists as well, in which case
					// shoehorn that in here. Most likely this at the end of the entry
					// list, otherwise it should have been handled by the 
					// LIST_AT_OPERATOR_ENTRY state. 
					// TODO
				}
				else if(inputChar == '('){
					// Opening bracket. 
					// TODO
				}
				else if(inputChar == ')'){
					// Closing bracket. 
					// TODO
				}
				else{
					// Nothing of the above, therefore unknown input. Return with status
					return calc_funStatus_UNKNOWN_INPUT;
				}
			}
			else{
				// Exising list, we are either in the middle of a string entry, 
				// or at the end of a string entry. 
				// For now, this is not allowed, so return with error
				if(pCalcCoreState->inputBase != pCurrentListEntry->inputBase){
					return calc_funStatus_INPUT_BASE_ERROR;
				}
				// Here we are in the middle of a string, so shoehorn in either new
				// character or operator. Note that operator would split the string 
				// array and make a new entry. 
				// TODO
			}
			
			break;
			// If an operator, then create a new list entry splitting the list. 
		case inputModStatus_LIST_AT_OPERATOR_ENTRY:
			// Everything went well, we can add input into list
			// Char shall be added at the end of the string buffer
			// Operator shall replace the current operator, and the current
			// operator shall be inserted in a new list entry. 
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
   State:
   - Untested, Unfinished

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
		case inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY:
			// This is OK, no nothing, but cursor might need to be updated.
			// However, this is at the start of the buffer, so nothing to remove. 
			break; 
		default:
			// Unknown return, throw an error. 
			break;
	}
}




/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/
inputModStatus_t getInputListEntryWrapper(
		inputListEntry_t *pInputList, 
		uint8_t cursorPosition,
		inputListEntry_t **ppInputListAtCursor, 
		inputStringEntry_t **ppInputString)
{
	return getInputListEntry(
		pInputList, 
		cursorPosition,
		ppInputListAtCursor, 
		ppInputString);
}