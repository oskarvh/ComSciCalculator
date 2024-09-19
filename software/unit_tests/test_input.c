/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Test suite to test basic input/output functions of the ComSciCalc lib.
 *
 * Requirements:
 * 1. Inputs shall be added at cursor.
 * 2. Backslash operator (\b) shall remove the last operator or number left of
 * the cursor.
 * 3. Inputs shall be restricted to the current input format (dec, float, int).
 * 4. Inputs shall be restricted to the current input base (dec, bin hex).
 * 5. Cursor shall be limited to current expressions length.
 */

// Standard lib
#include <string.h>

#include "test_suite.h"

/*
 * Positive (happy path) tests
 */
testParams_t params_addInput[] = {
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1234\b\0",
        .pCursor = {0, 0, 0, 0, 0},
        .pExpectedString = "123\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "0x123\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
    },
    {
        .pInputString = "1010\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "0b1010\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_BIN},
    },
    {
        .pInputString = "123+\0",
        .pCursor = {0, 0, 0, 2},
        .pExpectedString = "1+23\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(123\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n123)\0",
        .pCursor = {0, 0, 0, 0, 4, 0, 0, 0, 0},
        .pExpectedString = "NAND(123+123)\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123\b\0",
        .pCursor = {0, 0, 0, 1},
        .pExpectedString = "13\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 1},
        .pExpectedString = "13\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 2},
        .pExpectedString = "NAND(3\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1n3\b\0",
        .pCursor = {0, 0, 0, 3},
        .pExpectedString = "1NAND(3\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "123+n456*(12+45))\0",
        .pCursor = {0},
        .pExpectedString = "123+NAND(456*(12+45))\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1234\0",
        .pCursor = {0, 0, 0, 5},
        .pExpectedString = "4123\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "1\0",
        .pCursor = {0},
        .pExpectedString = "\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_NONE},
    },
    {
        .pInputString = ".\0",
        .pCursor = {0},
        .pExpectedString = ".\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
};

void test_addRemoveInput(void) {
    calcCoreState_t calcCore;
    int numTests = sizeof(params_addInput) / sizeof(params_addInput[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &params_addInput[i]);
        calcCoreAddInput(&calcCore, &params_addInput[i]);
        calcCoreGetBuffer(&calcCore, pOutputString);
        TEST_ASSERT_EQUAL_STRING(params_addInput[i].pExpectedString,
                                 pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
    }
}

/*
 * Negative path test.
 */
testParams_t params_addNegativeInput[] = {
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_BIN},
    },
    {
        .pInputString = "q\0",
        .pCursor = {0},
        .pExpectedString = "\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_HEX},
    },
    {
        .pInputString = "12\0",
        .pCursor = {0, 200},
        .pExpectedString = "21\0",

        .inputBase = {[0 ... MAX_STR_LEN - 1] = inputBase_DEC},
    },
};
void test_addInvalidInput(void) {
    calcCoreState_t calcCore;
    int numTests =
        sizeof(params_addNegativeInput) / sizeof(params_addNegativeInput[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &params_addNegativeInput[i]);
        calcCoreAddInput(&calcCore, &params_addNegativeInput[i]);
        calcCoreGetBuffer(&calcCore, pOutputString);

        TEST_ASSERT_EQUAL_STRING(params_addNegativeInput[i].pExpectedString,
                                 pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        TEST_ASSERT_EQUAL_INT(0, calcCore.allocCounter);
    }
}