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
    testSetup_t *pTestSetup, 
    testParams_t *pTestParams){

    if(pCoreState == NULL){
        printf("[ERROR]: setupTestStruct: pCoreState is NULL\r\n");
        TEST_FAIL();
    }
    if(pTestSetup == NULL){
        printf("[ERROR]: setupTestStruct: pTestSetup is NULL\r\n");
        TEST_FAIL();
    }
    if(pTestParams == NULL){
        printf("[ERROR]: setupTestStruct: pTestParams is NULL\r\n");
        TEST_FAIL();
    }

    pTestSetup->pCoreState = pCoreState;
    pTestSetup->pCoreState->inputBase = pTestSetup->inputBase;
    if(calc_coreInit(pTestSetup->pCoreState) != calc_funStatus_SUCCESS){
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

    // Set the pointers to the input/output strings, and cursor
    pTestSetup->pInputString = pTestParams->pInputString;
    pTestSetup->pOutputString = pTestParams->pOutputString;
    pTestSetup->pCursor = pTestParams->pCursor;
    pTestSetup->pExpectedString = pTestParams->pExpectedString;
    pTestSetup->inputBase = pTestParams->inputBase;
}

/* -------------------------------------------------
 * Function to tear down the test structure and 
 * variables used within.
 * ------------------------------------------------- */
void teardownTestStruct(testSetup_t *pTestSetup){
    
    if(calc_coreBufferTeardown(pTestSetup->pCoreState) != calc_funStatus_SUCCESS){
        printf("[ERROR]: Could not tear down calculator core state!\r\n");
        // If this is the case, we cannot proceed. 
        TEST_FAIL();
    }

    // Set the pointer to NULL to force the next test to initialize them
    pTestSetup->pInputString = NULL;
    pTestSetup->pOutputString = NULL;
    pTestSetup->pCursor = NULL;
    pTestSetup->pExpectedString = NULL;
}


/* -------------------------------------------------
 * Function to add input to the calculator core. 
 * Core should already be initialized. Assumes
 * that the length of the arrays have been set correct. 
 * ------------------------------------------------- */
void calcCoreAddInput(testSetup_t *pTestSetup){
    // Extract the variables from the test setup struct
    calcCoreState_t *pCoreState = pTestSetup->pCoreState;
    char *pInputString = pTestSetup->pInputString;
    char *pOutputString = pTestSetup->pOutputString;
    int *pCursor = pTestSetup->pCursor;
    
    // Set the input base
    pCoreState->inputBase = pTestSetup->inputBase;

    // Loop through the input string
    int i = 0;
    uint8_t status = calc_funStatus_SUCCESS;
    while(*pInputString != '\0'){
        if(pCursor != NULL){
            pCoreState->cursorPosition = pCursor[i++];
        }
        calc_addInput(pCoreState, *pInputString);
        if(status != calc_funStatus_SUCCESS){
            printf("Could not add input \r\n");
            printf("Status = %d\n", status);
        }
        pInputString++;
    }

    // Get the output, using the comSciCalc library function
    status = calc_printBuffer(pCoreState, pOutputString, MAX_STR_LEN);
    if(status != calc_funStatus_SUCCESS){
        printf("Could not print \r\n");
        printf("Status = %d\n", status);
    }
}