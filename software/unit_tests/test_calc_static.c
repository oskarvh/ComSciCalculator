/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Test suite to test positive and negative paths
 * of calculations using the comSciCalc_lib. 
 * 
 * The parameters are static, and aims to test the 
 * solvability and non-solvability of expressions.
 * 
 * 
 * Requirements:
 * 1. For any correct syntax, the comSciCalc_lib shall solve the expression, unless unsolvable
 * 2. ComSciCalc_lib shall catch unsolvable, syntax correct expressions (e.g. divide by zero)
 * 3. ComSciCalc_lib shall catch and highlight incorrect syntax. 
 */

// Standard lib
#include <string.h>

#include "test_suite.h"


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
testParams_t params_solvable[] = {
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
    int numTests = sizeof(params_solvable) / sizeof(params_solvable[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &params_solvable[i]);
        calcCoreAddInput(&calcCore, &params_solvable[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &params_solvable[i]);
        
        TEST_ASSERT_EQUAL_STRING(params_solvable[i].pExpectedString,
                                 params_solvable[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        TEST_ASSERT_EQUAL_INT_MESSAGE(params_solvable[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
    }
}

testParams_t params_unsolvable[] = {
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
    int numTests = sizeof(params_unsolvable) / sizeof(params_unsolvable[0]);
    for (int i = 0; i < numTests; i++) {
        setupTestStruct(&calcCore, &params_unsolvable[i]);
        calcCoreAddInput(&calcCore, &params_unsolvable[i]);
        int8_t state = calc_solver(&calcCore);
        calcCoreGetBuffer(&calcCore, &params_unsolvable[i]);
        
        TEST_ASSERT_EQUAL_STRING(params_unsolvable[i].pExpectedString,
                                 params_unsolvable[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        TEST_ASSERT_EQUAL_INT_MESSAGE(params_unsolvable[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
        }
    }
}


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
        TEST_ASSERT_EQUAL_STRING(long_expression[i].pExpectedString,
                                 long_expression[i].pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
        TEST_ASSERT_EQUAL_INT_MESSAGE(long_expression[i].expectedResult,
                                      calcCore.result, "Result not right.");
        TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter,
                                       "Leaky memory!");
        if (calcCore.allocCounter != 0) {
            printf("************************************************\r\n\r\n");
            printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
            printf("************************************************\r\n\r\n");
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
    }
}