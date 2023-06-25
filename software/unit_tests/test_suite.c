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
        .pInputString = "1010\0",
        .pCursor = {0,0,0},
        .pExpectedString = "0b1010\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_BIN},
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
    {
        .pInputString = "1234\0",
        .pCursor = {0,0,0,5},
        .pExpectedString = "4123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "1\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_NONE},
    },
    {
        .pInputString = ".\0",
        .pCursor = {0},
        .pExpectedString = ".\0",
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
testParams_t invalidInputTestParams[] = {
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_BIN},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_HEX},
    },
    {
        .pInputString = "12\0",
        .pCursor = {0,200},
        .pExpectedString = "21\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
    },
};
void test_addInvalidInput(void){   
    calcCoreState_t calcCore;
    int numTests = sizeof(invalidInputTestParams)/sizeof(invalidInputTestParams[0]);
    for(int i = 0 ; i < numTests ; i++){
        //printf("Test %i\r\n", i);
        setupTestStruct(&calcCore, &invalidInputTestParams[i]);
        calcCoreAddInput(&calcCore, &invalidInputTestParams[i]);
        calcCoreGetBuffer(&calcCore, &invalidInputTestParams[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Got     : %s \r\nExpected: %s\r\n", 
                    invalidInputTestParams[i].pOutputString,
                    invalidInputTestParams[i].pExpectedString);
        }
        TEST_ASSERT_EQUAL_STRING(
            invalidInputTestParams[i].pExpectedString,
            invalidInputTestParams[i].pOutputString);
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
        .pInputString = "123\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123,
    },
    {
        .pInputString = "123+456\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456,
    },
    {
        .pInputString = "123+456+789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456+789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 123+456+789,
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
    {
        .pInputString = "101a+s123,456,1213)-789\0",
        .pCursor = {0,0,0},
        .pExpectedString = "0x101a+SUM(0x123,0x456,0x1213)-0x789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_HEX},
        .expectedResult = 0x101a+(0x123+0x456+0x1213)-0x789,
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
        .pInputString = "123+456+\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123+456+\0",
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
    {
        .pInputString = "123*(456*(1+2)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123*(456*(1+2)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "123(\0",
        .pCursor = {0,0,0},
        .pExpectedString = "123(\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "(123,5)\0",
        .pCursor = {0,0,0},
        .pExpectedString = "(123,5)\0",
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


testParams_t base_conversion_params[] = {
    {
        .pInputString = "123i\0",
        .pCursor = {0,0,0,0},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC,
            [1] = inputBase_DEC,
            [2] = inputBase_DEC,
            [3] = inputBase_HEX,
            },
        .expectedResult = 123, 
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0,0,0,2},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC,
            [1] = inputBase_DEC,
            [2] = inputBase_DEC,
            [3] = inputBase_HEX,
            },
        .expectedResult = 123, 
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0,0,0,4},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC,
            [1] = inputBase_DEC,
            [2] = inputBase_DEC,
            [3] = inputBase_HEX,
            },
        .expectedResult = 123, 
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0,0,0,4},
        .pExpectedString = "0b1111011\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC,
            [1] = inputBase_DEC,
            [2] = inputBase_DEC,
            [3] = inputBase_BIN,
            },
        .expectedResult = 123, 
    },
    {
        .pInputString = "123+789i\0",
        .pCursor = {0,0,0,0,0,0,0,0},
        .pExpectedString = "123+0x315\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC, // 1
            [1] = inputBase_DEC, // 2
            [2] = inputBase_DEC, // 3
            [3] = inputBase_DEC, // +
            [4] = inputBase_DEC, // 7
            [5] = inputBase_DEC, // 8
            [6] = inputBase_DEC, // 9
            [7] = inputBase_HEX, // i
            },
        .expectedResult = 123+789, 
    },
    {
        .pInputString = "123+789i\0",
        .pCursor = {0,0,0,0,0,0,0,8},
        .pExpectedString = "0x7b+789\0",
        .pOutputString = {0},
        .inputBase = {
            [0] = inputBase_DEC, // 1
            [1] = inputBase_DEC, // 2
            [2] = inputBase_DEC, // 3
            [3] = inputBase_DEC, // +
            [4] = inputBase_DEC, // 7
            [5] = inputBase_DEC, // 8
            [6] = inputBase_DEC, // 9
            [7] = inputBase_HEX, // i
            },
        .expectedResult = 123+789, 
    },
};
void test_base_conversion(void){
    calcCoreState_t calcCore;
    int numTests = sizeof(base_conversion_params)/sizeof(base_conversion_params[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &base_conversion_params[i]);
        calcCoreAddInput(&calcCore, &base_conversion_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &base_conversion_params[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected result: %i \r\nReturned result: %i \r\n", 
                    base_conversion_params[i].pExpectedString,
                    base_conversion_params[i].pOutputString,
                    base_conversion_params[i].expectedResult,
                    calcCore.result
                    );
        }
        TEST_ASSERT_EQUAL_STRING(
            base_conversion_params[i].pExpectedString,
            base_conversion_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(base_conversion_params[i].expectedResult, calcCore.result, "Result not right.");
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

void test_null_pointers(void){
    calcCoreState_t calcCore;
    calcCoreState_t *pCalcCore = NULL;
    calc_funStatus_t funStatus = calc_coreInit(pCalcCore);
    int16_t syntaxIssueLoc = -1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,funStatus,"Unexpected return from core init");
    funStatus = calc_coreBufferTeardown(pCalcCore);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,funStatus,"Unexpected return from core teardown");
    funStatus = calc_addInput(pCalcCore,'1');
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,funStatus,"Unexpected return from add input");
    funStatus = calc_removeInput(pCalcCore);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,funStatus,"Unexpected return from add input");
    syntaxIssueLoc = -1;
    funStatus = calc_printBuffer(pCalcCore, NULL, 0, &syntaxIssueLoc);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,funStatus,"Unexpected return from print buffer");
    syntaxIssueLoc = -1;
    funStatus = calc_printBuffer(&calcCore, NULL, 0, &syntaxIssueLoc);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_STRING_BUFFER_ERROR,funStatus,"Unexpected return from print buffer");
}

