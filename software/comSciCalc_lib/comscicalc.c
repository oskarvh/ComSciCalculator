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
 * March 23:
 * I'm currently working on the solver, and I think I have a good enough idea
 * on how to handle that, but I have identified a few things that needs to be
 * addressed:
 * 1. It would be super nice with a pointer in the operator to a documentation
 *    string, so I added that. 
 * 2. I'm not sure how I want to handle the bitwise operators. I was first 
 *    planning to have them as OP(arg1, arg2), but I'm starting to think
 *    that it's cleaner to have it as arg1->OP->arg2. It should be enough
 *    to just change the operator to non-depth increasing for this. 
 * 3. However, we do need to add a comma ',' for the special functions. 
 *    This would be the argument separator. Should be easy enough to add 
 *    as it would be handled almost the same as brackets. 
 * 4. At some point, I need to add support for floating point and fixed 
 *    point. I can't say that I'm looking forward to this though...
 * Other than that, it's progressing nicely. Since the last update, 
 * most of my time had gone into moving from python to the C unity test
 * framework, with a quick pitstop at CFFI in python. 
 * I have concluded that there isn't a nice way to integrate this code into
 * a python based test framework, so I have gone with a C based test 
 * framwork instead. 
 * At the moment, adding, removing and printing the buffer works fine, 
 * those have static tests, so no randomized input yet. 
 * But I was able to squash some bugs there. Moreover, the tests for 
 * finding the deepest bracket, along with converting the list entries 
 * from char to int is working nicely. The choice of having duplicate 
 * lists for input buffer and solving buffer is needed so that we have
 * the input buffer to print. 
 *
 * March 25:
 * The expression solver is working for basic stuff like 123+456 and 
 * 123+456*789, and I'm working on depth increasing expressions. 
 * I have thought at bit about how operators and functions are to be 
 * handled, and I have set the normal bitwise operators to non-depth
 * increasing. 
 * I think the expression solver will handle ignoring non-increasing
 * non-number types, such as a comma, but I have not tested that yet. 
 * Moreover, I'm starting to think about what features makes sense. 
 * I want to leave it as open to expansions with new functionality as I can
 * in that I want the have any type of function in there. 
 * At the same time, I have been thinking about imaginary numbers and
 * variables, if that could be something that the calculator should support. 
 * I think imaginary numbers would be awesome. 
 * The difference in operators and custom functions are starting to 
 * be very very small, so I might as well remove the custom function option
 * and just have operators. Really the only difference I can see is if
 * a custom function should be dynamically linked at runtime. But I cannot 
 * see the need for that right now.. 
 * After some thought I removed the custom function, it's now merged into 
 * operators. 
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

/* -------- CALCULATOR CORE FUNCTIONS -------- */



