/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 */

/* ----------------- DEFINES ----------------- */
#define MAX_STR_LEN 100
/* ----------------- HEADERS ----------------- */

// ComSciCalc headers
#include "../comSciCalc_lib/comscicalc.h"
#include "../comSciCalc_lib/comscicalc_operators.h"

/* -------------------------------------------
 * ------- ENUMS, TYPEDEFS AND STRUCTS -------
 * -------------------------------------------*/
// Struct for the test parameters. 
typedef struct testParams {
    char pInputString[MAX_STR_LEN];
    char pExpectedString[MAX_STR_LEN];
    char pOutputString[MAX_STR_LEN];
    int pCursor[MAX_STR_LEN];
    inputBase_t inputBase[MAX_STR_LEN];
    SUBRESULT_INT expectedResult;
    numberFormat_t numberFormat;
    char pResultStringDec[MAX_STR_LEN];
    char pResultStringBin[MAX_STR_LEN];
    char pResultStringHex[MAX_STR_LEN];
} testParams_t;

/* -------------------------------------------
 * ----------- FUNCTION PROTOTYPES -----------
 * -------------------------------------------*/

// Setup and teardown functions. Need custom here. 
void setupTestStruct(calcCoreState_t *pCoreState, 
    testParams_t *pTestParams);
void teardownTestStruct(calcCoreState_t *pCoreState);

// Function to add the input to the calc core. 
void calcCoreAddInput(calcCoreState_t *pCoreState, testParams_t *pTestParams);
void calcCoreGetBuffer(calcCoreState_t *pCoreState, testParams_t *pTestParams);
