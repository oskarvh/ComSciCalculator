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

// This function will run before each test. 
void setUp(void) {
    // set stuff up here
}

// This function will run after each test. 
void tearDown(void) {
    // clean stuff up here
}


// Define the test parameters
#define NUM_ADD_INPUT_TESTS 4
testParams_t addInputSuiteTestParams[NUM_ADD_INPUT_TESTS] = {
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = inputBase_DEC
    },
    {
        .pInputString = "123+\0",
        .pCursor = {0,0,0,2},
        .pExpectedString = "1+23\0",
        .pOutputString = {0},
        .inputBase = inputBase_DEC
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(123)\0",
        .pOutputString = {0},
        .inputBase = inputBase_DEC
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0,0,0,0,4,0,0,0,0},
        .pExpectedString = "NAND(123+123)\0",
        .pOutputString = {0},
        .inputBase = inputBase_DEC
    }
};

void test_addInputSuite(void){
    /* -------------------------------------------
     * Simple add input. 
     * -------------------------------------------*/
    testSetup_t testSetup;
    
    calcCoreState_t calcCore;

    for(int i = 0 ; i < NUM_ADD_INPUT_TESTS ; i++){
        addInputSuiteTestParams[i];
        setupTestStruct(&calcCore, &testSetup, &addInputSuiteTestParams[i]);
        calcCoreAddInput(&testSetup);
        TEST_ASSERT_EQUAL_STRING(
            addInputSuiteTestParams[i].pExpectedString,
            testSetup.pOutputString);
        teardownTestStruct(&testSetup);
    }
    
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_addInputSuite);
    return UNITY_END();
}