/* Function initialize the calculator core. This will allocate the first list entry as well. 
   Args: 
   - pCalcCoreState: pointer to an allocated calculator core state
   Returns:
   - Status
   State:
   - Draft, testing
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

    // Set the allocation counter to 0
    pCalcCoreState->allocCounter = 0;

    // Set the result to 0 and solved to false
    pCalcCoreState->result = 0;
    pCalcCoreState->solved = false;

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
   - Draft, testing
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
   - Draft, testing
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
    		inputListEntry_t *pNext = (inputListEntry_t*)pListEntry->pNext;
    		free(pListEntry);
            pCalcCoreState->allocCounter--;
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
   - Draft, testing
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
    pCalcCoreState->allocCounter++;
	pNewListEntry->pFunEntry = NULL;

	// Add the input
	pNewListEntry->entry.c = inputChar;
    // Set the input subresult to 0
    pNewListEntry->entry.subresult = 0;
	if(charIsNumerical(pCalcCoreState->inputBase,inputChar)){
		pNewListEntry->entry.typeFlag = 
			CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_CHAR, DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
	} 
	else if( charIsOperator(inputChar) ){
		// Get the operator
		const operatorEntry_t *pOp = getOperator(inputChar);
		if(pOp->bIncDepth){
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_CHAR, DEPTH_CHANGE_INCREASE, INPUT_TYPE_OPERATOR);
		}
		else {
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_CHAR, DEPTH_CHANGE_KEEP, INPUT_TYPE_OPERATOR);
		}
		pNewListEntry->pFunEntry = (void*)pOp;

	}
	else if( charIsBracket(inputChar) ){
		if(inputChar == OPENING_BRACKET){
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_CHAR, DEPTH_CHANGE_INCREASE, INPUT_TYPE_EMPTY);
		}
		else {
			pNewListEntry->entry.typeFlag = 
				CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_CHAR, DEPTH_CHANGE_DECREASE, INPUT_TYPE_EMPTY);
		}
	}
	else{
		// Unknown input. Free and return
		if(pNewListEntry != NULL){
		  free(pNewListEntry);
          pCalcCoreState->allocCounter--;
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
		pCurrentListEntry->pNext = pNewListEntry;
		
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
   - Draft, in testing. 
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

    // If the current list entry pointer is NULL, 
    // then we have either added nothing, or we're at the 
    // head of the list. 
    if(pCurrentListEntry == NULL){
        // We cannot do anything, as we cannot remove anything! 
        return calc_funStatus_INPUT_LIST_NULL;
    }

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
    if(pCurrentListEntry->pPrevious == NULL){
        // If the previous pointer was NULL, this was the head of the
        // list. Align the entry point. 
        pCalcCoreState->pListEntrypoint = pCurrentListEntry->pNext;
    }
	
	free(pCurrentListEntry);
    pCalcCoreState->allocCounter--;

	return calc_funStatus_SUCCESS;
}

/* --------------------------------------------------------------
 * Function to find the deepest point within the buffer. 
 * Returns the pointer to the start and end of the deepest point. 
 * Works by increasing a counter when the depth increases, 
 * and for each consecutive depth increase updates the start 
 * pointer, until a depth decrease is found. 
 * It always returns the first deepest point. 
 * From that starting point, it then locates the next 
 * depth decrease, which would be the end. 
 * Input: Pointers to pointer of where to write the results. 
 * Moreover, the starting point is given by *ppStart. 
 *
 * Return values:
 * 0: successful
 * -1: Brackets not matched. 
 * Status: Tested, seems to work. 
 * Return the deepest point including operator and brackets. 
 * -------------------------------------------------------------- */
int findDeepestPoint(inputListEntry_t **ppStart, inputListEntry_t **ppEnd){
    // Counter to keep track of the current depth.
    int currentDepth = 0;
    int deepestDepth = 0;

    // Local variables to keep track of the start and end point. 
    // Set the start and end points to start and end of the buffer, 
    inputListEntry_t *pStart = *ppStart;
    inputListEntry_t *pEnd = *ppEnd;
    inputListEntry_t *pIter = *ppStart;

    // Loop until the end of the list has been found
    while(pIter != NULL){
        if(GET_DEPTH_FLAG(pIter->entry.typeFlag) == DEPTH_CHANGE_INCREASE){
            // Increase in depth. Increase the current depth
            currentDepth++;
            
            if(currentDepth > deepestDepth){
                // This is the new deepest point
                deepestDepth = currentDepth;
                pStart = pIter;
            }
        }
        else if(GET_DEPTH_FLAG(pIter->entry.typeFlag) == DEPTH_CHANGE_DECREASE){
            // Decrease in depth
            currentDepth--;
        }
        pIter = pIter->pNext;
    }

    // If the depth doesn't match up, this cannot be solved. 
    if(currentDepth != 0){
        return -1;
    }

    // Set the pointer to the end to the pointer to the start
    pEnd = pStart;
    // ... and loop through to the next decrease, or end of the list. 
    while(pEnd != NULL){
        if(GET_DEPTH_FLAG(pEnd->entry.typeFlag) == DEPTH_CHANGE_DECREASE){
            break;
        }
        pEnd = pEnd->pNext;
    }

    // Record the values and return
    *ppStart = pStart;
    *ppEnd = pEnd;
    return 0;
}

