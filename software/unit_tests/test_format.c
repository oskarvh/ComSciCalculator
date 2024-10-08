/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Unit test for checking input and output formatting.
 *
 *
 * Requirements:
 * 1. ComSciCalc_lib shall support at least 3 input and output bases: Decimal,
 * Hexadecimal and binary
 * 2. ComSciCalc_lib shall support at least 3 input and output formats: integer,
 *    fixed point and floating point.
 * 3. The fixed point format shall support a variable decimal point location,
 * and the total number of bits for decimal and integer bits shall equal the
 * total number of bits.
 * 4. ComSciCalc_lib shall support 32 and 64 bit input and output formats.
 */

// Standard lib
#include <string.h>

#include "test_suite.h"

testParams_t base_conversion_params[] = {
    {
        .pInputString = "123i\0",
        .pCursor = {0, 0, 0, 0},
        .pExpectedString = "0x7b\0",

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
        calcCoreGetBuffer(&calcCore, pOutputString);

        TEST_ASSERT_EQUAL_STRING(base_conversion_params[i].pExpectedString,
                                 pOutputString);
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
    }
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
        calcCoreGetBuffer(&calcCore, pOutputString);

        TEST_ASSERT_EQUAL_STRING(leading_zeros_test_params[i].pExpectedString,
                                 pOutputString);
        teardownTestStruct(&calcCore);

        // Check that an equal amount of mallocs and free's happened
        // in the calculator core
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
    }
}

testParams_t output_formatting[] = {
    // Integer input, Integer output
    {
        .pInputString = "123\0",
        .pCursor = {0, 0, 0},
        .pExpectedString = "123\0",
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
        calcCoreGetBuffer(&calcCore, pOutputString);

        TEST_ASSERT_EQUAL_STRING(output_formatting[i].pExpectedString,
                                 pOutputString);
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
    }
}