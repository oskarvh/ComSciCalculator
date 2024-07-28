#ifndef PIN_MAP_H
#define PIN_MAP_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"

/* ------------ DEVICE MAPPINGS ------------ */
// SPI0, used for connecting to LED
#define SPI0_MISO_PIN 0
#define SPI0_CSn_PIN 1
#define SPI0_SCLK_PIN 2
#define SPI0_MOSI_PIN 3

// I2C, Used for offboard I2C
#define SDA0_LED_PIN 4
#define SCL0_LED_PIN 5
#define LED_SDBn_PIN 16

// I2C, Used for offboard keyboard
#define SDA1_KBD_PIN 6
#define SCL1_KBD_PIN 7
#define KBD_INTn_PIN 8
#define KBD_RST_PIN 9

// UART
#define UART0_TX_PIN 12
#define UART0_RX_PIN 13

// FT81x non-SPI functions
#define FT81X_PDn_PIN 17
#define FT81X_INTn_PIN 18

// General purpose pins, reserved offboard use
#define GPIO10_PIN 10
#define GPIO11_PIN 11
#define NC_GPIO14_PIN 14
#define NC_GPIO15__PIN 15
#define GPIO19_PIN 19
#define GPIO20_PIN 20
#define GPIO21_PIN 21
#define GPIO22_PIN 22
#define GPIO23_PIN 23
#define GPIO24_PIN 24
#define GPIO25_PIN 25
#define GPIO26_PIN 26
#define GPIO27_PIN 27
#define GPIO28_PIN 28
#define GPIO29_PIN 29


/* ------------ FUNCTION MAPPINGS ------------ */
// SPI mappings to FT81x/EVE driver:
// #define EVE_CS SPI0_CSn_PIN
// #define EVE_PDN FT81X_PDn_PIN
// #define EVE_SCK SPI0_SCLK_PIN
// #define EVE_MOSI SPI0_MOSI_PIN
// #define EVE_MISO SPI0_MISO_PIN
// #define EVE_SPI spi0

// UART0 mappings and settings
#define UART0_BR 115200
#define UART0 uart0

#endif