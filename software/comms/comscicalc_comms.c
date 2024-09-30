/*
MIT License

Copyright (c) 2024 Oskar von Heideken

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Standard library
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// Extended library

// Project includes
#include "comscicalc_comms.h"
#include "crc.h"

void initComms(void){
    // Initialize the CRC table
    crcInit();
}

/*
 * PROTOCOL LAYER FUNCTIONS
 */

// Function to sanity check the header
bool check_header(comms_header_t *pHeader){
    if(pHeader->som != SOM){
        // Incorrect start of message
        return false;
    }
    if(pHeader->mt == 0 && pHeader->ml != 0){
        // Must have message type if message lenth is non-zero
        return false;
    }
    return true;
}

int8_t protocol_decode_msg(void *pMsg, void *pData, comms_options_t *pOptions){
    // This function decodes and parses a message pointed to by pMsg, 
    // with length msgLen
    if(pMsg == NULL){
        return NULL_PTR;
    }
    uint16_t totalPacketLen = 8;
    uint8_t status = OK;
    // The header comes first
    comms_header_t *pHeader = malloc(sizeof(comms_header_t));
    memcpy(pHeader, pMsg, sizeof(comms_header_t));
    // Sanity check the header
    if(!check_header(pHeader)){
        // There something wrong with the header. 
        return HEADER_FAULT;
    }
    pOptions->msgLen = pHeader->ml;
    pOptions->messageType = pHeader->mt;
    pOptions->timeout_ms = GET_TIMEOUT_MS(pHeader->settings);
    pOptions->retries = GET_RETRIES(pHeader->settings);
    
    // Handle the data portion
    if(pHeader->ml > 0){
        if(pData == NULL){
            status = NULL_PTR;
        }
        else{
            memcpy(pData, &(pMsg[8]), pHeader->ml);
            totalPacketLen += pHeader->ml;
        }
    }
    
    // Check the CRC and checksum
    if(pHeader->settings & ST_USE_CHECKSUM_BITMASK == ST_USE_CHECKSUM_BITMASK){
        // Check the checksum
        pOptions->bUseChecksum = true;
        totalPacketLen += 4;
        // TODO, compare calculated checksum to received checksum, and set status accordingly
    }
    if((pHeader->settings & ST_USE_CRC_BITMASK) == ST_USE_CRC_BITMASK){
        // Check the CRC
        pOptions->bUseCRC = true;
        uint32_t calculatedCrc = crcFast(pMsg, totalPacketLen);
        uint32_t receivedCrc = 0;
        memcpy(&receivedCrc, &(pMsg[totalPacketLen]), sizeof(uint32_t));
        if(receivedCrc != calculatedCrc){
            status = CRC_ERROR;
        }
    }
    return status;
}

/**
 * @brief Constructs the settings field in the header
 * @param timeout_ms Timeout to wait for ACK in ms
 * @param bUseChecksum True if checksum is used, false if not.
 * @param bUseCRC True if CRC is used, false if not.
 * @return settings field
 */
static uint32_t constructSettings(uint16_t timeout_ms, uint8_t retries, bool bUseChecksum, bool bUseCRC){
    uint32_t settingsField = 0;
    if(bUseChecksum){
        settingsField |= ST_USE_CHECKSUM_BITMASK;
    }
    if(bUseCRC){
        settingsField |= ST_USE_CRC_BITMASK;
    }
    // Timeout base is either 1, 10, 100 or 1000
    uint16_t timeoutBase = 0;
    if(timeout_ms < 10){
        timeoutBase = 0;
    }
    else if (timeout_ms < 100){
        timeoutBase = 1;
    }
    else if (timeout_ms < 1000){
        timeoutBase = 2;
    }
    else{
        timeoutBase = 3;
    }
    // Timeout multiplier is generally 0-9
    uint16_t timeoutMultiplier = 0;
    if(timeoutBase == 0){
        timeoutMultiplier = timeout_ms;
    }
    else {
        timeoutMultiplier = timeout_ms / pow(10,timeoutBase);
    }
    // Add timeout field to settings
    settingsField |= ((timeoutBase << 4) | (timeoutMultiplier << 6))&ST_TIMEOUT_BITMASK;

    // Add the number of retries
    settingsField |= retries << 12;

    return settingsField;
}

int8_t protocol_encode_msg(void *pOutgoing, void *pMsg, comms_options_t *pOptions){
    // Constructs a protocol message to be placed in pOutgoing. 
    // Local variable to keep track of the outgoing message. 
    char *pData = pOutgoing;
    if(pData == NULL){
        return NULL_PTR;
    }
    // Construct the header
    comms_header_t header = {
        .som = SOM,
        .mt = pOptions->messageType,
        .ml = pOptions->msgLen,
        .settings = constructSettings(pOptions->timeout_ms, pOptions->retries, pOptions->bUseChecksum, pOptions->bUseCRC),
    };
    uint16_t totalPacketLen = 8; 
    // Copy the header over to the data
    memcpy(pData, &header, 8);
    pData = &(pData[8]);

    // Copy over the data itself
    if(pMsg != NULL && pOptions->msgLen > 0){
        memcpy(pData, pMsg, pOptions->msgLen);
        pData = &(pData[pOptions->msgLen]);
        totalPacketLen += pOptions->msgLen;
    }

    if(pOptions->bUseChecksum){
        // TODO: Calculate the checksum and add it
        pData = &(pData[4]);
        totalPacketLen += 4;
    }
    if(pOptions->bUseCRC){
        // TODO: Calculate CRC and add it
        pOptions->bUseCRC = true;
        uint32_t crc = crcFast(pOutgoing, totalPacketLen);
        memcpy(pData, &crc, sizeof(uint32_t));
        pData = &(pData[4]);
    }
    return OK;
}

/*
 * LINK LAYER FUNCTIONS
 */

inline linkLayerHeader_t link_get_data_type(void *pData){
    // Since pData points to the header, and it 32 bit aligned, 
    // just cast the pointer type to linkLayerHeader_t and read out the type
    return (*((linkLayerHeader_t*)pData));
}