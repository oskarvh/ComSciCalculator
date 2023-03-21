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
 *
 * March 14(pi day):
 * I'm starting to think that the setup is a bit too advanced. 
 * I don't remeber the reason why I had separated the string entry and
 * the operator. I think it was due to the solver (which hasn't been written
 * yet) but I'm starting to think that 400 lines of code just to add a char
 * to a list is a bit on the heavy side. Therefore, I think I'll re-structure
 * the list entry so that it either takes an operator, function, bracket or 
 * char, and then just handle the solver later. One thing that is nice with 
 * having a string per entry is that solving will probably be easier, but at 
 * the same same time, that will most likely not be the main issue, as I would
 * have to convert everything to string after all. 
 * Another nice thing with the entry containing an entire string is that 
 * handling base changes (hex/dec/bin) is easier. 
 * It's a tradeoff though. I'll try it out using linker switches first though. 
 *
 * March 15:
 * I spent a whole lot of time thinking this through, and in the spirit of 
 * keeping things simple, I decided to act on last nights decision. 
 * Tests that the old code was failing is now passing. Although a first
 * pass didn't test cursor != 0, they are at least passing the random input tests. 
 * I'm feeling happier with this solution, as it is simpler. 
 * I don't think that the solver would have been a whole lot easier with the 
 * last approach, but changing input base would. That being said, it is not 
 * impossible to do. Come to think of it, the approach would be the same except
 * finding the start and end of the string is less efficient now, but not harder. 
 *
 */

/*
Overview:
List entries are a doubly linked list where each element consists of either:
1. A string (list)
2. an operator
3. a custom function
4. empty. 

- A list entry can have a string and an operator, if that operator does not increase depth. 
- A list that has an that operator increase depth cannot have a string. 
- A custom function acts like an operator, but always increase depth. 
- A bracket entry is an entry which increases depth, but does not have a custom function, 
  an operator or a string. 
- An entry can only be empty with none of those, only if there is no input. 



*/

/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */
// comsci header file
#include "comscicalc.h"

// Standard library
#include <stdio.h>
#include <string.h>

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
			if( ('0' <= c) && (c <= '9') ){
				return true;
			}
			break;
    	case inputBase_HEX:
    		if( (('0' <= c) && (c <= '9')) ||  (('a' <= c) && (c <= 'f')) ){
				return true;
			}
			break;
    	case inputBase_BIN:
    		if( ('0' <= c) && (c <= '1') ){
				return true;
			}
			break;
    	default:
    		// Return false if input base is set to None. 
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

/* Function to check if character is bracket
   Args: 
   - c: incoming character. 
   Returns:
   - True if char is bracket, otherwise false
*/
static bool charIsBracket(char c){
	if( (c == '(') || (c == ')') ){
		return true;
	}
	return false;
}

