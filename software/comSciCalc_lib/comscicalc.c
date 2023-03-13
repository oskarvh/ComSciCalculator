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

// Operator functions
#include "comscicalc_operators.h"

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

/* Function to check an entry corresponds to a depth increasing operator 
   Args: 
   - pListEntry: Pointer to list entry 
   Returns:
   - True if list entry is depth increasing operator
*/
static bool entryIsDepthIncreasingOperator(inputListEntry_t *pListEntry){
	if( (pListEntry->op != operators_NONE) && 
		(pListEntry->depth == (((inputListEntry_t*)(pListEntry->pNext))->depth)+1) ){
		return true;
	}
	return false;
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
		calcCoreState_t *calcCoreState,
		inputListEntry_t **ppInputListAtCursor, 
		inputStringEntry_t **ppInputString)
{
	uint8_t cursorPosition = calcCoreState->cursorPosition;
	// Check pointer to input list
	if(calcCoreState->pListEntrypoint == NULL){
		return inputModStatus_INPUT_LIST_NULL;
	}

	// Find the pointer to the last list entry
	inputListEntry_t *pListEntry = calcCoreState->pListEntrypoint;
	while(pListEntry->pNext != NULL){
		pListEntry = pListEntry->pNext;
	}

	// The cursor is only allowed to be unsigned:
	if(cursorPosition > (uint8_t)((int8_t)-1)){
		return inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY;
	}

	// Get the pointer to the last list entry
	inputStringEntry_t *pStringEntry = pListEntry->pLastInputStringEntry;

	// Start going backwards. 
	for(uint8_t i = 0 ; i < cursorPosition ; i++){
		// If the string entry is not NULL, keep going backwards in the string 
		if(pStringEntry != NULL){
			pStringEntry = pStringEntry->pPrevious;
		}
		else {
			// If string entry is NULL, then we need to proceed to the previous entry
			// We should then end up at the previous entires last input string entry point

			// Go to the previous list entry, if it exists. 
			if(pListEntry != NULL){
				// Go to previous list entry				
				pListEntry = pListEntry->pPrevious;
			}
			else {
				// If there is no previous list entry, hence we've
				// reaced the start of the list. 
				// Return the iterator
				return i;
			}
			// Set the current string entry if there is a list entry. 
			if(pListEntry != NULL){
				// And set pointer at last string entry. Note, this is allowed to be NULL
				pStringEntry = pListEntry->pLastInputStringEntry;
			}
		}
	}
	// Set the return values and return the state. 
	*ppInputListAtCursor = pListEntry;
	*ppInputString = pStringEntry;

	return inputModStatus_SUCCESS;
}

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

	// Allocate the first list entry
	//int32_t *pTmp = calloc(28, 1);

	pCalcCoreState->pListEntrypoint = malloc(sizeof(inputListEntry_t));

	// Check if the allocation failed
	if(pCalcCoreState->pListEntrypoint == NULL){
		return calc_funStatus_ALLOCATE_ERROR;
	}

	// Initialize the list entry
	initListEntry(pCalcCoreState->pListEntrypoint);

	return calc_funStatus_SUCCESS;
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

	inputListEntry_t *pListEntry = pCalcCoreState->pListEntrypoint;
	// Check if the entry point pointer is NULL
	if(pListEntry == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Find the start of the entry list
	while( (pListEntry->pPrevious) != NULL){
		pListEntry = (inputListEntry_t*)(pListEntry->pPrevious);
	}

	// Go from start to finish and free all string entires and list entries
	while(pListEntry != NULL){
		inputStringEntry_t *pStringEntry = pListEntry->pInputStringEntry;
		while(pStringEntry != NULL){
			inputStringEntry_t *pNext = (inputStringEntry_t *)(pStringEntry->pNext);
			free(pStringEntry);
			pStringEntry = pNext;
		}
		// Free the list entry
		inputListEntry_t *pNext = (inputListEntry_t *)pListEntry->pNext;
		free(pListEntry);
		pListEntry = pNext;
	}

	// We should not free the calcCoreState, it not be malloced. 

	return calc_funStatus_SUCCESS;
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
	calcCoreState_t* pCalcCoreState,
	char inputChar
	)
{
	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}
	inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;

	// Check pointer to input list
	if(pInputList == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Get the current list and string entries based on the cursor. 
	
	inputListEntry_t *pReturnedListEntry;
	inputStringEntry_t *pCurrentStringEntry;
	inputModStatus_t listState = getInputListEntry(
		pCalcCoreState,
		&pReturnedListEntry, 
		&pCurrentStringEntry
	);
	inputListEntry_t *pCurrentListEntry = pReturnedListEntry;
	

	// Check health of list entry. E.g. if operator is present, no custom function is allowed. 
	if(!listEntryHealty(pCurrentListEntry)){
		return calc_funStatus_ENTRY_LIST_ERROR;
	}

	// Check if current entry is NULL, meaning that the cursor is pointing before
	// the first entry. 
	if(pCurrentListEntry == NULL){
		// For all string entries, the current list entry should point
		// to the first entry. 
		pCurrentListEntry = pCalcCoreState->pListEntrypoint;
	}
	// Based on the input, handle the insertion. 
	if(charIsNumerical(pCalcCoreState->inputBase,inputChar)){
		
		// Needs to be handled differently based on different situtations
		// One thing is always true though, allocate a new string buffer entry
		// and insert the character. 
		inputStringEntry_t *pNewStringEntry = malloc(sizeof(inputStringEntry_t));
		if(pNewStringEntry == NULL){
			return calc_funStatus_ALLOCATE_ERROR;
		}
		pNewStringEntry->c = inputChar;
		pNewStringEntry->pNext = NULL;
		pNewStringEntry->pPrevious = NULL;

		// When inputting a character, and the current string is not NULL, then 
		// this is simply put into an existing string entry, either in the middle
		// or at the start. 
		if(pCurrentStringEntry != NULL){
			// Set the pointers to the newly created entry
			pNewStringEntry->pNext = pCurrentStringEntry->pNext;
			pNewStringEntry->pPrevious = pCurrentStringEntry;

			// Adjust the current strings pointer:
			pCurrentStringEntry->pNext = pNewStringEntry;

			// If there is an entry before this one, then also modify that. 
			if(pNewStringEntry->pNext != NULL){
				((inputStringEntry_t *)(pNewStringEntry->pNext))->pPrevious = pNewStringEntry;
			} 
			else{
				// If there isn't a next one, then this entry is at the end of the 
				// string list, therefore adjust the list last entry string end pointer
				pCurrentListEntry->pLastInputStringEntry = pNewStringEntry;
			}
		}
		else {
			// The current string entry is NULL, and could therefore be a couple of different 
			// situations
			if(pCurrentListEntry->pNext == NULL || pCurrentListEntry->pPrevious == NULL){
				
				// Check if this entry has a string from before, but cursor is at the start
				if(pCurrentListEntry->pInputStringEntry != NULL){
					
					pNewStringEntry->pNext = pCurrentListEntry->pInputStringEntry;
					pNewStringEntry->pPrevious = NULL;

					// Modify the next entry, if any
					if(pNewStringEntry->pNext != NULL){
						((inputStringEntry_t *)(pNewStringEntry->pNext))->pPrevious = pNewStringEntry;
					}
					// Modify the string list entry point:
					pCurrentListEntry->pInputStringEntry = pNewStringEntry;
				}
				else {
					// If there isn't a next entry, and there is no string, then 
					// attach the string to the list entry. This could be the case e.g, 
					// after a '+' input. 
					pCurrentListEntry->pInputStringEntry = pNewStringEntry;
					pCurrentListEntry->pLastInputStringEntry = pNewStringEntry;
				}
				

			}
			else{
				// There is a next and a previous list entry
				if(pCurrentListEntry->depth != ((inputListEntry_t *)(pCurrentStringEntry->pPrevious))->depth){
					// Depth is different between this entry and the next. 
					// This requires a new list entry, as this is either depth increasing operator, 
					// bracket or custom function, which none can have string entries. 
					// Insert the new entry before this current one. 
					inputListEntry_t *pNewListEntry = malloc(sizeof(inputListEntry_t));
					if(pNewListEntry == NULL){
						return calc_funStatus_ALLOCATE_ERROR;
					}
					initListEntry(pNewListEntry);
					// Set the depth of this entry to the previous entry's depth. 
					pNewListEntry->depth = ((inputListEntry_t*)(pCurrentListEntry->pPrevious))->depth;

					// Set the input base of this new entry of the user input base
					pNewListEntry->inputBase = pCalcCoreState->inputBase;

					// Now add the string entry to this newly created list. 
					pNewListEntry->pInputStringEntry = pNewStringEntry;
					pNewListEntry->pLastInputStringEntry = pNewStringEntry;

					// And at last, add this new list entry before the current one
					pNewListEntry->pPrevious = pCurrentListEntry->pPrevious;
					pNewListEntry->pNext = pCurrentListEntry;
					// Change the pointers of previous entry to point to this new one
					((inputListEntry_t*)(pCurrentListEntry->pPrevious))->pNext = pNewListEntry;
					// And then change the current's previous pointer to the new list entry
					pCurrentListEntry->pNext = pNewListEntry;
				}
				else{
					// This entry has no string and no depth increase. Could be the case where
					// there are several non-depth increasing operators, e.g. '+++++'. 
					// Simply add the new string entry
					pCurrentListEntry->pInputStringEntry = pNewStringEntry;
					pCurrentListEntry->pLastInputStringEntry = pNewStringEntry;
				}
			}
			
		}
	}
	else if(charIsOperator(inputChar)){
		// If depth increasing operator:
		// 1. pCurrentEntry->pNext->operator = Depth increasing operator
		// 2. pCurrentEntry->pNext->customFuntion = None
		// 3. pCurrentEntry->pNext->depth = pCurrentEntry->depth + 1
		// 4. pCurrentEntry->pNext->pNext != NULL
		// Simply put, an entry with only operator and depth increase is operator.
		bool bIncreaseDepth = getOperator(inputChar)->bIncDepth;

		// All operators require a new list entry
		inputListEntry_t *pNewListEntry = malloc(sizeof(inputListEntry_t));
		if(pNewListEntry == NULL){
			return calc_funStatus_ALLOCATE_ERROR;
		}
		initListEntry(pNewListEntry);
		
		// Inherit the depth of the current one, this is increased 
		// at the end either way. 
		pNewListEntry->depth = pCurrentListEntry->depth;


		// Case is the same except depth increase if the input is depth increasing, 
		// or if the current entry already has a current operator. 
		//if(getOperator(inputChar)->bIncDepth || (pCurrentListEntry->op != operators_NONE) ){

		// Check if this entry has a string entry. 
		if(pCurrentListEntry->pInputStringEntry != NULL){
			// Check if we're inside, at the end or at the start of the string
			if(pCurrentStringEntry == NULL){
				// Beginning of string entry - Add the new list entry before. 
				pNewListEntry->pNext = pCurrentListEntry;
				pNewListEntry->pPrevious = pCurrentListEntry->pPrevious;
				pCurrentListEntry->pPrevious = pNewListEntry;
				// Check if there is a previous entry
				if(pNewListEntry->pPrevious != NULL){
					((inputListEntry_t*)(pNewListEntry->pPrevious))->pNext = pNewListEntry;	
					// Increase the depth from the previous entry instead of this one. 
					pNewListEntry->depth = ((inputListEntry_t*)(pNewListEntry->pPrevious))->depth;
					// Note: if there was an earlier entry, that means
					// that there was an earlier operator/bracket. 
				}
				else{
					// The new list was added at the first entry. 
					// Take care of the core state entry point. 
					pCalcCoreState->pListEntrypoint = pNewListEntry;
					pNewListEntry->depth = 0;
				}
				// Add operator to the new entry. 
				pNewListEntry->op = inputChar;
			}
			else if(pCurrentStringEntry ==  pCurrentListEntry->pLastInputStringEntry){
				// End of string entry. Add entry at the end if 
				// there is already an operator, or if the operator
				// requires an increase in depth. 
				pNewListEntry->pNext = pCurrentListEntry->pNext;
				pNewListEntry->pPrevious = pCurrentListEntry;
				pCurrentListEntry->pNext = pNewListEntry;

				// Handle where to put the operator
				if( bIncreaseDepth || (pCurrentListEntry->op != operators_NONE) ){
					// Add operator to the new entry. 
					pNewListEntry->op = inputChar;

					// Check if there is a next entry
					if(pNewListEntry->pNext != NULL){
						((inputListEntry_t*)(pNewListEntry->pNext))->pPrevious = pNewListEntry;	
					}
					else{
						// If there isn't a next entry, and we've added 
						// an entry with an operator, we actually need to add one more,
						// since there should always be a list entry after an operator. 
						inputListEntry_t *pAddedListEntry = malloc(sizeof(inputListEntry_t));
						if(pAddedListEntry == NULL){
							return calc_funStatus_ALLOCATE_ERROR;
						}
						initListEntry(pAddedListEntry);
						// Add the new list entry after. 
						pAddedListEntry->pNext = NULL;
						pAddedListEntry->pPrevious = pNewListEntry;
						pNewListEntry->pNext = pAddedListEntry;
					}
					
				}
				else{
					// Current entry does not have an operator, 
					// and the new operator is not increasing the depth. Add here
					pCurrentListEntry->op = inputChar;
				}
			}
			else {
				// In the middle of the string. 

				// If the operator requires depth increase, 
				// we actually need two new list entries here: 
				// The one we already made, to hold the operator, and then a 
				// next one to hold the detached string after this entry. 
				if(bIncreaseDepth){

					inputListEntry_t *pDetachedListEntry = malloc(sizeof(inputListEntry_t));
					if(pDetachedListEntry == NULL){
						return calc_funStatus_ALLOCATE_ERROR;
					}
					initListEntry(pDetachedListEntry);
					// Move the operator from the current list entry to the next one, 
					// as operators always comes last. 
					pDetachedListEntry->op = pCurrentListEntry->op;
					// and set the head list operator to none. 
					pCurrentListEntry->op = operators_NONE;
					// Set the pointers to the newly created detached list entry
					pDetachedListEntry->pPrevious = pNewListEntry;
					pDetachedListEntry->pNext = pCurrentListEntry->pNext;
					pCurrentListEntry->pNext = pNewListEntry;

					// Split up the string entry. First the end point of the string
					pDetachedListEntry->pLastInputStringEntry = pCurrentListEntry->pLastInputStringEntry;
					pCurrentListEntry->pLastInputStringEntry = 	pCurrentStringEntry;
					// .. and the string entry points:
					pDetachedListEntry->pInputStringEntry = pCurrentStringEntry->pNext;
					pCurrentListEntry->pInputStringEntry = pCurrentStringEntry;

					// Separate the string entries themselves
					((inputStringEntry_t *)(pCurrentListEntry->pLastInputStringEntry))->pNext = NULL;
					((inputStringEntry_t *)(pDetachedListEntry->pInputStringEntry))->pPrevious = NULL;

					// Insert the new list entry with the operator:
					pNewListEntry->pNext = pDetachedListEntry;
					pNewListEntry->pPrevious = pCurrentListEntry;

					// The detached head inherits the base from the current entry
					pDetachedListEntry->inputBase = pCurrentListEntry->inputBase;
					// The detached head inherits the depth of the new list entry. 
					pDetachedListEntry->depth = pNewListEntry->depth;

					// Add the current operator to the new list entry. 
					pNewListEntry->op = inputChar;
				}
				else {
					// The new entry inherits the younger part of the earlier part
					// of the string, and the existing entry inherits the last part. 
					// Add the new entry before the current entry. 
					pNewListEntry->pNext = pCurrentListEntry;
					pNewListEntry->pPrevious = pCurrentListEntry->pPrevious;
					pCurrentListEntry->pPrevious = pNewListEntry;

					// Check if this is the first entry
					if(pNewListEntry->pPrevious != NULL){
						((inputListEntry_t*)(pNewListEntry->pPrevious))->pNext = pNewListEntry;
					}
					else {
						pCalcCoreState->pListEntrypoint = pNewListEntry;
						pNewListEntry->depth = 0;
					}

					pNewListEntry->pInputStringEntry = pCurrentListEntry->pInputStringEntry;
					pNewListEntry->pLastInputStringEntry = pCurrentStringEntry;
					pCurrentListEntry->pInputStringEntry = pCurrentStringEntry->pNext;

					// Separate the string entries themselves
					((inputStringEntry_t *)(pNewListEntry->pLastInputStringEntry))->pNext = NULL;
					((inputStringEntry_t *)(pCurrentListEntry->pInputStringEntry))->pPrevious = NULL;
					// The operator is then added to the new entry. 
					// If the new entry replaces the existing first entry, 
					// also replace that. 
					pNewListEntry->op = inputChar;
				}
			}
		}
		else {
			// This entry does not have a string.  

			// The new list entry needs to be added prior to the entry the
			// cursor is pointing at. 
			pNewListEntry->pPrevious = pCurrentListEntry->pPrevious;
			pNewListEntry->pNext = pCurrentListEntry;
			pCurrentListEntry->pPrevious = pNewListEntry;
			pNewListEntry->op = inputChar;

			if(pNewListEntry->pPrevious != NULL){
				((inputListEntry_t*)(pNewListEntry->pNext))->pPrevious = pNewListEntry;	
			}
			// Special case: first entry:
			if(pNewListEntry->pPrevious == NULL){
				// Add new list entry to the top of the list. 
				pCalcCoreState->pListEntrypoint = pNewListEntry;
			}
			else {
				((inputListEntry_t*)(pNewListEntry->pPrevious))->pNext = pNewListEntry;	
			}
			
		}

		// Propagate and increase depth for all entries after this one. 
		if(bIncreaseDepth){
			inputListEntry_t* pTmpListEntry = (inputListEntry_t*)pNewListEntry->pNext;
			while(pTmpListEntry != NULL){
				// Increase depth
				pTmpListEntry->depth += 1;
				// Move on to next entry
				pTmpListEntry = (inputListEntry_t*)(pTmpListEntry->pNext);
			}
		}
	}
	else if(inputChar == OPENING_BRACKET){
		// When adding an opening bracket to pCurrenEntry, make a new list entry such that:
		// 1. pCurrentEntry->pNext->operator = None
		// 2. pCurrentEntry->pNext->customFuntion = None
		// 3. pCurrentEntry->pNext->depth = pCurrentEntry->depth + 1
		// 4. pCurrentEntry->pNext->pNext != NULL
		// Simply put, an empty entry with a depth increase corresponds to an opening bracket. 

		// Should be the same as depth increasing operator. 
	}
	else if(inputChar == CLOSING_BRACKET){
		// Should be the same as depth increasing operator, but decreasing the depth
		// instead. 
	}
	else if(charIsCustomFunction(inputChar)){
		// Should be the same as depth increasing operator. 
	}
	else{
		// Unknown input, return warning. 
		return calc_funStatus_UNKNOWN_INPUT;
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
calc_funStatus_t calc_removeInput(calcCoreState_t* pCalcCoreState){
	// Check pointer to calculator core state
	if(pCalcCoreState == NULL){
		return calc_funStatus_CALC_CORE_STATE_NULL;
	}
	inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;
	
	// Check pointer to input list
	if(pInputList == NULL){
		return calc_funStatus_INPUT_LIST_NULL;
	}

	// Get the current list and string entries based on the cursor. 
	inputListEntry_t *pCurrentListEntry;
	inputStringEntry_t *pCurrentStringEntry;
	inputModStatus_t listState = getInputListEntry(
		pCalcCoreState,
		&pCurrentListEntry, 
		&pCurrentStringEntry
	);

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
	inputListEntry_t *pInputList = pCalcCoreState->pListEntrypoint;
	
	// Check pointer to input list
	if(pInputList == NULL){
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
	while(pInputList != NULL){

		// Get the pointer to the string entries:
		inputStringEntry_t *pStringEntry = (inputStringEntry_t *)pInputList->pInputStringEntry;

		// If the string itsn't empty, then print the 0x or 0b if hex or dec
		if(pStringEntry != NULL){
			if(pInputList->inputBase == inputBase_HEX){
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
			if(pInputList->inputBase == inputBase_BIN){
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

		// Loop through the string 
		while(pStringEntry != NULL){
			if(numCharsWritten < stringLen){
				*pString++ = pStringEntry->c;
			}
			else {
				return calc_funStatus_STRING_BUFFER_ERROR;
			}
			pStringEntry = pStringEntry->pNext;
		}

		// Print the operator. Note, if no operator then it's just a null terminator. 
		const operatorEntry_t *pOperator = getOperator(pInputList->op);
		uint8_t tmpStrLen = strlen(pOperator->opString);
		if(numCharsWritten < stringLen-tmpStrLen){
			numCharsWritten += sprintf(pString, pOperator->opString);
			pString += tmpStrLen;
		}

		// Print the custom function. 
		customFunc_t *pCustomFunction = pInputList->pCustomFunction;
		if(pCustomFunction != NULL){
			tmpStrLen = strlen(pCustomFunction->opString);
			if(numCharsWritten < stringLen-tmpStrLen){
				numCharsWritten += sprintf(pString, pCustomFunction->opString);
				pString += tmpStrLen;
			}
		}

		// Print brackets:
		if(pInputList->pNext != NULL){
			if( ((inputListEntry_t*)(pInputList->pNext))->depth > pInputList->depth ){
				// Next entry has deeper depth, print opening bracket
				*pString++ = OPENING_BRACKET;
			}
			if( ((inputListEntry_t*)(pInputList->pNext))->depth < pInputList->depth ){
				// Next entry has shallower depth, print closing bracket
				*pString++ = CLOSING_BRACKET;
			}
		}

		// Go to the next list entry
		pInputList = pInputList->pNext;
	}
	// Add a NULL terminator at the end. 
	pString[numCharsWritten] = '\0';
	return calc_funStatus_SUCCESS;
}


/* -------------------------------------------
 * ------------ FUNCTION WRAPPERS ------------
 * -------------------------------------------*/
inputModStatus_t getInputListEntryWrapper(
		calcCoreState_t *calcCoreState,
		inputListEntry_t **ppInputListAtCursor, 
		inputStringEntry_t **ppInputString)
{
	return getInputListEntry(
		calcCoreState,
		ppInputListAtCursor, 
		ppInputString);
}