int charToInt(char c){
    if( (c >= '0') && (c <= '9') ){
        return (int)(c-'0');
    }
    if( (c >= 'a') && (c <= 'f') ){
        return (int)(c-'a'+10);
    }
}

/* --------------------------------------------------------------
 * Function to copy the input list and convert chars into 
 * numbers. 
 * State: Tested quickly with simple output. 
 * -------------------------------------------------------------- */
int copyAndConvertList(calcCoreState_t* pCalcCoreState, inputListEntry_t **ppSolverListStart){
    inputListEntry_t *pCurrentListEntry = pCalcCoreState->pListEntrypoint;
    // If no input list, simply return NULL
    if(pCurrentListEntry == NULL){
        return calc_funStatus_INPUT_LIST_NULL;
    }

    
    inputListEntry_t *pNewListEntry = NULL;
    inputListEntry_t *pPreviousListEntry = NULL;
    // Loop through the input list and allocate new 
    // instances. 
    while(pCurrentListEntry != NULL){

        // Allocate a new entry
        pNewListEntry = malloc(sizeof(inputListEntry_t));
        if(pNewListEntry == NULL){
            return calc_funStatus_ALLOCATE_ERROR;
        }
        pCalcCoreState->allocCounter++;
        // Copy all parameters over
        memcpy(pNewListEntry, pCurrentListEntry, sizeof(inputListEntry_t));

        // Check if this is the start of the list, in which case save
        // the parameter. 
        if(pCurrentListEntry == pCalcCoreState->pListEntrypoint){
            *ppSolverListStart = pNewListEntry;
        }
        if(GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) == INPUT_TYPE_NUMBER){
            // If the current entry is numerical, aggregate this
            // until the entry is either NULL or not numerical
            pNewListEntry->entry.typeFlag = CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_INT, DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
            pNewListEntry->entry.subresult = 0;
            while(GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) == INPUT_TYPE_NUMBER){
                // Aggregate the input based on the input base
                if(pCurrentListEntry->inputBase == inputBase_DEC){
                    pNewListEntry->entry.subresult *= 10;
                }
                if(pCurrentListEntry->inputBase == inputBase_HEX){
                    pNewListEntry->entry.subresult *= 16;
                }
                if(pCurrentListEntry->inputBase == inputBase_BIN){
                    pNewListEntry->entry.subresult *= 2;
                }
                pNewListEntry->entry.subresult += charToInt(pCurrentListEntry->entry.c);
                //printf("Input: %c, output: %i\r\n", pCurrentListEntry->entry.c, pNewListEntry->entry.subresult);
                pCurrentListEntry = pCurrentListEntry->pNext;

                if(pCurrentListEntry == NULL){
                    break;
                }
            }
        }
        else {
            // Not a number input, therefore move on to the next entry directly
            pCurrentListEntry = pCurrentListEntry->pNext;
        }
        
        // Set the next and previous pointer of new entry
        pNewListEntry->pNext = NULL;
        pNewListEntry->pPrevious = pPreviousListEntry;
        if(pPreviousListEntry != NULL){
            pPreviousListEntry->pNext = pNewListEntry;
        }
        pPreviousListEntry = pNewListEntry;
    }
    return calc_funStatus_SUCCESS;
}

/* --------------------------------------------------------------
 * Function to solve a single expression. 
 * This function parses an expression in the format of:
 * [bracket/function/operator/none][expression][bracket/none]
 * 
 * It also check that the expression is in a valid form. 
 * It takes the start, end and pointer to results as input, 
 * and return 0 if successful and -1 if expression is valid. 
 *
 * The solving procedure is as follows: 
 * 1. Solve for all multiplications
 * 2. Solve for all divisions
 * 3. Solve for any other operators that does not increase depth, 
 *    e.g. '>>' or '<<'.
 * 3. Solve for all additions
 * 4. Solve for all subtractions
 * 5. Solve the outer function if any.
 * TBD: the current way this is handled is that 
 * logic operators are on the form of OP(arg1, arg2). 
 * This is nice if we want to extend to N arguments. 
 *
 * WARNING! THIS WILL MESS WITH THE LIST! 
 * 
 * State: not complete, not tested. 
 *        But the solver should now be able to figure out 123 and 123+456
 * -------------------------------------------------------------- */