/* Function to get the operator entry based on input character. 
   Args: 
   - c: incoming character. 
   Returns:
   - pointer to operator if successful, otherwise NULL
*/
static const operatorEntry_t *getOperator(char c){
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

/* Function to initialize a list entry, excluding pointer
   Args: 
   - pListEntry: Pointer to list entry
*/
#ifndef UNIFIED_STRING_ENTRY
static void initListEntry(inputListEntry_t *pListEntry){
	pListEntry->pPrevious				= NULL;
	pListEntry->pNext					= NULL;
	pListEntry->inputBase				= inputBase_NONE;
	pListEntry->pInputStringEntry		= NULL;
	pListEntry->pLastInputStringEntry	= NULL;
	pListEntry->op						= operators_NONE;
	pListEntry->pCustomFunction			= NULL;
	pListEntry->depth 					= 0;
}
#endif
/* Function to check if input character corresponds to custom function. 
   Args: 
   - c: Input character. 
   Returns:
   - True if there is a custom function corresponding to this char, otherwise false
*/
static bool charIsCustomFunction(char c){
	// TODO
	return false;
}

/* Function to check health of list entry. 
   Args: 
   - pListEntry: Pointer to list entry 
   Returns:
   - True if list entry is healty, false otherwise
*/
#ifndef UNIFIED_STRING_ENTRY
static bool listEntryHealty(inputListEntry_t *pListEntry){
	// Checks for operator
	if(pListEntry->op != operators_NONE){
		// Check 1: If an operator is present, there must be an entry after this. 
		if(pListEntry->pNext == NULL){
			return false;
		}
		// Check 2: If an operator requires an increase in depth, and there isn't one in entry
		if(getOperator(pListEntry->op)->bIncDepth){
			// There is always a next operator, as we've already checked this
			if(pListEntry->depth == ((inputListEntry_t*)(pListEntry->pNext))->depth){
				return false;
			}
		}
		// Check 3: Cannot have operator and custom function
		if(pListEntry->pCustomFunction != NULL){
			return false;
		}
	}

	// Checks for custom function
	if(pListEntry->pCustomFunction != NULL){
		// Check 1: If an custom function is present, there must be an entry after this. 
		if(pListEntry->pNext == NULL){
			return false;
		}
		// Check 2: Custom functions require a depth increase
		// There is always a next operator, as we've already checked this
		if(pListEntry->depth == ((inputListEntry_t*)(pListEntry->pNext))->depth){
			return false;
		}
		// Check 3: Cannot have an operator and custom function at the same time. 
		if(pListEntry->op != operators_NONE){
			return false;
		}
	}
	return true;
}
#endif
/* Function to check an entry corresponds to a depth increasing operator 
   Args: 
   - pListEntry: Pointer to list entry 
   Returns:
   - True if list entry is depth increasing operator
*/
#ifndef UNIFIED_STRING_ENTRY
static bool entryIsDepthIncreasingOperator(inputListEntry_t *pListEntry){
	if( (pListEntry->op != operators_NONE) && 
		(pListEntry->depth == (((inputListEntry_t*)(pListEntry->pNext))->depth)+1) ){
		return true;
	}
	return false;
}
#endif
/* -------- CALCULATOR CORE FUNCTIONS -------- */

/* Function get the pointer to a custom function
   Args: 
   - inputChar: the character which corresponds to the custom function. 
   Returns:
   - Pointer to that custom funciton entry
*/
static customFunc_t *getCustomFunction(char inputChar){
	// TODO
	return NULL;
}

/* -------- CALCULATOR CORE FUNCTIONS -------- */



/* Function initialize the calculator core. This will allocate the first list entry as well. 
   Args: 
   - pCalcCoreState: pointer to an allocated calculator core state
   Returns:
   - Status
   State:
   - Untested, Unfinished
*/
calc_funStatus_t calc_coreInit(calcCoreState_t *pCalcCoreState){
	// Check the calc code pointer
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}

	// Initialize the cursor to 0
	pCalcCoreState->cursorPosition = 0;

	// Set the input base to NONE
	pCalcCoreState->inputBase = inputBase_NONE;

	// Set the first pointer to NULL
	pCalcCoreState->pListEntrypoint = NULL;

	return calc_funStatus_SUCCESS;
}

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
		calcCoreState_t *calcCoreState,
		inputListEntry_t **ppInputListAtCursor
		)
{
	uint8_t cursorPosition = calcCoreState->cursorPosition;
	inputListEntry_t *pListEntry = calcCoreState->pListEntrypoint;

	// Check pointer to input list
	if(pListEntry != NULL){
		// Find the pointer to the last list entry
		while(pListEntry->pNext != NULL){
			pListEntry = pListEntry->pNext;
		}

		// Prevent wrap-around issues
		if( ((int8_t)cursorPosition) < 0 ){
			return inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY;
		}

		// Start going backwards. 
		for(uint8_t i = 0 ; i < cursorPosition ; i++){
			// Check if there is a previous entry
			if(pListEntry != NULL){
				pListEntry = pListEntry->pPrevious;
			}
			else {
				// If there isn't then return the number
				// of steps taken in this list. 
				*ppInputListAtCursor = pListEntry;
				return i;
			}
		}
	}
	// Set the return values and return the state. 
	*ppInputListAtCursor = pListEntry;

	return inputModStatus_SUCCESS;
}

/* Function to tear down the calculator core state and deallocate ALL buffers. 
   NOTE: Must run calc_coreInit before running any other calculator core functions after this. 
   Args: 
   - pCalcCoreState: pointer to an allocated calculator core state
   Returns:
   - Status
   State:
   - Untested, Unfinished
*/
calc_funStatus_t calc_coreBufferTeardown(calcCoreState_t *pCalcCoreState){
	// Check the calc code pointer
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}

	// Get the pointer to the list entry.
	inputListEntry_t *pListEntry = pCalcCoreState->pListEntrypoint;
    // List entry is allowed to be NULL as well
	if(pListEntry != NULL){
    	// Find the first entry, if this isn't it. 
    	while( (pListEntry->pPrevious) != NULL){
    		pListEntry = (inputListEntry_t*)(pListEntry->pPrevious);
    	}

    	// Go from start to finish and free all entries
    	while(pListEntry != NULL){
    		// Free the list entry
    		inputListEntry_t *pNext = (inputListEntry_t *)pListEntry->pNext;
    		free(pListEntry);
    		pListEntry = pNext;
    	}
    }

	// Note: We should not free the calcCoreState, as it's statically allocated

	return calc_funStatus_SUCCESS;
}

