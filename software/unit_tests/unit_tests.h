/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 */

/* ----------------- DEFINES ----------------- */
#define MAX_STR_LEN 255
/* ----------------- HEADERS ----------------- */

// ComSciCalc headers
#include "../comSciCalc_lib/comscicalc.h"
#include "../comSciCalc_lib/comscicalc_operators.h"

/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
// Struct for the test setup
typedef struct testSetup {
    calcCoreState_t *pCoreState;
    char *pInputString;
    char *pExpectedString;
    char *pOutputString; 
    int *pCursor;
    inputBase_t inputBase;
} testSetup_t;

// Struct for the test parameters. 
typedef struct testParams {
    char pInputString[MAX_STR_LEN];
    char pExpectedString[MAX_STR_LEN];
    char pOutputString[MAX_STR_LEN];
    int pCursor[MAX_STR_LEN];
    inputBase_t inputBase;
} testParams_t;

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

// Setup and teardown functions. Need custom here. 
void setupTestStruct(calcCoreState_t *pCoreState, 
    testSetup_t *pTestSetup, 
    testParams_t *pTestParams);
void teardownTestStruct(testSetup_t *pTestSetup);

// Function to add the input to the calc core. 
void calcCoreAddInput(testSetup_t *pTestSetup);