int solveExpression(calcCoreState_t* pCalcCoreState, inputListEntry_t **ppResult, inputListEntry_t *pExprStart, inputListEntry_t *pExprEnd){
    // Loop until there isn't anything between the start and end pointer.

    // Sanity check the input
    if(pExprStart == NULL){
        printf("ERROR! Input pointer is NULL\r\n");
        return -3;
    }
    inputListEntry_t *pStart = pExprStart;
    inputListEntry_t *pEnd = pExprEnd;
    bool outerExpression = false;
    if(GET_INPUT_TYPE(pStart->entry.typeFlag) != INPUT_TYPE_NUMBER){
        printf("Expression starts with depth increase\r\n");
        // If the first entry isn't a number, it's either a bracket, 
        // a depth increasing operator or a custom function. 
        // Double check that the last entry is the same type. 
        if(GET_INPUT_TYPE(pEnd->entry.typeFlag) == INPUT_TYPE_NUMBER){
            printf("ERROR! Unmatched brackets\r\n");
            return -1;
        }
        if(pStart->pNext == pEnd){
            // Empty inside brackets, return error. 
            printf("ERROR! Empty expression within brackets\r\n");
            return -1;
        }
        // Move the start and end so that to remove the 
        // operator/function/bracket, and check if the remaining list
        // is emtpy
        pStart = pExprStart->pNext;
        pEnd = pExprEnd->pPrevious;
        if(pStart == NULL){
            printf("ERROR. \r\n");
        }
        outerExpression = true;
    }
    // While the start of the buffers next entry isn't pointing at the end
    // continue solving. 
    // Note: If only numbers entered, this will be skipped, as 
    // pStart->pNext = pEnd. 
    // 
    bool operatorFound = true;
    inputListEntry_t *pHigestOp = NULL;
    while(pStart->pNext != pEnd){
        printf("IN THE LOOP\r\n");
        // Solve between pStart and pEnd. 
        // Start by finding the highest priority operator
        // Priority is ascending, with 0 being the highest priority. 
        inputListEntry_t *pCurrentListEntry = pStart;
        pHigestOp = NULL;
        uint8_t highestPriority = 255;
        while( (pCurrentListEntry != pEnd) && (pCurrentListEntry != NULL) ){
            printf("LOOKING FOR THE HIGHEST ORDER OPERATOR\r\n");
            if(GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) == INPUT_TYPE_OPERATOR){
                // Get the operator solving priority
                printf("CURRENT INPUT IS OPERATOR\r\n");
                if(pCurrentListEntry->pFunEntry == NULL){
                    // Thas been as error, the function pointer for an operator
                    // should always exist! 
                    printf("ERROR! Function pointer is NULL\r\n");
                    return -2;
                }
                uint8_t currentPriority = ((operatorEntry_t*)(pCurrentListEntry->pFunEntry))->solvPrio;
                if(currentPriority < highestPriority){
                    pHigestOp = pCurrentListEntry;
                    highestPriority = currentPriority;
                }
            }
            else if (GET_INPUT_TYPE(pCurrentListEntry->entry.typeFlag) == INPUT_TYPE_EMPTY){
                // TODO. 
                // This should for example be a comma sign. 
            }
            printf("Current list entry: %i\r\n", (uint32_t)pCurrentListEntry);
            pCurrentListEntry = pCurrentListEntry->pNext;
            
        }

        // Sanity check the result of the search. 
        if(pHigestOp == NULL){
            printf("Highest order operator not found!\n");
            operatorFound = false;
            break;
        }
        printf("Highest operator: %s\r\n", ((operatorEntry_t*)(pHigestOp->pFunEntry))->opString);
        // The operation that we should now perform is on
        // the current operator and the entries before and after. 
        // Ensure that these exist, and are indeed numbers. 
        inputListEntry_t *pPrev = pHigestOp->pPrevious;
        inputListEntry_t *pNext = pHigestOp->pNext;
        if( (pNext == NULL) || (pPrev == NULL) ){
            printf("ERROR: Pointer(s) before and after operators are NULL\n");
            return -5;
        }
        if( (GET_INPUT_TYPE(pNext->entry.typeFlag) != INPUT_TYPE_NUMBER) || 
            (GET_INPUT_TYPE(pPrev->entry.typeFlag) != INPUT_TYPE_NUMBER) ){
            printf("ERROR: %c and %c surrounding %c are not numbers!\n",
                pNext->entry.c, pPrev->entry.c, pHigestOp->entry.c);
            return -6;
        }
        if( (GET_SUBRESULT_TYPE(pNext->entry.typeFlag) != SUBRESULT_TYPE_INT) || 
            (GET_SUBRESULT_TYPE(pPrev->entry.typeFlag) != SUBRESULT_TYPE_INT) ){
            printf("ERROR: %c and %c are not resolved integers!\n",
                pNext->entry.c, pPrev->entry.c);
            return -7;
        }

        // Finally, we are ready to solve this subresult!
        // This will write the subresult to the operators
        // subresult field, set the subresult type to int and the 
        // input type to number. 
        printf("Solving %i %s %i\n",pPrev->entry.subresult, ((operatorEntry_t*)(pHigestOp->pFunEntry))->opString, pNext->entry.subresult);

        SUBRESULT(*pFun)(SUBRESULT, SUBRESULT) = (math_operator*)(((operatorEntry_t*)(pHigestOp->pFunEntry))->pFun);
        pHigestOp->entry.subresult = (*pFun)(pPrev->entry.subresult, pNext->entry.subresult);
        printf("Subresult = %i \r\n", pHigestOp->entry.subresult);
        pHigestOp->entry.typeFlag = CONSTRUCT_TYPEFLAG(SUBRESULT_TYPE_INT, DEPTH_CHANGE_KEEP, INPUT_TYPE_NUMBER);
        
        // The subresult is now stored in the operators entry, so we have to 
        // remove the two number on either side, and replace it with 
        // the operator only, as it's been solved! 
        pHigestOp->pPrevious = pPrev->pPrevious;
        pHigestOp->pNext = pNext->pNext;
        if(pPrev->pPrevious != NULL){
            ((inputListEntry_t *)(pPrev->pPrevious))->pNext = pHigestOp;
        }
        if(pNext->pNext != NULL){
            ((inputListEntry_t *)(pNext->pNext))->pPrevious = pHigestOp;
        }
        // Free the previous and next entries
        free(pPrev);
        free(pNext);
        // Start next round of looking on this entry. 
        pStart = pHigestOp;
        pCalcCoreState->allocCounter -= 2;
        // It should just magically continue from here
    }
    // if applicable, solve the outer most operator as well. 
    if(outerExpression){
        printf("Need to solve outer function as well!\n");
        // TODO. 
        pStart = pExprStart;
        pEnd = pExprEnd;

        // The operator is now at pStart. Check that it isn't just a bracket though
        if(GET_INPUT_TYPE(pStart->entry.typeFlag) == INPUT_TYPE_OPERATOR){
            // This is a depth increasing operator.
            
            // Save the results to pStart.
        }

        // Free everything after pStart until the end. 
        while(pStart->pNext != pExprEnd){
            inputListEntry_t *pTmp = pStart->pNext;
            pStart->pNext = pTmp->pNext;
            free(pTmp);
            pCalcCoreState->allocCounter--;
        }
        // Free the end and repoint. 
        pStart->pNext = pExprEnd->pNext;
        if(pExprEnd->pNext != NULL){
            ((inputListEntry_t *)(pExprEnd->pNext))->pPrevious = pStart;
        }
        free(pExprEnd);
        pCalcCoreState->allocCounter--;
    }
    *ppResult = pStart;
    return 0;
}

