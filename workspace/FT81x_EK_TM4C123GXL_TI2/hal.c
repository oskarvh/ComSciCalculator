/*
 * hal.c
 *
 * Platform dependent abstraction layer
 * This file glues SPI driver together to
 * the hardware specific drivers
 *  Created on: 4 maj 2023
 *      Author: oskar
 */
// Platform independent includes
#include "hal.h"

#if PLATFORM == TI_TM4C
// TI specific HAL
#include <ti/drivers/SPI.h>
#include <ti/drivers/PWM.h>
#include <ti/drivers/GPIO.h>
#include <driverlib/sysctl.h>
#include <driverlib/gpio.h>
#include <driverlib/spi.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>
#include "Board.h"
#include <unistd.h>
#endif

#if PLATFORM == TI_TM4C
// TI specific global variables
PWM_Handle backLightPwmHandle;

#endif
// Platform independent global variables



hal_t hal_init(void){
    hal_t hal;
#if PLATFORM == TI_TM4C
    // Initialize the SPI
    SPI_Params spiParams;
    SPI_Params_init(&spiParams);
    // Set the bitrate to 11MHz as default.
    // This can be increased later on
    spiParams.bitRate = 5000000;
    spiParams.mode = SPI_MASTER;
    //spiParams.frameFormat = SPI_POL0_PHA0;
    // Initialize SPI handle as default master
    spiHandle = SPI_open(Board_SPI0, &spiParams);

    // Enable PWM peripherals
    // NOTE: Need to change the PWM_config table in the board file EK_TM4C123GXL.
    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
    // Enable PWM on PA5
    GPIOPinConfigure(GPIO_PB5_M0PWM3);
    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_5);
    // Init the PWM module
    PWM_init();
    PWM_Params params;
    PWM_Params_init(&params);
    params.period = 255;// Period and duty in microseconds
    params.polarity = PWM_POL_ACTIVE_HIGH; // Set active low as we pull the backlight down.
    backLightPwmHandle = PWM_open(Board_PWM3, &params);

    // Populate the stuff needed for the hal
    hal.pSpiHandle = &spiHandle;
    hal.pGpioHandle = &backLightPwmHandle;
#endif
    hal.initd = true;

    return hal;
}

void hal_spi_no_cs(hal_t *pHal, uint8_t *pTxData, uint8_t *pRxData, uint8_t len){
#if PLATFORM == TI_TM4C
    SPI_Transaction transaction;
    transaction.count = len;
    transaction.rxBuf = (void *) pRxData;
    transaction.txBuf = (void *) pTxData;
    // Send command
    SPI_transfer(spiHandle, &transaction);
#endif
}

void inline hal_spi_set_cs(uint8_t cs_n){
#if PLATFORM == TI_TM4C
    // Drive manual CS low:
    GPIO_write(GPIO_CS_PIN, cs_n);
#endif
}

void hal_spi(hal_t *pHal, uint8_t *pTxData, uint8_t *pRxData, uint8_t len){
    // Drive manual CS low:
    hal_spi_set_cs(0);
    // Send command
    hal_spi_no_cs(pHal, pTxData, pRxData, len);
    // Drive the CS high for end of command.
    hal_spi_set_cs(1);
}



void hal_sleep(uint32_t us){
    usleep(us);
}
