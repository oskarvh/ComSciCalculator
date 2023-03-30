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


static bool verbose;
// This function will run before each test. 
void setUp(void) {
    // set stuff up here
}

// This function will run after each test. 
void tearDown(void) {
    // clean stuff up here
}

/* ----------------------------------------------------------------
 * Testing adding and removing input. 
 * Note, \b is removing. 
 * Usage: The input string corresponds to the
 * characters sent to the comSciCalc core, 
 * where \b is backslash (i.e. remove). 
 * Each entry has a corresponding cursor value
 * and input base value. 
 * The output is recorded in the pOutputString
 * in the format that the string would be 
 * displayed on screen. 
 * That is then compared to the expected string. 
 * ----------------------------------------------------------------*/
testParams_t addInputTestParams[] = {
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "1234\b\0",
        .pCursor = {0,0,0,0,0},
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
        .pInputString = "123+n123\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0,0,0,0,4,0,0,0,0},
        .pExpectedString = "NAND123+123)\0",
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
        .pExpectedString = "NAND3\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0,0,0,3},
        .pExpectedString = "1NAND3\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n(456*(12+45))\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(456*(12+45))\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
};
void test_addRemoveInput(void){   
    calcCoreState_t calcCore;
    int numTests = sizeof(addInputTestParams)/sizeof(addInputTestParams[0]);
    for(int i = 0 ; i < numTests ; i++){
        //printf("Test %i\r\n", i);
        setupTestStruct(&calcCore, &addInputTestParams[i]);
        calcCoreAddInput(&calcCore, &addInputTestParams[i]);
        calcCoreGetBuffer(&calcCore, &addInputTestParams[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Got     : %s \r\nExpected: %s\r\n", 
                    addInputTestParams[i].pOutputString,
                    addInputTestParams[i].pExpectedString);
        }
        TEST_ASSERT_EQUAL_STRING(
            addInputTestParams[i].pExpectedString,
            addInputTestParams[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
        if(verbose){
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}


/* ----------------------------------------------------------------
 * Test for the function to find the deepest point within 
 * a list. 
 * ----------------------------------------------------------------*/
testParams_t findDeepestPointTestParams[] = {
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC}, // GCC specific initializer
    },
    {
        .pInputString = "123(456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123(456\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123(456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123n456\0",
        .pCursor = {0},
        .pExpectedString = "123NAND456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n456*(12+45))\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND456*(12+45))\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "n123+456)\0",
        .pCursor = {0},
        .pExpectedString = "NAND123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
};
void test_findingDeepestPoint(void){
    calcCoreState_t calcCore;
    
    // Loop through all tests
    int numTests = sizeof(findDeepestPointTestParams)/sizeof(findDeepestPointTestParams[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &findDeepestPointTestParams[i]);
        calcCoreAddInput(&calcCore, &findDeepestPointTestParams[i]);

        // Instead of printing this directly, find the deepest point
        inputListEntry_t *pStart = calcCore.pListEntrypoint;
        inputListEntry_t *pEnd = NULL;
        int status = wrap_findDeepestPoint(&pStart, &pEnd);

        // Temporarily modify the list structure
        inputListEntry_t *pOrigEntrypoint = calcCore.pListEntrypoint;
        inputListEntry_t *pStartPrev = NULL;
        if(pStart != NULL){
            pStartPrev = pStart->pPrevious;
            pStart->pPrevious = NULL;
        }
        inputListEntry_t *pEndNext = NULL;
        if(pEnd != NULL) {
            pEndNext = pEnd->pNext;
            pEnd->pNext = NULL;
        }

        calcCore.pListEntrypoint = pStart;
        // IMPORTANT: If we abort between here, we'll leak memory

        calcCoreGetBuffer(&calcCore, &findDeepestPointTestParams[i]);

        // Reset the list again!  
        calcCore.pListEntrypoint = pOrigEntrypoint;
        if(pStart != NULL){
            pStart->pPrevious = pStartPrev;
        }
        if(pEnd != NULL) {
            pEnd->pNext = pEndNext;
        }
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Got     : %s \r\nExpected: %s\r\n", 
                    findDeepestPointTestParams[i].pOutputString,
                    findDeepestPointTestParams[i].pExpectedString);
        }
        TEST_ASSERT_EQUAL_STRING(
            findDeepestPointTestParams[i].pExpectedString,
            findDeepestPointTestParams[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
        if(verbose){
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

/* ----------------------------------------------------------------
 * Test for the function to find the deepest point within 
 * a list. 
 * ----------------------------------------------------------------*/
testParams_t solverParams[] = {
    {
        .pInputString = "()\0",
        .pCursor = {0,0,0},
        .pExpectedString = "()\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0,
    },
    {
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123,
    },

    {
        .pInputString = "f2\0",
        .pCursor = {0,0,0},
        .pExpectedString = "0xf2\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_HEX},
        .expectedResult = 242,
    },
    {
        .pInputString = "1100\0",
        .pCursor = {0,0,0},
        .pExpectedString = "0b1100\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_BIN},
        .expectedResult = 12,
    },
    {
        .pInputString = "123+456\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 579,
    },
    {
        .pInputString = "123+456+789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456+789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 1368,
    },
    {
        .pInputString = "123*456+789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123*456+789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 56877,
    },
    {
        .pInputString = "(123+456+789)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(123+456+789)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 1368,
    },
    {
        .pInputString = "~123,456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT(123,456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0,
    },
    {
        .pInputString = "s12,34,56,78)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "SUM(12,34,56,78)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 180,
    },
};
void test_solver(void){   
    calcCoreState_t calcCore;
    int numTests = sizeof(solverParams)/sizeof(solverParams[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &solverParams[i]);
        calcCoreAddInput(&calcCore, &solverParams[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &solverParams[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected result: %i \r\nReturned result: %i \r\n", 
                    solverParams[i].pExpectedString,
                    solverParams[i].pOutputString,
                    solverParams[i].expectedResult,
                    calcCore.result
                    );
        }
        TEST_ASSERT_EQUAL_STRING(
            solverParams[i].pExpectedString,
            solverParams[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_UINT_MESSAGE(solverParams[i].expectedResult, calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter, "Leaky memory!");
        if(calcCore.allocCounter != 0){
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if(verbose){
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

// Need to write more tests. 
// Need to cover: 
// 1. hex, dec and bin input. 
// 2. Varied input
// 3. Testcase for each calc function (candidate for random testing)
// 4. Negative testing : Test negative outcome.
// 4a. Try to add illegal chars. 
// 4b. Try to calculate functions with wrong arguments. 
// 4c. Parenhesis mismatch
/* ----------------------------------------------------------------
 * Solvable equations (positive testing)
 * ----------------------------------------------------------------*/
testParams_t solvable_params[] = {
    {
        .pInputString = "123+456\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456,
    },
    {
        .pInputString = "(123+456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456,
    },
    {
        .pInputString = "123+456*789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456*789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456*789,
    },
    {
        .pInputString = "(123+456*789)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(123+456*789)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456*789,
    },
    {
        .pInputString = "(123+456)*789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(123+456)*789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = (123+456)*789,
    },
    {
        .pInputString = "123+(456+789)/1011\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+(456+789)/1011\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+(456+789)/1011,
    },
    {
        .pInputString = "~123)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT(123)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = ~123,
    },
    {
        .pInputString = "~123+456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = ~(123+456),
    },
    {
        .pInputString = "789+~123+456)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "789+NOT(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 789+~(123+456),
    },
    {
        .pInputString = "~123+456)-789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT(123+456)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = ~(123+456)-789,
    },
    {
        .pInputString = "1011+~123+456)-789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "1011+NOT(123+456)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 1011+~(123+456)-789,
    },
    {
        .pInputString = "1011+~123+456+1213)-789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "1011+NOT(123+456+1213)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 1011+~(123+456+1213)-789,
    },
    {
        .pInputString = "1011+s123,456,1213)-789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "1011+SUM(123,456,1213)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 1011+(123+456+1213)-789,
    },
};
void test_solvable_solution(void){
    calcCoreState_t calcCore;
    int numTests = sizeof(solvable_params)/sizeof(solvable_params[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &solvable_params[i]);
        calcCoreAddInput(&calcCore, &solvable_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &solvable_params[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected result: %i \r\nReturned result: %i \r\n", 
                    solvable_params[i].pExpectedString,
                    solvable_params[i].pOutputString,
                    solvable_params[i].expectedResult,
                    calcCore.result
                    );
        }
        TEST_ASSERT_EQUAL_STRING(
            solvable_params[i].pExpectedString,
            solvable_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(solvable_params[i].expectedResult, calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter, "Leaky memory!");
        if(calcCore.allocCounter != 0){
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if(verbose){
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}


testParams_t unsolvable_params[] = {
    {
        .pInputString = "123+\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "~123+\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT(123+\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "~)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "NOT()\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "()\0",
        .pCursor = {0,0,0},
        .pExpectedString = "()\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "(\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = ")\0",
        .pCursor = {0,0,0},
        .pExpectedString = ")\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s*,+)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "SUM(*,+)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s,)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "SUM(,)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s,1)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "SUM(,1)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
};
void test_unsolvable_solution(void){
    calcCoreState_t calcCore;
    int numTests = sizeof(unsolvable_params)/sizeof(unsolvable_params[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &unsolvable_params[i]);
        calcCoreAddInput(&calcCore, &unsolvable_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &unsolvable_params[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected result: %i \r\nReturned result: %i \r\n", 
                    unsolvable_params[i].pExpectedString,
                    unsolvable_params[i].pOutputString,
                    unsolvable_params[i].expectedResult,
                    calcCore.result
                    );
        }
        TEST_ASSERT_EQUAL_STRING(
            unsolvable_params[i].pExpectedString,
            unsolvable_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(unsolvable_params[i].expectedResult, calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter, "Leaky memory!");
        if(calcCore.allocCounter != 0){
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if(verbose){
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

/* ----------------------------------------------------------------
 * Main. Only starts the tests. 
 * ----------------------------------------------------------------*/
int main(void)
{
    verbose = true;
    UNITY_BEGIN();
    //RUN_TEST(test_addRemoveInput);
    //RUN_TEST(test_findingDeepestPoint);
    //RUN_TEST(test_solver);
    RUN_TEST(test_solvable_solution);
    RUN_TEST(test_unsolvable_solution);
    return UNITY_END();
}