/* Function to add a character or operator to the entry list
   Args: 
   - pInputList: Pointer to first entry of the input list
   - pCalcCoreState: pointer to calculator core state
   Returns:
   - state of the function
   State:
   - Draft state

*/
calc_funStatus_t calc_addInput( 
	calcCoreState_t* pCalcCoreState,
	char inputChar
	)
{
	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}
	inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;

	// Get the current list and string entries based on the cursor. 
	inputListEntry_t *pCurrentListEntry;
	inputModStatus_t listState = getInputListEntry(
		pCalcCoreState,
		&pCurrentListEntry
	);	

	if(listState > 0){
		// Cursor went too far. TBD should this be recified here?
		pCalcCoreState->cursorPosition = (uint8_t)listState;
	}

	// Allocate a new entry
	inputListEntry_t *pNewListEntry = malloc(sizeof(inputListEntry_t));

	if(pNewListEntry == NULL){
		return calc_funStatus_ALLOCATE_ERROR;
	}
	pNewListEntry->pFunEntry = NULL;

	// Add the input
	pNewListEntry->entry.c = inputChar;
	if(charIsNumerical(pCalcCoreState->inputBase,inputChar)){
		pNewListEntry->entry.typeFlag = 
			CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
	} 
	else if( charIsOperator(inputChar) ){
		// Get the operator
		const operatorEntry_t *pOp = getOperator(inputChar);
		if(pOp->bIncDepth){
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_INCREASE, INPUT_TYPE_OPERATOR);
		}
		else {
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_KEEP, INPUT_TYPE_OPERATOR);
		}
		pNewListEntry->pFunEntry = (void*)pOp;

	}
	else if( charIsBracket(inputChar) ){
		if(inputChar == OPENING_BRACKET){
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_INCREASE, INPUT_TYPE_EMPTY);
		}
		else {
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_DECREASE, INPUT_TYPE_EMPTY);
		}
	}
	else if( charIsCustomFunction(inputChar) ){
		customFunc_t *pCustomFunction = getCustomFunction(inputChar);
		// Custom functions always increase depth
		pNewListEntry->entry.typeFlag = 
			CONSTRUCT_TYPEFLAG(DEPTH_CHANGE_INCREASE, INPUT_TYPE_FUNCTION);
		pNewListEntry->pFunEntry = (void*)pCustomFunction;
	}
	else{
		// Unknown input. Free and return
		if(pNewListEntry != NULL){
		  free(pNewListEntry);
		}
		return calc_funStatus_UNKNOWN_INPUT;
	}

	// Add the current input base. Note: base change and propagation not handled here
	pNewListEntry->inputBase = pCalcCoreState->inputBase;

	// Add the new list entry to the correct place in the list
	if(pCurrentListEntry == NULL){
		// Top of the list. Add new entry before, and change the list entry point
		pNewListEntry->pNext = pCalcCoreState->pListEntrypoint;
		pNewListEntry->pPrevious = NULL;
		if(pNewListEntry->pNext != NULL){
			((inputListEntry_t*)(pNewListEntry->pNext))->pPrevious = pNewListEntry;
		}
		pCalcCoreState->pListEntrypoint = pNewListEntry;
	}
	else{
		// Add entry after the current entry
		pNewListEntry->pPrevious = pCurrentListEntry;
		pNewListEntry->pNext = pCurrentListEntry->pNext;
		if(pNewListEntry->pNext != NULL){
			((inputListEntry_t*)(pNewListEntry->pNext))->pPrevious = pNewListEntry;
		}
		if(pNewListEntry->pPrevious != NULL){
			((inputListEntry_t*)(pNewListEntry->pPrevious))->pNext = pNewListEntry;
		}
	}
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
calc_funStatus_t calc_removeInput(calcCoreState_t* pCalcCoreState){
	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}
	inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;

	// Get the current list and string entries based on the cursor. 
	inputListEntry_t *pCurrentListEntry;
	inputModStatus_t listState = getInputListEntry(
		pCalcCoreState,
		&pCurrentListEntry
	);	

	// The aim is to remove the pointer currently pointed at, 
	// so first align the pointers of previous and next entires
	// and then free the memory. 
	if(pCurrentListEntry->pNext != NULL){
		((inputListEntry_t*)(pCurrentListEntry->pNext))->pPrevious = 
			pCurrentListEntry->pPrevious;
	}
	if(pCurrentListEntry->pPrevious != NULL){
		((inputListEntry_t*)(pCurrentListEntry->pPrevious))->pNext = 
			pCurrentListEntry->pNext;
	}
	if(pCurrentListEntry != NULL){
		free(pCurrentListEntry);
	}
	
	return calc_funStatus_SUCCESS;
}


