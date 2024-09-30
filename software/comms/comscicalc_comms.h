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

/*
 * This header file defines the types used in the comSciCalc communication protocol. 
 * See the readme file for more info.
 */

// Standard library
#include <stdint.h>
#include <stdbool.h>
// Extended library

/*
 * DEFINES AND MACROS
 */
//! Start of Message
#define SOM 0x1b5f9f

/**
 * @defgroup Protocol layer message types
 * @{
 */
//! Message type: Data transfer
#define MT_DT_BITMASK 0x01
//! Message type: ACK
#define MT_ACK_BITMASK 0x02
//! Message type: NACK
#define MT_NACK_BITMASK 0x04
//! Message type: Retransmission
#define MT_RT_BITMASK 0x8
/**@}*/

/**
 * @defgroup Protocol layer settings
 * @{
 */
//! Settings: Use checksum
#define ST_USE_CHECKSUM_BITMASK 0x000001
//! Settings: Use CRC
#define ST_USE_CRC_BITMASK 0x000002
//! Settings: Timeout bitmask
#define ST_TIMEOUT_BITMASK 0x000FF0
//! Settings: Retries bitmask
#define ST_RETRIES_BITMASK 0x0FF000
/**@}*/

/**
 * @defgroup Function return statuses
 * @{
 */
#define OK 0
#define HEADER_FAULT -1
#define NOK -2
#define NULL_PTR -3
#define CHECKSUM_ERROR -4
#define CRC_ERROR -5
/**@}*/

//! Macro to retrieve the timeout settings in millisec from settings
#define GET_TIMEOUT_MS(settings) (pow(10,((settings>>4)&0x3))*((settings>>6)&0x3f))
//! Macro to retrieve the retries
#define GET_RETRIES(settings) ((settings>>12)&0xFF)


/*
 * TYPES
 */
/**
 * @brief Enum typedef for message types
 */
typedef enum {
    dataTransfer = 0x01,
    ack = 0x02,
    nack = 0x04,
    retransmission = 0x08,
} messageTypes_t;

/**
 * @brief Internal options struct. 
 * This is not sent directly, but rather a common 
 * interface to read and write settings
 * from/to a protocol function.
 */
typedef struct comms_options {
    //! Length of the received message
    uint8_t msgLen;
    //! Message type
    uint8_t messageType;
    //! Use Checksum
    bool bUseChecksum;
    //! Use CRC
    bool bUseCRC;
    //! Timeout in ms
    uint16_t timeout_ms;
    //! Retries
    uint8_t retries;
}comms_options_t;

/**
 * @brief Link layer mesage type enumeration
 */
typedef enum {
    //! Undefined
    not_defined = -1,
    //! Data input
    data_input = 1,
    //! Request for state
    state_request = 2,
    //! State
    state = 3,
    //! Data section. For writing to memory
    write_section = 4,
    //! Data section. For reading memory
    read_section = 5,
} linkLayerDataype_t;

/**
 * @brief Link layer header
 */
typedef struct linkLayerHeader
{
    //! Message type
    int32_t messageType;
} linkLayerHeader_t;


/**
 * @brief Communication protocol header
 * This struct is always static, whereas the checksum can vary
 */
typedef struct comms_header {
    //! Start of message
    uint32_t som : 24;
    //! Message length
    uint32_t ml : 8;
    //! Message type
    uint32_t mt : 8;
    //! Message settings
    uint32_t settings: 24;
} comms_header_t;



/*
 * VARIABLES
 */

/*
 * FUNCTION PROTOTYPES
 */
/**
 * @brief Initialize the comms
 */
void initComms(void);
/**
 * @brief Decode a protocol layer message
 * @param pMsg Pointer to incoming data, i.e. message to be decoded
 * @param msgLen Length of data
 * @param pData Pointer to where to place the data for the link layer
 * @param pOptions Pointer to options struct, written to
 * @return Status
 */
int8_t protocol_decode_msg(void *pMsg, void *pData, comms_options_t *pOptions);
/**
 * @brief Encode a protocol layer message
 * @param pOutgoing Pointer to the outgoing buffer
 * @param pMsg Pointer to the data coming from the link layer
 * @param pOptions Pointer to options struct, read from
 * @return Status
 */
int8_t protocol_encode_msg(void *pOutgoing, void *pMsg, comms_options_t *pOptions);

/**
 * @brief Get link layer data type
 * @param pData Pointer to the (beginning of the) link layer data
 * @return Data type (linkLayerMessageType)
 */
inline linkLayerHeader_t link_get_data_type(void *pData);