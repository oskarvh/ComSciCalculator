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

// utils
#include "../comSciCalc_lib/print_utils.h"

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
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1234\b\0",
        .pCursor = {0, 0, 0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "0x123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
    },
    {
        .pInputString = "1010\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "0b1010\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_BIN},
    },
    {
        .pInputString = "123+\0",
        .pCursor = {0, 0, 0, 2},
        .pExpectedString = "1+23\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0, 0, 0, 0, 4, 0, 0, 0, 0},
        .pExpectedString = "NAND(123+123)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123\b\0",
        .pCursor = {0, 0, 0, 1},
        .pExpectedString = "13\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 1},
        .pExpectedString = "13\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 2},
        .pExpectedString = "NAND(3\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 3},
        .pExpectedString = "1NAND(3\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n456*(12+45))\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(456*(12+45))\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1234\0",
        .pCursor = {0, 0, 0, 5},
        .pExpectedString = "4123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_NONE},
    },
    {
        .pInputString = ".\0",
        .pCursor = {0},
        .pExpectedString = ".\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
};
void test_addRemoveInput(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(addInputTestParams) / sizeof(addInputTestParams[0]);
    for (int i = 0; i < numTests; i++) {
        // printf("Test %i\r\n", i);
        setupTestStruct(&calcCore, &addInputTestParams[i]);
        calcCoreAddInput(&calcCore, &addInputTestParams[i]);
        calcCoreGetBuffer(&calcCore, &addInputTestParams[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Got     : %s \r\nExpected: %s\r\n",
                   addInputTestParams[i].pOutputString,
                   addInputTestParams[i].pExpectedString);
        }
        TEST_ASSERT_EQUAL_STRING(addInputTestParams[i].pExpectedString,
                                 addInputTestParams[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
        if (verbose) {
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
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_BIN},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
    },
    {
        .pInputString = "12\0",
        .pCursor = {0, 200},
        .pExpectedString = "21\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
};
void test_addInvalidInput(void) {
    calcCoreState_t calcCore;
    int numTests =
        sizeof(invalidInputTestParams) / sizeof(invalidInputTestParams[0]);
    for (int i = 0; i < numTests; i++) {
        // printf("Test %i\r\n", i);
        setupTestStruct(&calcCore, &invalidInputTestParams[i]);
        calcCoreAddInput(&calcCore, &invalidInputTestParams[i]);
        calcCoreGetBuffer(&calcCore, &invalidInputTestParams[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Got     : %s \r\nExpected: %s\r\n",
                   invalidInputTestParams[i].pOutputString,
                   invalidInputTestParams[i].pExpectedString);
        }
        TEST_ASSERT_EQUAL_STRING(invalidInputTestParams[i].pExpectedString,
                                 invalidInputTestParams[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
        if (verbose) {
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
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123,
    },
    {
        .pInputString = "123+456\0",
        .pCursor = {0},
        .pExpectedString = "123+456\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + 456,
    },
    {
        .pInputString = "123+456+789\0",
        .pCursor = {0},
        .pExpectedString = "123+456+789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + 456 + 789,
    },
    {
        .pInputString = "(123+456)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + 456,
    },
    {
        .pInputString = "123+456*789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123+456*789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + 456 * 789,
    },
    {
        .pInputString = "(123+456*789)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "(123+456*789)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + 456 * 789,
    },
    {
        .pInputString = "(123+456)*789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "(123+456)*789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = (123 + 456) * 789,
    },
    {
        .pInputString = "123+(456+789)/1011\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123+(456+789)/1011\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123 + (456 + 789) / 1011,
    },
    {
        .pInputString = "~123)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NOT(123)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = ~123,
    },
    {
        .pInputString = "~123+456)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NOT(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = ~(123 + 456),
    },
    {
        .pInputString = "789+~123+456)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "789+NOT(123+456)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 789 + ~(123 + 456),
    },
    {
        .pInputString = "~123+456)-789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NOT(123+456)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = ~(123 + 456) - 789,
    },
    {
        .pInputString = "1011+~123+456)-789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "1011+NOT(123+456)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 1011 + ~(123 + 456) - 789,
    },
    {
        .pInputString = "1011+~123+456+1213)-789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "1011+NOT(123+456+1213)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 1011 + ~(123 + 456 + 1213) - 789,
    },
    {
        .pInputString = "1011+s123,456,1213)-789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "1011+SUM(123,456,1213)-789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 1011 + (123 + 456 + 1213) - 789,
    },
    {
        .pInputString = "101a+s123,456,1213)-789\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "0x101a+SUM(0x123,0x456,0x1213)-0x789\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
        .expectedResult = 0x101a + (0x123 + 0x456 + 0x1213) - 0x789,
    },

};
void test_solvable_solution(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(solvable_params) / sizeof(solvable_params[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &solvable_params[i]);
        calcCoreAddInput(&calcCore, &solvable_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &solvable_params[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %i \r\nReturned result: %i \r\n",
                   solvable_params[i].pExpectedString,
                   solvable_params[i].pOutputString,
                   solvable_params[i].expectedResult, calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(solvable_params[i].pExpectedString,
                                 solvable_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(solvable_params[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

testParams_t unsolvable_params[] = {
    {
        .pInputString = "123+\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123+\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "123+456+\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123+456+\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "~123+\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NOT(123+\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "~)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NOT()\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "()\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "()\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "(\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "(\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = ")\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = ")\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s*,+)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "SUM(*,+)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s,)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "SUM(,)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "s,1)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "SUM(,1)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "123*(456*(1+2)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123*(456*(1+2)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "123(\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123(\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
    {
        .pInputString = "(123,5)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "(123,5)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0, // There shouldn't be a result
    },
};
void test_unsolvable_solution(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(unsolvable_params) / sizeof(unsolvable_params[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &unsolvable_params[i]);
        calcCoreAddInput(&calcCore, &unsolvable_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &unsolvable_params[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %i \r\nReturned result: %i \r\n",
                   unsolvable_params[i].pExpectedString,
                   unsolvable_params[i].pOutputString,
                   unsolvable_params[i].expectedResult, calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(unsolvable_params[i].pExpectedString,
                                 unsolvable_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(unsolvable_params[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

testParams_t base_conversion_params[] = {
    {
        .pInputString = "123i\0",
        .pCursor = {0, 0, 0, 0},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC,
                [1] = inputBase_DEC,
                [2] = inputBase_DEC,
                [3] = inputBase_HEX,
            },
        .expectedResult = 123,
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0, 0, 0, 2},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC,
                [1] = inputBase_DEC,
                [2] = inputBase_DEC,
                [3] = inputBase_HEX,
            },
        .expectedResult = 123,
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0, 0, 0, 4},
        .pExpectedString = "0x7b\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC,
                [1] = inputBase_DEC,
                [2] = inputBase_DEC,
                [3] = inputBase_HEX,
            },
        .expectedResult = 123,
    },
    {
        .pInputString = "123i\0",
        .pCursor = {0, 0, 0, 4},
        .pExpectedString = "0b1111011\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC,
                [1] = inputBase_DEC,
                [2] = inputBase_DEC,
                [3] = inputBase_BIN,
            },
        .expectedResult = 123,
    },
    {
        .pInputString = "123+789i\0",
        .pCursor = {0, 0, 0, 0, 0, 0, 0, 0},
        .pExpectedString = "123+0x315\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC, // 1
                [1] = inputBase_DEC, // 2
                [2] = inputBase_DEC, // 3
                [3] = inputBase_DEC, // +
                [4] = inputBase_DEC, // 7
                [5] = inputBase_DEC, // 8
                [6] = inputBase_DEC, // 9
                [7] = inputBase_HEX, // i
            },
        .expectedResult = 123 + 789,
    },
    {
        .pInputString = "123+789i\0",
        .pCursor = {0, 0, 0, 0, 0, 0, 0, 8},
        .pExpectedString = "0x7b+789\0",
        .pOutputString = {0},
        .inputBase =
            {
                [0] = inputBase_DEC, // 1
                [1] = inputBase_DEC, // 2
                [2] = inputBase_DEC, // 3
                [3] = inputBase_DEC, // +
                [4] = inputBase_DEC, // 7
                [5] = inputBase_DEC, // 8
                [6] = inputBase_DEC, // 9
                [7] = inputBase_HEX, // i
            },
        .expectedResult = 123 + 789,
    },
};
void test_base_conversion(void) {
    calcCoreState_t calcCore;
    int numTests =
        sizeof(base_conversion_params) / sizeof(base_conversion_params[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &base_conversion_params[i]);
        calcCoreAddInput(&calcCore, &base_conversion_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &base_conversion_params[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %i \r\nReturned result: %i \r\n",
                   base_conversion_params[i].pExpectedString,
                   base_conversion_params[i].pOutputString,
                   base_conversion_params[i].expectedResult, calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(base_conversion_params[i].pExpectedString,
                                 base_conversion_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(base_conversion_params[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

void test_null_pointers(void) {
    calcCoreState_t calcCore;
    calcCoreState_t *pCalcCore = NULL;
    calc_funStatus_t funStatus = calc_coreInit(pCalcCore);
    int16_t syntaxIssueLoc = -1;
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,
                                  funStatus,
                                  "Unexpected return from core init");
    funStatus = calc_coreBufferTeardown(pCalcCore);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,
                                  funStatus,
                                  "Unexpected return from core teardown");
    funStatus = calc_addInput(pCalcCore, '1');
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,
                                  funStatus,
                                  "Unexpected return from add input");
    funStatus = calc_removeInput(pCalcCore);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,
                                  funStatus,
                                  "Unexpected return from add input");
    syntaxIssueLoc = -1;
    funStatus = calc_printBuffer(pCalcCore, NULL, 0, &syntaxIssueLoc);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_CALC_CORE_STATE_NULL,
                                  funStatus,
                                  "Unexpected return from print buffer");
    syntaxIssueLoc = -1;
    funStatus = calc_printBuffer(&calcCore, NULL, 0, &syntaxIssueLoc);
    TEST_ASSERT_EQUAL_INT_MESSAGE(calc_funStatus_STRING_BUFFER_ERROR, funStatus,
                                  "Unexpected return from print buffer");
}

testParams_t fixed_point_test_params[] = {
    {
        .pInputString = "123.5\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x7b8000, // See
                      // https://chummersone.github.io/qformat.html#converter
    },
    {
        .pInputString = "123.45\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x007b7333, // See
                        // https://chummersone.github.io/qformat.html#converter
    },
    {
        .pInputString = "0.5\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x00008000, // See
                        // https://chummersone.github.io/qformat.html#converter
    },
    {
        .pInputString = "123.3335\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x007b5560, // See
                        // https://chummersone.github.io/qformat.html#converter
    },
    {
        .pInputString = "175.188\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x00af3021, // See
                        // https://chummersone.github.io/qformat.html#converter
    },
    {
        .pInputString = "18.1\0",
        .inputBase = {[0 ... MAX_STR_LEN - 1] = 10},
        .expectedResult =
            0x0012199a, // See
                        // https://chummersone.github.io/qformat.html#converter
    },

};
void test_string_to_fixed_point(void) {
    int numTests =
        sizeof(fixed_point_test_params) / sizeof(fixed_point_test_params[0]);
    for (int i = 0; i < numTests; i++) {
        SUBRESULT_INT fp = 0;
        fp = strtofp(fixed_point_test_params[i].pInputString, false, 16,
                     (uint16_t)(fixed_point_test_params[i].inputBase[0]));
        if (verbose) {
            printf("fp = 0x%llx, ", fp);
            printf("expected = 0x%llx\r\n",
                   fixed_point_test_params[i].expectedResult);
        }
        // Test that it's within 1 bit. Conversion between float and fixed is
        // not absolute, but given the circumstances, it should be OK.
        TEST_ASSERT(abs(fixed_point_test_params[i].expectedResult - fp) <= 1);
    }
}

testParams_t leading_zeros_test_params[] = {
    {
        .pInputString = "0123+056\0",
        .pCursor = {0, 0, 0, 0},
        .pExpectedString = "0123+056\0",
        .pOutputString = {0},
        .expectedResult = 123 + 56,
        .numberFormat.fixedPointDecimalPlace = 16,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
    },
    {
        .pInputString = "012.8000+07e.ab85\0",
        .pCursor = {0, 0, 0, 0},
        .pExpectedString = "0x012.8000+0x07e.ab85\0",
        .pOutputString = {0},
        .expectedResult =
            0x912b85, // See
                      // https://chummersone.github.io/qformat.html#converter
        .numberFormat.fixedPointDecimalPlace = 16,
        .numberFormat.inputBase = inputBase_HEX,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FIXED,
    },
};
void test_leading_zeros(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(leading_zeros_test_params) /
                   sizeof(leading_zeros_test_params[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &leading_zeros_test_params[i]);
        memcpy(&(calcCore.numberFormat),
               &(leading_zeros_test_params[i].numberFormat),
               sizeof(numberFormat_t));
        for (int j = 0; j < MAX_STR_LEN; j++) {
            leading_zeros_test_params[i].inputBase[j] =
                leading_zeros_test_params[i].numberFormat.inputBase;
        }
        calcCoreAddInput(&calcCore, &leading_zeros_test_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &leading_zeros_test_params[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %i \r\nReturned result: %i \r\n",
                   leading_zeros_test_params[i].pExpectedString,
                   leading_zeros_test_params[i].pOutputString,
                   leading_zeros_test_params[i].expectedResult,
                   calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(leading_zeros_test_params[i].pExpectedString,
                                 leading_zeros_test_params[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            leading_zeros_test_params[i].expectedResult, calcCore.result,
            "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

/* ----------------------------------------------------------------
 * Solvable equations (positive testing)
 * ----------------------------------------------------------------*/
testParams_t long_expression[] = {
    {
        .pInputString =
            "1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1\0",
        .pCursor = {0, 0, 0},
        .pExpectedString =
            "1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1+1\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 32,
    },
};
void test_solvable_long_expression(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(long_expression) / sizeof(long_expression[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &long_expression[i]);
        calcCoreAddInput(&calcCore, &long_expression[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &long_expression[i]);
        if (verbose) {
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %i \r\nReturned result: %i \r\n",
                   long_expression[i].pExpectedString,
                   long_expression[i].pOutputString,
                   long_expression[i].expectedResult, calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(long_expression[i].pExpectedString,
                                 long_expression[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(long_expression[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

testParams_t output_formatting[] = {
    // Integer input, Integer output
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_INT,
        .pResultStringDec = "123\0",
        .pResultStringHex = "0x7B\0",
        .pResultStringBin = "0b111 1011\0",
    },
    // Integer input, fixed point output
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 123,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_FIXED,
        .pResultStringDec = "123.0\0",
        .pResultStringHex = "0x7B.0\0",
        .pResultStringBin = "0b111 1011.0\0",
    },
    // Integer input, IEEE 754 floating point output
    // See https://www.h-schmidt.net/FloatConverter/IEEE754.html
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = (float)123,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 32,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.0\0",
        .pResultStringHex = "0x42F60000\0",
        .pResultStringBin = "0b0100 0010 1111 0110 0000 0000 0000 0000\0",
    },
    // Integer input, double precision floating point output
    // see https://www.binaryconvert.com/convert_double.html
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = (double)123,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.0\0",
        .pResultStringHex = "0x405EC00000000000\0",
        .pResultStringBin =
            "0b0100 0000 0101 1110 1100 0000 0000 0000 0000 0000 0000 0000"
            " 0000 0000 0000 0000\0",
    },
    //==========================FLOATING POINT INPUT==========================
    // float input, Integer output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x42f63d71, // see
        // https://www.h-schmidt.net/FloatConverter/IEEE754.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 32,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_INT,
        .pResultStringDec = "123\0",
        .pResultStringHex = "0x7B\0",
        .pResultStringBin = "0b111 1011\0",
    },
    // double precision float input, Integer output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x405EC7AE147AE148, // see
        // https://www.binaryconvert.com/convert_double.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_INT,
        .pResultStringDec = "123\0",
        .pResultStringHex = "0x7B\0",
        .pResultStringBin = "0b111 1011\0",
    },
    // float input, float output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x42f63d71, // see
        // https://www.h-schmidt.net/FloatConverter/IEEE754.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 32,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x42F63D71\0",
        .pResultStringBin = "0b0100 0010 1111 0110 0011 1101 0111 0001\0",
    },
    // double precision float input, double precision float output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x405EC7AE147AE148, // see
        // https://www.binaryconvert.com/convert_double.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x405EC7AE147AE148\0",
        .pResultStringBin =
            "0b0100 0000 0101 1110 1100 0111 1010 1110 0001 0100 0111 1010"
            " 1110 0001 0100 1000\0",
    },
    // float input, fixed point 16.16 output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x42f63d71, // see
        // https://www.h-schmidt.net/FloatConverter/IEEE754.html
        .numberFormat.fixedPointDecimalPlace = 16,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 32,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FIXED,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x7B.1EB8\0",
        .pResultStringBin = "0b111 1011.0001 1110 1011 1\0",
    },
    // double precision floating point input, fixed point 32.32 output
    {
        .pInputString = "123.12\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123.12\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x405EC7AE147AE148, // see
        // https://www.h-schmidt.net/FloatConverter/IEEE754.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FIXED,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x7B.1EB851EB\0",
        .pResultStringBin =
            "0b111 1011.0001 1110 1011 1000 0101 0001 1110 1011\0",
    },
    {
        .pInputString = "23.1+100.02\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "23.1+100.02\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x405EC7AE147AE148, // see
        // https://www.binaryconvert.com/convert_double.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x405EC7AE147AE148\0",
        .pResultStringBin =
            "0b0100 0000 0101 1110 1100 0111 1010 1110 0001 0100 0111 1010"
            " 1110 0001 0100 1000\0",
    },
    {
        // THIS CURRENTLY FAILS!!!
        .pInputString = "12.312*10.0\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "12.312*10.0\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
        .expectedResult = 0x405EC7AE147AE147, // see
        // https://www.binaryconvert.com/convert_double.html
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_DEC,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_FLOAT,
        .numberFormat.outputFormat = INPUT_FMT_FLOAT,
        .pResultStringDec = "123.12\0",
        .pResultStringHex = "0x405EC7AE147AE147\0",
        .pResultStringBin =
            "0b0100 0000 0101 1110 1100 0111 1010 1110 0001 0100 0111 1010"
            " 1110 0001 0100 0111\0",
    },

};
// This function tests conversions between different input format
// e.g. fixed, float and int, and output formats, e.g. fixed, float, int.
void test_format_conversion(void) {
    //! String for displaying the integer format to the user
    char int_display_string[] = "INT";
    //! String for displaying the fixed point format to the user.
    char fixed_display_string[] = "FIXED";
    //! String for displaying the floating point format to the user
    char float_display_string[] = "FLOAT";
    //! Array of pointers to the strings used to show which format is active on
    //! the screen.
    const char *formatDisplayStrings[] = {
        [INPUT_FMT_INT] = int_display_string,
        [INPUT_FMT_FIXED] = fixed_display_string,
        [INPUT_FMT_FLOAT] = float_display_string,
    };
    calcCoreState_t calcCore;
    int numTests = sizeof(output_formatting) / sizeof(output_formatting[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &output_formatting[i]);
        calcCoreAddInput(&calcCore, &output_formatting[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &output_formatting[i]);
        if (verbose) {
            printf("input: %s, output:%s\r\n",
                   formatDisplayStrings[(
                       output_formatting[i].numberFormat.inputFormat)],
                   formatDisplayStrings[(
                       output_formatting[i].numberFormat.outputFormat)]);
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %lli \r\nReturned result: %lli \r\n",
                   output_formatting[i].pExpectedString,
                   output_formatting[i].pOutputString,
                   output_formatting[i].expectedResult, calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(output_formatting[i].pExpectedString,
                                 output_formatting[i].pOutputString);
        // Convert the results to string:
        char resultStringDec[MAX_STR_LEN] = {0};
        convertResult(resultStringDec, calcCore.result,
                      &(output_formatting[i].numberFormat), inputBase_DEC);
        TEST_ASSERT_EQUAL_STRING(output_formatting[i].pResultStringDec,
                                 resultStringDec);
        char resultStringHex[MAX_STR_LEN] = {0};
        convertResult(resultStringHex, calcCore.result,
                      &(output_formatting[i].numberFormat), inputBase_HEX);
        TEST_ASSERT_EQUAL_STRING(output_formatting[i].pResultStringHex,
                                 resultStringHex);
        char resultStringBin[MAX_STR_LEN] = {0};
        convertResult(resultStringBin, calcCore.result,
                      &(output_formatting[i].numberFormat), inputBase_BIN);
        TEST_ASSERT_EQUAL_STRING(output_formatting[i].pResultStringBin,
                                 resultStringBin);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(output_formatting[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

testParams_t logic_expression_test_params[] = {
    // NAND implementation
    {
        .pInputString = "n0,f)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NAND(0x0,0xf)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
        .expectedResult = 15,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_HEX,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_INT,
        .pResultStringDec = "15\0",
        .pResultStringHex = "0xF\0",
        .pResultStringBin = "0b1111\0",
    },
    {
        .pInputString = "n0,f,f)\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "NAND(0x0,0xf,0xf)\0",
        .pOutputString = {0},
        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
        .expectedResult = 0,
        .numberFormat.fixedPointDecimalPlace = 32,
        .numberFormat.inputBase = inputBase_HEX,
        .numberFormat.numBits = 64,
        .numberFormat.sign = false,
        .numberFormat.inputFormat = INPUT_FMT_INT,
        .numberFormat.outputFormat = INPUT_FMT_INT,
        .pResultStringDec = "0\0",
        .pResultStringHex = "0x0\0",
        .pResultStringBin = "0b0\0",
    },
};
// This test checks the logic operation
void test_logic_operations(void) {
    //! String for displaying the integer format to the user
    char int_display_string[] = "INT";
    //! String for displaying the fixed point format to the user.
    char fixed_display_string[] = "FIXED";
    //! String for displaying the floating point format to the user
    char float_display_string[] = "FLOAT";
    //! Array of pointers to the strings used to show which format is active on
    //! the screen.
    const char *formatDisplayStrings[] = {
        [INPUT_FMT_INT] = int_display_string,
        [INPUT_FMT_FIXED] = fixed_display_string,
        [INPUT_FMT_FLOAT] = float_display_string,
    };
    calcCoreState_t calcCore;
    int numTests = sizeof(logic_expression_test_params) /
                   sizeof(logic_expression_test_params[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &logic_expression_test_params[i]);
        calcCoreAddInput(&calcCore, &logic_expression_test_params[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &logic_expression_test_params[i]);
        if (verbose) {
            printf(
                "input: %s, output:%s\r\n",
                formatDisplayStrings[(
                    logic_expression_test_params[i].numberFormat.inputFormat)],
                formatDisplayStrings[(logic_expression_test_params[i]
                                          .numberFormat.outputFormat)]);
            printf("------------------------------------------------\r\n");
            printf("Input:           %s \r\nExpected:        %s \r\nExpected "
                   "result: %lli \r\nReturned result: %lli \r\n",
                   logic_expression_test_params[i].pExpectedString,
                   logic_expression_test_params[i].pOutputString,
                   logic_expression_test_params[i].expectedResult,
                   calcCore.result);
        }
        TEST_ASSERT_EQUAL_STRING(
            logic_expression_test_params[i].pExpectedString,
            logic_expression_test_params[i].pOutputString);
        // Convert the results to string:
        char resultStringDec[MAX_STR_LEN] = {0};
        convertResult(resultStringDec, calcCore.result,
                      &(logic_expression_test_params[i].numberFormat),
                      inputBase_DEC);
        TEST_ASSERT_EQUAL_STRING(
            logic_expression_test_params[i].pResultStringDec, resultStringDec);
        char resultStringHex[MAX_STR_LEN] = {0};
        convertResult(resultStringHex, calcCore.result,
                      &(logic_expression_test_params[i].numberFormat),
                      inputBase_HEX);
        TEST_ASSERT_EQUAL_STRING(
            logic_expression_test_params[i].pResultStringHex, resultStringHex);
        char resultStringBin[MAX_STR_LEN] = {0};
        convertResult(resultStringBin, calcCore.result,
                      &(logic_expression_test_params[i].numberFormat),
                      inputBase_BIN);
        TEST_ASSERT_EQUAL_STRING(
            logic_expression_test_params[i].pResultStringBin, resultStringBin);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        // printf("Allocation counter = %i\r\n", calcCore.allocCounter);
        TEST_ASSERT_EQUAL_INT_MESSAGE(
            logic_expression_test_params[i].expectedResult, calcCore.result,
            "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
        if (verbose) {
            printf("------------------------------------------------\r\n\r\n");
        }
    }
}

/* ----------------------------------------------------------------
 * Main. Only starts the tests.
 * ----------------------------------------------------------------*/
int main(void) {
    verbose = false;
    UNITY_BEGIN();
    // RUN_TEST(test_addRemoveInput);
    // RUN_TEST(test_addInvalidInput);
    // RUN_TEST(test_solvable_solution);
    // RUN_TEST(test_unsolvable_solution);
    // RUN_TEST(test_null_pointers);
    // RUN_TEST(test_base_conversion);
    // RUN_TEST(test_string_to_fixed_point);
    // RUN_TEST(test_leading_zeros);
    // RUN_TEST(test_solvable_long_expression);
    // RUN_TEST(test_format_conversion);
    RUN_TEST(test_logic_operations);
    return UNITY_END();
}