/* --------------------------------------------------------------
 * Solver. This is one of the core function of the calculator.
 * 
 * This goes through the buffer, and starts solve parts of the 
 * buffer from the deepest point. 
 *
 * The solver will solve using int, if there isn't a float 
 * present. 
 *
 * There are several ways to solve this. Either solving the 
 * multiplications first, then functions and operators requiring
 * depth increase, then the rest. However, since there can be 
 * nested brackets, the best way might be to solve it by:
 * 1. Go through the buffer and locate the deepest calculation
 * 2. Partially solve the inside of the deepest brackets. 
 * 3. Solve the function outside of the brackets, if any. 
 * 4. Repeat steps 1-3 until there are no depth increases left
 * 5. Finally solve for each left and return
 * -------------------------------------------------------------- */
calc_funStatus_t calc_solver(calcCoreState_t* pCalcCoreState){
    pCalcCoreState->solved = false;

    // Local variables to keep track while the solver is
    // at work. Should be copied to core state when done. 
    RESULT result = 0;
    bool solved = false;
    int depth = 0;
    bool overflow = false;

    // Copy the list and convert the numbers from
    // chars into actual ints (or floats if that's the case)
    inputListEntry_t *pSolverListStart = NULL;
    copyAndConvertList(pCalcCoreState, &pSolverListStart);

    // Loop through and find the deepest point of the buffer
    inputListEntry_t *pStart = pSolverListStart;
    inputListEntry_t *pEnd = NULL;

    // Do a NULL check on the start:
    if(pStart == NULL){
        // No list to solve for. Simply return
        printf("ERROR: No input list\r\n");
        return calc_funStatus_INPUT_LIST_NULL;
    }

    
    // Loop until the pointers are back at start and end
    while(!solved){
        printf("HERE\n");
        // Find the deepest calculation
        if(findDeepestPoint(&pStart, &pEnd) < 0){
            // There was an error in finding the bracket.
            printf("Could not find the deepest point. \r\n"); 
            return calc_funStatus_SOLVE_INCOMPLETE;
        }
        // The deepest calculation is now between pStart and pEnd. 
        // This is in the form of:
        // [bracket/function/operator/none][expression][bracket/none]
        // The expression can consist of however many operators, 
        // but no depth increasing ones. In the case of the last 
        // expression, there might be no brackets or expressions. 

        // The numbers in the expression is always integers, 
        // since the input list has been converted. 
        // It's now a case of continously shrinking that list, 
        // solving each expression at a time
        // 
        // Solving the expression will shrink the list, 
        // and it's not certain that the start of the solver 
        // list isn't freed. Therefore the expression solver
        // needs to return the list entry where it put the result
        inputListEntry_t *pResult = NULL;
        if(solveExpression(pCalcCoreState, &pResult, pStart, pEnd) < 0){
            printf("ERROR: Could not solve expression\r\n");
            return calc_funStatus_SOLVE_INCOMPLETE;
        }
        if(pResult == NULL){
            printf("No result written, but expression solver returned OK. \n");
            return calc_funStatus_SOLVE_INCOMPLETE;
        }
        if((pResult->pNext == NULL) && (pResult->pPrevious == NULL)){
            printf("SOLVED! Result is %i\r\n", pResult->entry.subresult);
            solved = true;
            break;
        }
    }
    
    // Free the temporary list
    while(pSolverListStart != NULL){
        inputListEntry_t *pNext = pSolverListStart->pNext;
        free(pSolverListStart);
        pSolverListStart = pNext;
        pCalcCoreState->allocCounter--;
    }
    return calc_funStatus_SUCCESS;
}

/* --------------------------------------------------------------
 * Function to print the list entries. 
 * This function relies on the list being in good shape.  
 * Args: 
 * - pCalcCoreState: pointer to calculator core state
 * - pString: pointer to string which to print the buffer
 * - stringLen: maximum number of characters to write to pString
 *  Returns:
 *  - state of the function. 
 *  State:
 * - Draft
 * -------------------------------------------------------------- */
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


/* --------------------------------------------
 * ------------- FUNCTION WRAPPERS ------------
 * ---These are exposed for testing purposes---
 * --------------------------------------------*/

int wrap_findDeepestPoint(inputListEntry_t **ppStart, inputListEntry_t **ppEnd){
    return findDeepestPoint(ppStart, ppEnd);
}