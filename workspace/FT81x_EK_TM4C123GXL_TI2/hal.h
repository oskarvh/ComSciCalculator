/*
 * hal.h
 *
 * Platform dependent abstraction layer.
 * NOTE: You need to fill in your platform specific
 * functions here. So any pin-mapping and function
 * wrapping happens here.
 * This is written for the TI TM4C family
 * micro-controllers, but if another brand is used then
 * the functions needs to be changed.
 *  Created on: 4 maj 2023
 *      Author: oskar
 */

#ifndef HAL_H_
#define HAL_H_
//! Supported platforms
#define TI_TM4C 1

//! Define for this platform to be used.
#define PLATFORM TI_TM4C
//! SPI frequency in MHz. Max 11 MHz for settings, max 30 MHz for active
#define SPI_FREQ_SETTINGS 10
#define SPI_FREQ_ACTIVE 30

// Platform independent includes
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Struct holding the hal state and functions
 */
typedef struct hal{
    /**
     * @param initd True if HAL is initialized, false if not
     */
    bool initd;
    /**
     * @param pSpiHandle Pointer to the HAL SPI handle
     * This handle is used for the other SPI functions
     * if needed, for any platform specific HAL functions
     * to handle the state.
     */
    void *pSpiHandle;
    /**
     * @param pGpioHandle Pointer to the HAL GPIO handle
     * This handle is used for the other GPIO functions
     * if needed, for any platform specific HAL functions
     * to handle the state.
     */
    void *pGpioHandle;
}hal_t;

/**
 * @brief HAL init function
 * @returns The initialized HAL struct
 *
 * Initializes all the hardware needed for the FT81x
 * I.e. SPI and GPIO's
 */
hal_t hal_init(void);

/**
 * @brief Function to send data over SPI
 * @param pHal Pointer to the HAL struct
 * @param ptxData Pointer to which data to send
 * @param pRxData Pointer to which data to receive
 * @param len How many bytes to send
 *
 * Sending the SPI data, given as a uint8_t pointer
 */
void hal_spi(hal_t *pHal, uint8_t *pTxData, uint8_t *pRxData, uint8_t len);

/**
 * @brief Function to sleep/wait
 * @param us Time to sleep, in microseconds
 *
 * This should supports threading, so that while the thread is
 * sleeping or waiting, other commands should be able to execute
 */
void hal_sleep(uint32_t us);


void inline hal_spi_set_cs(uint8_t cs_n);
void hal_spi_no_cs(hal_t *pHal, uint8_t *pTxData, uint8_t *pRxData, uint8_t len);

#endif /* HAL_H_ */