testParams_t float_input_params[] = {
    {
        .pInputString = "123.45\0",
        .pCursor = {0,0,0,0},
        .pExpectedString = "123.45\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0x42f6e666,// See https://www.h-schmidt.net/FloatConverter/IEEE754.html 
    },
    {
        .pInputString = "123.45+34.5\0",
        .pCursor = {0,0,0,0},
        .pExpectedString = "123.45+34.5\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN-1] = inputBase_DEC},
        .expectedResult = 0x431df333,// See https://www.h-schmidt.net/FloatConverter/IEEE754.html 
    },
};
void test_float_input(void){
    calcCoreState_t calcCore;
    int numTests = sizeof(float_input_params)/sizeof(float_input_params[0]);
    for(int i = 0 ; i < numTests ; i++){
        setupTestStruct(&calcCore, &float_input_params[i]);
        calcCore.numberFormat.formatBase = INPUT_FMT_FLOAT;
        calcCoreAddInput(&calcCore, &float_input_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &float_input_params[i]);
        if(verbose){
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected result: %f \r\nReturned result: %i \r\n", 
                    float_input_params[i].pExpectedString,
                    float_input_params[i].pOutputString,
                    float_input_params[i].expectedResult,
                    calcCore.result
                    );
        }
        TEST_ASSERT_EQUAL_STRING(
            float_input_params[i].pExpectedString,
            float_input_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        //printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        printf("Got: 0x%x. ", calcCore.result);
        printf(" Expected 0x%x\r\n", float_input_params[i].expectedResult);
        TEST_ASSERT_EQUAL_INT_MESSAGE(float_input_params[i].expectedResult, calcCore.result, "Result not right.");
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
    //RUN_TEST(test_addInvalidInput);
    //RUN_TEST(test_solvable_solution);
    //RUN_TEST(test_unsolvable_solution);
    //RUN_TEST(test_null_pointers);
    //RUN_TEST(test_base_conversion);
    RUN_TEST(test_float_input);
    return UNITY_END();
}