/* Function to print the list entries. 
   This function relies on the list being in good shape.  
   Args: 
   - pCalcCoreState: pointer to calculator core state
   - pString: pointer to string which to print the buffer
   - stringLen: maximum number of characters to write to pString
   Returns:
   - state of the function. 
   State:
   - Untested, Unfinished
*/
calc_funStatus_t calc_printBuffer(calcCoreState_t* pCalcCoreState, char *pResString, uint16_t stringLen){

	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}
	inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;
	
	// Check pointer to input list
	if(pCurrentListEntry == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Check the string entry
	if(pResString == NULL){
		calc_funStatus_STRING_BUFFER_ERROR;
	}

	// Make a local variable of the string entry to interate on. 
	char *pString = pResString;

	// Variable to keep track of the number of chars written to string
	// Add one as as we need a null terminator at the end. 
	uint16_t numCharsWritten = 1;

	// Loop through all buffers
	uint8_t previousInputType = INPUT_TYPE_EMPTY;
	while(pCurrentListEntry != NULL){
		// Depending on the input type, print different things. 
		uint8_t currentInputType = GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag);

		if(currentInputType == INPUT_TYPE_NUMBER){
			// If the previous input type wasn't a number, 
			// then print the precursor. For hex it's 0x, for bin it's 0b
			if(previousInputType != currentInputType){
				if(pCurrentListEntry->inputBase == inputBase_HEX){
					// Print '0x' if there is room
					if(numCharsWritten < (stringLen - 2) ){
						numCharsWritten += sprintf(pString, "0x");
						// Increase the pointer two steps. 
						pString += 2;
					}
					else {
						return calc_funStatus_STRING_BUFFER_ERROR;
					}
				}
				if(pCurrentListEntry->inputBase == inputBase_BIN){
					// Print '0b' if there is room
					if(numCharsWritten < (stringLen - 2) ){
						numCharsWritten += sprintf(pString, "0b");
						// Increase the pointer two steps. 
						pString += 2;
					}
					else {
						return calc_funStatus_STRING_BUFFER_ERROR;
					}
				}
			}
			if(numCharsWritten < stringLen){
				*pString++ = pCurrentListEntry->entry.c;
				numCharsWritten++;
			}
			else {
				return calc_funStatus_STRING_BUFFER_ERROR;
			}
		}
		else if(currentInputType == INPUT_TYPE_OPERATOR){
			// Input is operator. Print the string related to that operator. 
			const operatorEntry_t *pOperator = (operatorEntry_t*)pCurrentListEntry->pFunEntry;
			uint8_t tmpStrLen = strlen(pOperator->opString);

			if(numCharsWritten < stringLen-tmpStrLen){
				numCharsWritten += sprintf(pString, pOperator->opString);
				pString += tmpStrLen;
			}
			else {
				return calc_funStatus_STRING_BUFFER_ERROR;
			}
			// If the operator increase depth, then print an opening bracket too
			if(GET_DEPTH_FLAG(pCurrentListEntry->entry.typeFlag) == DEPTH_CHANGE_INCREASE){
				if(numCharsWritten < stringLen){
					*pString++ = '(';
					numCharsWritten++;
				}
				else {
					return calc_funStatus_STRING_BUFFER_ERROR;
				}
			}
		}
		else if(currentInputType == INPUT_TYPE_FUNCTION){
			// Input is custom function. Print the string related to that function. 
			const customFunc_t *pCustomFunction = (customFunc_t*)pCurrentListEntry->pFunEntry;
			uint8_t tmpStrLen = strlen(pCustomFunction->opString);

			if(numCharsWritten < stringLen-tmpStrLen){
				numCharsWritten += sprintf(pString, pCustomFunction->opString);
				pString += tmpStrLen;
			}
			else {
				return calc_funStatus_STRING_BUFFER_ERROR;
			}
			// The function should always increase depth, but check if it should and 
			// add opening bracket. 
			if(GET_DEPTH_FLAG(pCurrentListEntry->entry.typeFlag) == DEPTH_CHANGE_INCREASE){
				if(numCharsWritten < stringLen){
					*pString++ = '(';
					numCharsWritten++;
				}
				else {
					return calc_funStatus_STRING_BUFFER_ERROR;
				}
			}
		}
		else if(currentInputType == INPUT_TYPE_EMPTY){
			// This should only be brackets, but it's at least only one char. 
			if(numCharsWritten < stringLen){
				*pString++ = pCurrentListEntry->entry.c;
				numCharsWritten++;
			}
			else {
				return calc_funStatus_STRING_BUFFER_ERROR;
			}
		}
		else {
			// List was broken. This should not happen. 
			return calc_funStatus_ENTRY_LIST_ERROR;
		}

		previousInputType = currentInputType;
		pCurrentListEntry = pCurrentListEntry->pNext;
	}

	return calc_funStatus_SUCCESS;
}


/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/
