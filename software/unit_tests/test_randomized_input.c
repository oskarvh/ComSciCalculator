/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Test suite that randomized input and check that the output is OK.
 *
 */

// Standard lib
#include <string.h>

#include "test_suite.h"

int randomInt(int min, int max) { return rand() % (max - min + 1) + min; }

/**
 * @brief Randomized an input string and the result of that.
 * @param pString Randomized string
 * @param maxLen Maximum length of the randomized string
 * @return Nothing
 */
void randomize_input(char *pString, uint16_t maxLen) {

    // Set all chars to 0
    memset(pString, '\0', maxLen);

    while (strlen(pString) < maxLen) {
        // Randomize which input format should be used for this character
        uint8_t base = baseToRadix[rand() % (2 - 0 + 1)] - 1;
        // Generate a random int
    }
}

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

    setupTestStruct(&calcCore, &output_formatting[i]);
    calcCoreAddInput(&calcCore, &output_formatting[i]);
    int8_t state = calc_solver(&calcCore);
    calcCoreGetBuffer(&calcCore, &output_formatting[i]);

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
    TEST_ASSERT_EQUAL_UINT_MESSAGE(0, calcCore.allocCounter, "Leaky memory!");
    if (calcCore.allocCounter != 0) {
        printf("************************************************\r\n\r\n");
        printf("WARNING: leaky memory: %i!\r\n", calcCore.allocCounter);
        printf("************************************************\r\n\r\n");
    }
}