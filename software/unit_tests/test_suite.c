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
#include "test_suite.h"

// utils
#include "../comSciCalc_lib/print_utils.h"

// This function will run before each test.
void setUp(void) {
    // set stuff up here
}

// This function will run after each test.
void tearDown(void) {
    // clean stuff up here
}

int main(void) {
    UNITY_BEGIN();
    // See test_suite.h for included tests.
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
    // RUN_TEST(test_logic_operations);
    RUN_TEST(test_comms);
    return UNITY_END();
}