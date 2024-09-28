/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Header file for unit tests of ComSciCalc lib.
 */

// Unity test framework.
#include "../Unity/src/unity.h"

// Unit test header
#include "unit_tests.h"

// Tests included here:
extern void test_addRemoveInput(void);
extern void test_addInvalidInput(void);
extern void test_solvable_solution(void);
extern void test_unsolvable_solution(void);
extern void test_base_conversion(void);
extern void test_string_to_fixed_point(void);
extern void test_base_conversion(void);
extern void test_null_pointers(void);
extern void test_leading_zeros(void);
extern void test_solvable_long_expression(void);
extern void test_format_conversion(void);
extern void test_logic_operations(void);
extern void test_comms(void);