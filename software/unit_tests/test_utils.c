/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Test suite to unit test utility functions,
 * which might not be fully covered by other functions,
 * but is sufficiently advanced to warrant its own unit tests.
 *
 *
 * There are no associated top level requirements here.
 *
 */

// Standard lib
#include <string.h>

#include "test_suite.h"

//! Test to check that NULL pointers are caught by functions.
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