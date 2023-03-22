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

/* -------------------------------------------
 * Testing adding and removing input
 * -------------------------------------------*/
testParams_t addInputSuiteTestParams[] = {
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "0x123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_HEX},
    },
    {
        .pInputString = "123+\0",
        .pCursor = {0,0,0,2},
        .pExpectedString = "1+23\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(123)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0,0,0,0,4,0,0,0,0},
        .pExpectedString = "NAND(123+123)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123\b\0",
        .pCursor = {0,0,0,0},
        .pExpectedString = "12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123\b\0",
        .pCursor = {0,0,0,1},
        .pExpectedString = "13\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0,0,0,1},
        .pExpectedString = "13\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0,0,0,2},
        .pExpectedString = "NAND(3\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    }
};
void test_addRemoveInput(void){   
    calcCoreState_t calcCore;
    int numTests = sizeof(addInputSuiteTestParams)/sizeof(addInputSuiteTestParams[0]);
    for(int i = 0 ; i < numTests ; i++){
        //printf("Test %i\r\n", i);
        setupTestStruct(&calcCore, &addInputSuiteTestParams[i]);
        calcCoreAddInput(&calcCore, &addInputSuiteTestParams[i]);
        printf("------------------------------------------------\r\n");
        printf("Got     : %s \r\nExpected: %s\r\n", 
                addInputSuiteTestParams[i].pOutputString,
                addInputSuiteTestParams[i].pExpectedString);
        //TEST_ASSERT_EQUAL_STRING(
        //    addInputSuiteTestParams[i].pExpectedString,
        //    addInputSuiteTestParams[i].pOutputString);
        teardownTestStruct(&calcCore);
        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        printf("------------------------------------------------\r\n\r\n");
        TEST_ASSERT_EQUAL_INT(
            0,
            calcCore.allocCounter);
    }
}


int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_addRemoveInput);
    return UNITY_END();
}