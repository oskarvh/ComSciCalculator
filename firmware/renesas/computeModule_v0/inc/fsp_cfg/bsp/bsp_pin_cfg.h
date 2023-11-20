/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define ARDUINO_A0_MIKROBUS_AN (BSP_IO_PORT_00_PIN_00)
#define ARDUINO_A1 (BSP_IO_PORT_00_PIN_01)
#define ARDUINO_A2 (BSP_IO_PORT_00_PIN_03)
#define ARDUINO_A4 (BSP_IO_PORT_00_PIN_14)
#define ARDUINO_A5 (BSP_IO_PORT_00_PIN_15)
#define SCI_MISO_0 (BSP_IO_PORT_01_PIN_00)
#define SCI_MOSI_0 (BSP_IO_PORT_01_PIN_01)
#define SCI_SCK_0 (BSP_IO_PORT_01_PIN_02)
#define SCI_SS_0 (BSP_IO_PORT_01_PIN_03)
#define ARDUINO_D2 (BSP_IO_PORT_01_PIN_05)
#define ARDUINO_D3 (BSP_IO_PORT_01_PIN_11)
#define ARDUINO_SS_MIKCRBUS_SS (BSP_IO_PORT_02_PIN_05)
#define PMOD1_SS1 (BSP_IO_PORT_02_PIN_06)
#define PMOD1_SS2 (BSP_IO_PORT_02_PIN_07)
#define SCI_SCL2_KBD (BSP_IO_PORT_03_PIN_01)
#define SCI_SDA2_KBD (BSP_IO_PORT_03_PIN_02)
#define KBD_RST (BSP_IO_PORT_03_PIN_03)
#define KBD_INTn (BSP_IO_PORT_03_PIN_04)
#define BACKLIGHT_PWM (BSP_IO_PORT_04_PIN_00)
#define FT811_INTn (BSP_IO_PORT_04_PIN_01)
#define FT811_PDn (BSP_IO_PORT_04_PIN_02)
#define USB_VBUS (BSP_IO_PORT_04_PIN_07)
#define SCI_SCL3_LED (BSP_IO_PORT_04_PIN_08)
#define SCI_SDA3_LED (BSP_IO_PORT_04_PIN_09)
#define LED_SDBn (BSP_IO_PORT_04_PIN_10)
#define USB_VBUS_EN (BSP_IO_PORT_05_PIN_00)
extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA4M3AF3CFM */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER

#endif /* BSP_PIN_CFG_H_ */
