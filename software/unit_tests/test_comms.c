/*
 * Copyright (c) 2024
 * Oskar von Heideken.
 *
 * Test suite for communication protocol
 */

// Standard lib
#include <string.h>

#include "comscicalc_comms.h"
#include "test_suite.h"

void test_comms(void) {
    // Create a buffer used to "send" the message
    char pBuf[8 + 255 + 16] = {0};
    char pMsg[8] = "Testing";
    comms_options_t options = {
        .msgLen = 8,
        .messageType = ack,
        .bUseChecksum = false,
        .bUseCRC = false,
        .timeout_ms = 200,
        .retries = 5,
    };
    // Encode a message
    int8_t txStatus = protocol_encode_msg(pBuf, pMsg, &options);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OK, txStatus, "Encoding failed\r\n");
    // Try to decode the message
    comms_options_t rxOptions = {
        .msgLen = 0,
        .messageType = 0,
        .bUseChecksum = false,
        .bUseCRC = false,
        .timeout_ms = 0,
        .retries = 0,
    };
    ;
    char pRxBuf[255] = {0};
    int8_t rxStatus = protocol_decode_msg(pBuf, pRxBuf, &rxOptions);
    TEST_ASSERT_EQUAL_INT_MESSAGE(OK, rxStatus, "Decoding failed\r\n");

    // Check that the received string is the same:
    TEST_ASSERT_EQUAL_STRING(pMsg, pRxBuf);

    // Check the settings
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.msgLen, rxOptions.msgLen,
                                  "Received incorrect msglen\r\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.messageType, rxOptions.messageType,
                                  "Received incorrect messageType\r\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.bUseChecksum, rxOptions.bUseChecksum,
                                  "Received incorrect bUseChecksum\r\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.bUseCRC, rxOptions.bUseCRC,
                                  "Received incorrect bUseCRC\r\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.timeout_ms, rxOptions.timeout_ms,
                                  "Received incorrect timeout_ms\r\n");
    TEST_ASSERT_EQUAL_INT_MESSAGE(options.retries, rxOptions.retries,
                                  "Received incorrect retries\r\n");
}