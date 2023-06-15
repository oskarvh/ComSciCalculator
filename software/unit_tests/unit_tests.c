/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Wrapper to glue the computer scientist calculator (comsci) C code to
 * to a Python module, using the python.h library. 
 * 
 * The python code can either act as a test harness, or host a GUI
 * but with the codebase for the calculator core being common with the
 * embedded system. 
 * 
 * 
 * ------------------- DEV LOGS -------------------
 * March 20: 
 * Starting using the unity test framework instead. 
 */

/* ----------------- DEFINES ----------------- */

/* ----------------- HEADERS ----------------- */

// Unity test framework. 
#include "../Unity/src/unity.h"

// Standard lib
#include <string.h>

// Unit test header
#include "unit_tests.h"

/* -------------------------------------------------
 * Function to setup the test based on parameters
 * ------------------------------------------------- */
void setupTestStruct(calcCoreState_t *pCoreState, 
    testParams_t *pTestParams){

    if(pCoreState == NULL){
        printf("[ERROR]: setupTestStruct: pCoreState is NULL\r\n");
        TEST_FAIL();
    }
    if(pTestParams == NULL){
        printf("[ERROR]: setupTestStruct: pTestParams is NULL\r\n");
        TEST_FAIL();
    }

    if(calc_coreInit(pCoreState) != calc_funStatus_SUCCESS){
        printf("[ERROR]: Could not intialize calculator core state!\r\n");
        // If this is the case, we cannot proceed. 
        TEST_FAIL();
    }

    if(pTestParams->pInputString == NULL){
        printf("[ERROR]: input string pointer is NULL");
        TEST_FAIL();
    }

    if(pTestParams->pOutputString == NULL){
        printf("[ERROR]: output string pointer is NULL");
        TEST_FAIL();
    }

    if(pTestParams->pExpectedString == NULL){
        printf("[ERROR]: expected string pointer is NULL");
        TEST_FAIL();
    }
}


/* -------------------------------------------------
 * Function to tear down the test structure and 
 * variables used within.
 * ------------------------------------------------- */
void teardownTestStruct(calcCoreState_t *pCoreState){
    if(calc_coreBufferTeardown(pCoreState) != calc_funStatus_SUCCESS){
        printf("[ERROR]: Could not tear down calculator core state!\r\n");
        // If this is the case, we cannot proceed. 
        TEST_FAIL();
    }
}


/* -------------------------------------------------
 * Function to add input to the calculator core. 
 * Core should already be initialized. Assumes
 * that the length of the arrays have been set correct. 
 * ------------------------------------------------- */
void calcCoreAddInput(calcCoreState_t *pCoreState, testParams_t *pTestParams){
    // Extract the variables from the test setup struct
    char *pInputChar = pTestParams->pInputString;
    int *pCursor = pTestParams->pCursor;

    // Loop through the input string
    int i = 0;
    uint8_t status = calc_funStatus_SUCCESS;
    while(*pInputChar != '\0'){
        if(pCursor != NULL){
            pCoreState->cursorPosition = pCursor[i];
        }
        // Set the input base
        pCoreState->numberFormat.inputBase = pTestParams->inputBase[i];

        if(*pInputChar == '\b'){
            // This is the symbol for backspace. 
            // therefore delete the symbol at current
            // cursor. 
            status = calc_removeInput(pCoreState);
        }
        else if(*pInputChar == 'i'){
            printf("Changing input base\r\n");
            calc_updateBase(pCoreState);
        }
        else{
            printf("Adding %c with base %i", pInputChar, pTestParams->inputBase[i]);
            status = calc_addInput(pCoreState, *pInputChar);
        }
        if(status != calc_funStatus_SUCCESS){
            printf("Could not add input \r\n");
            printf("Status = %d\n", status);
        }
        pInputChar++;
        i++;
    }
}

/* -------------------------------------------------
 * Function to print the test buffer. 
 * ------------------------------------------------- */
void calcCoreGetBuffer(calcCoreState_t *pCoreState, testParams_t *pTestParams){
    // Get the output, using the comSciCalc library function
    char *pOutputString = pTestParams->pOutputString;
    uint8_t status = calc_printBuffer(pCoreState, pOutputString, MAX_STR_LEN, NULL);
    if(status != calc_funStatus_SUCCESS){
        printf("Could not print \r\n");
        printf("Status = %d\n", status);
    }
}