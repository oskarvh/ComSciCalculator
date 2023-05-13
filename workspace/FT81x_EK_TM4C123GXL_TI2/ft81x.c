/*
 * ft81x.c
 * General driver for the FT81x series graphics
 * controller. Based on the programmers guide:
 * https://brtchip.com/wp-content/uploads/Support/Documentation/Programming_Guides/ICs/EVE/FT81X_Series_Programmer_Guide.pdf
 *  Created on: 4 maj 2023
 *      Author: Oskar von Heideken
 */

// FT81x header (includes defines etc.)
//#include "ft81x.h"
#include <string.h>
#include <unistd.h>

// Include RudolphRieder's FT81x driver:
#include "../software/FT800-FT813/EVE.h"
#include "../software/FT800-FT813/EVE_commands.h"
#include "../software/FT800-FT813/EVE_config.h"
#include "../software/FT800-FT813/EVE_target.h"

// Include the comSciCalc core
#include "../software/comSciCalc_lib/comscicalc.h"

/**
 * @brief Send a 3 byte host command (see datasheet)
 * @param pHal Pointer to the HAL object
 * @param command Which command to be sent
 * @param param Which parameter should be sent with that command
 *
 * Since this sends a 3 byte command, the MSB will always be ignored.
 * A host command is (almost) always initiated by sending 0b01 followed
 * by the command. The exception is STANDBY, which is all zeros for
 * some reason, and the FT81x reads from dummy memory causing a
 * STANDBY command. Yeah..
 */
static void ft81x_hostCommand(hal_t *pHal, uint8_t command, uint8_t param){
    // Make a temporary buffer to hold the data. MSB first
    uint8_t pData[3];
    pData[0] = command; // This is sent first.
    pData[1] = param;
    pData[2] = 0; // Last byte is always zero in host commands
    // Send the host command
    hal_spi(pHal, pData, NULL, 3);
}

/**
 * @brief Send up to N bytes of data
 * @param pHal Pointer to the HAL object
 * @param address Address to which data should be written
 * @param data Data to be written to that address
 * @param len How many bytes should be sent
 *
 * Data is sent as address | data, where the address is bitmasked
 */
static void ft81x_write(hal_t *pHal, uint32_t address, uint8_t *pData, uint8_t len){
    // Make the bitmasked command based on the address
    uint32_t cmd = address | FT81X_WR_BITMASK;

    // Allocate some space for a temporary buffer.
    // Address is always 3 bytes, but data length can vary up to 4 bytes
    uint8_t *pTempBuf = malloc(3+len);

    pTempBuf[0] = cmd>>16;
    pTempBuf[1] = cmd>>8;
    pTempBuf[2] = cmd;

    hal_spi_set_cs(0);
    hal_spi_no_cs(pHal, pTempBuf, NULL, 3);
    // Send the data
    hal_spi_no_cs(pHal, pData, NULL, len);
    hal_spi_set_cs(1);
}

/**
 * @brief Send up to 4 bytes of data. Special case of the write
 * @param pHal Pointer to the HAL object
 * @param address Address to which data should be written
 * @param data Data to be written to that address
 * @param len How many bytes should be sent
 *
 * Data is sent as address | data, where the address is bitmasked
 */
static void ft81x_write32(hal_t *pHal, uint32_t address, uint32_t data, uint8_t len){
    ft81x_write(pHal, address, (uint8_t*)(&data), len);
}



/**
 * @brief Read data from the FT81x chip
 * @param pHal Pointer to the HAL object
 * @param address Address to which data should read from
 * @param pRxBuf Pointer to RX buffer
 * @param len Maximum length of the RX buffer
 *
 * Address is sent as address | data, where the address is bitmasked
 */
static uint32_t ft81x_read(hal_t *pHal, uint32_t address, uint8_t *pRxBuf, uint8_t len){
    // If the RX buffer is null, then temporarily allocate a buffer for it
    bool bAllocatedRxBuf = false;
    if(pRxBuf == NULL){
        pRxBuf = malloc(4+len);
        bAllocatedRxBuf = true;
    }

    // Make the bitmasked command based on the address
    uint32_t cmd = address | FT81X_RD_BITMASK;

    // Allocate some space for a temporary buffer for the address along
    // with enough dummy bytes to read the length of the message
    uint8_t *pTempBuf = malloc(4+len);
    memset(pTempBuf, 0, 4 + len);
    pTempBuf[0] = cmd>>16;
    pTempBuf[1] = cmd>>8;
    pTempBuf[2] = cmd;

    // Note: the four first bytes are just garbage data..
    // Initiate the transfer
    hal_spi(pHal, pTempBuf, pRxBuf, 4+len);

    // Free the temporary buffer
    free(pTempBuf);

    // Copy the last len bytes to the return value, and return that
    uint32_t returnVal = 0;
    if(len <= 4){
        memcpy(&returnVal, &pRxBuf[4], len);
    }
    // Free the allocated RX buffer if this was allocated
    if(bAllocatedRxBuf){
        free(pRxBuf);
    }
    return returnVal;
}

/**
 * @brief Set resolution of the screen
 * @param pHal Pointer to the HAL object
 * @param pScreen Pointer to display settings struct
 *
 * The programmers guide and data sheet not great when it comes
 * explaining what each register actually mean, so some guess work is
 * involved here. Or more exact, the examples that the programmers
 * guide gives makes no sense whatsoever. Or is it the datasheet that
 * does not make sense? Who knows, they don't contain the same information.
 * This is at least going of table 4.13 and figure 4.7 in the datasheet.
 */
static void ft81x_setResolution(hal_t *pHal, display_t *pDisplay){
    // Set the total amount of pixels per line (sum of front and back porches and
    // visible area and sync pulses)
    ft81x_write32(pHal, FT81X_REG_HCYCLE, pDisplay->horzFrontPorch +
               pDisplay->horzRes +
               pDisplay->horzBackPorch +
               pDisplay->horzSyncWidth, 2);
    // 32 + 800 + 26 + 96 = 954 = 0x03BA. Sent BA->03, so OK.

    // Set the horizontal non-visible part of the display
    ft81x_write32(pHal, FT81X_REG_HOFFSET, pDisplay->horzFrontPorch +
               pDisplay->horzBackPorch +
               pDisplay->horzSyncWidth, 2);
    // 32 + 26 + 96 = 154 = 0x9A

    // Set the horizontal front porch length:
    ft81x_write32(pHal, FT81X_REG_HSYNC0, pDisplay->horzFrontPorch, 2);
    // 32 = 0x20

    // Set the horizontal sync pulse plus front porch length:
    ft81x_write32(pHal, FT81X_REG_HSYNC1, pDisplay->horzFrontPorch +
               pDisplay->horzSyncWidth, 2);


    // Set the total amount vertical pixels, or lines
    ft81x_write32(pHal, FT81X_REG_VCYCLE, pDisplay->vertFrontPorch +
                   pDisplay->vertRes +
                   pDisplay->vertBackPorch +
                   pDisplay->vertSyncWidth, 2);

    // Set the vertical non-visible part of the display
    ft81x_write32(pHal, FT81X_REG_VOFFSET, pDisplay->vertFrontPorch +
               pDisplay->vertBackPorch +
               pDisplay->vertSyncWidth, 2);

    // Set the vertical front porch length:
    ft81x_write32(pHal, FT81X_REG_VSYNC0, pDisplay->vertFrontPorch, 2);

    // Set the vertical sync pulse plus front porch length:
    ft81x_write32(pHal, FT81X_REG_VSYNC1, pDisplay->vertFrontPorch + pDisplay->vertSyncWidth, 2);

    // Set the "swizzle", or to which pins the RGB data should be shuffled out
    ft81x_write32(pHal, FT81X_REG_SWIZZLE, FT81X_SWIZZLE_RGB_MSB, 1);

    // Set the pixel clock polarity. 0 = rising edge, 1 = falling edge
    ft81x_write32(pHal, FT81X_REG_PCLK_POL, pDisplay->pclk_polarity, 1);

    // Output clock spreading. 0 = disables, 1 = enabled
    // This spreads out the R, G and B lines switching within the
    // clock cycle, reducing switching noise. On per default.
    ft81x_write32(pHal, FT81X_REG_CSPREAD, 1, 1);

    // Set the horizontal resolution
    ft81x_write32(pHal, FT81X_REG_HSIZE, pDisplay->horzRes, 2);

    // Set the vertical resolution
    ft81x_write32(pHal, FT81X_REG_VSIZE, pDisplay->vertRes, 2);

}

/**
 * @brief Sets the DISP pin on the FT81x high without altering other pins
 * @param pHal Pointer to the HAL object
 *
 */
static void ft81x_enable_disp_pin(hal_t *pHal){
    // Read the GPIO direction register and write back to enable
    // the DISP pin
    uint8_t regGpioDir = ft81x_read(pHal, FT81X_REG_GPIO_DIR, NULL, 1);
    ft81x_write32(pHal, FT81X_REG_GPIO_DIR, 0x80 | regGpioDir, 1);
    uint8_t regGpio = ft81x_read(pHal, FT81X_REG_GPIO, NULL, 1);
    ft81x_write32(pHal, FT81X_REG_GPIO, 0x80 | regGpio, 1);
}


ft81x_status_t ft81x_init(hal_t *pHal, display_t *pDisplay){

    // Check if HAL has been initialized
    if(!pHal->initd){
        return ft81x_halNotInitd;
    }

    // Send a reset pulse (wait some time for done)
    ft81x_hostCommand(pHal, FT81X_CMD_RST_PULSE, 0);
    // Wait for 300 ms - TBD: can we wait less time?
    hal_sleep(300000);

    // Set the clock to internal or external reference
#ifdef FT81X_EXTCLK
    ft81x_hostCommand(pHal, FT81X_CMD_CLKEXT, 0);
#else
    ft81x_hostCommand(pHal, FT81X_CMD_CLKINT, 0);
#endif
    // Wait for 300 ms - TBD: can we wait less time?
    hal_sleep(300000);

    // Set the ft81x in active state
    ft81x_hostCommand(pHal, FT81X_CMD_ACTIVE, 0);

    // After active is sent, the FT81x executes various
    // BIST's and diagnostics. This can take up to 300 ms
    // Wait here until REG_ID is set to 0x7C, and CPU_RESET is
    // set to 0. Poll these every 10 ms
    bool ft81x_boot_completed = false;
    while(!ft81x_boot_completed){
        // Read the REG_ID and CPURESET registers
        uint8_t reg_id = ft81x_read(pHal, FT81X_REG_ID, NULL, 1);
        uint8_t cpu_reset = ft81x_read(pHal, FT81X_REG_CPURESET, NULL, 1);
        if(reg_id == 0x7C && cpu_reset == 0x00){
            // Boot is completed, exit loop and continue with setup
            ft81x_boot_completed = true;
        }
        else{
            // Boot is not completed.
            // Sleep for 10 ms to enable system to do other tasks
            hal_sleep(10000);
        }

    }

    // Set the display resolution
    ft81x_setResolution(pHal, pDisplay);

    // Reset value should be zero so shouldn't be a need to set this
    //ft81x_write32(pHal, FT81X_RAM_DL + 0, FT81X_DLSTART(), 4);
#ifndef boring
/*
    // Begin display list
    ft81x_write32(pHal, FT81X_RAM_CMD + 0, FT81X_DLSTART(), 4);
    // Set background to black
    ft81x_write32(pHal, FT81X_RAM_CMD + 4, FT81X_CLEAR_COLOR_RGB(0,0,255), 4);

    // Clear all buffers to preset value
    ft81x_write32(pHal, FT81X_RAM_CMD + 8,FT81X_CLEAR(1,1,1), 4);

    // Close out display list
    ft81x_write32(pHal, FT81X_RAM_CMD + 12, FT81X_END_OF_DISPLAY_LIST, 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + 16, FT81X_SWAP(), 4);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, 20&0xfff, 4);
*/

    uint32_t ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) & 0xFFF;
    uint32_t cmdBuf[5];
    cmdBuf[0] = FT81X_DLSTART();
    cmdBuf[1] = FT81X_CLEAR_COLOR_RGB(0,0,255);
    cmdBuf[2] = FT81X_CLEAR(1,1,1);
    cmdBuf[3] = FT81X_END_OF_DISPLAY_LIST;
    cmdBuf[4] = FT81X_SWAP();
    ft81x_write(pHal, FT81X_RAM_CMD + ft81x_dlAddr, (uint8_t*)cmdBuf, 5*4);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, 5*4, 4);

#else
    ft81x_write32(pHal, FT81X_RAM_CMD + 0, FT81X_DLSTART(), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD+4,FT81X_CLEAR(1,1,1), 4);
    //wr32(RAM_DL + 4, BEGIN(BITMAPS)); // start drawing bitmaps
    ft81x_write32(pHal, FT81X_RAM_CMD+8,BEGIN(BITMAPS), 4);
    //wr32(RAM_DL + 8, VERTEX2II(220, 110, 31, 'F')); // ascii F in font 31
    //wr32(RAM_DL + 12, VERTEX2II(244, 110, 31, 'T')); // ascii T
    //wr32(RAM_DL + 16, VERTEX2II(270, 110, 31, 'D')); // ascii D
    //wr32(RAM_DL + 20, VERTEX2II(299, 110, 31, 'I')); // ascii I
    ft81x_write32(pHal, FT81X_RAM_CMD+12 , VERTEX2II(220, 110, 31, 'P'), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD+16, VERTEX2II(244, 110, 31, 'U'), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD+20, VERTEX2II(270, 110, 31, 'N'), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD+24, VERTEX2II(299, 110, 31, 'G'), 4);
    //wr32(RAM_DL + 24, END());
    ft81x_write32(pHal, FT81X_RAM_CMD+28,END(), 4);
    //wr32(RAM_DL + 28, COLOR_RGB(160, 22, 22)); // change colour to red
    ft81x_write32(pHal, FT81X_RAM_CMD+32, FT81X_COLOR_RGB(160, 22, 22), 4);
    //wr32(RAM_DL + 32, POINT_SIZE(320)); // set point size to 20 pixels in radius
    ft81x_write32(pHal, FT81X_RAM_CMD+36, POINT_SIZE(320), 4);
    //wr32(RAM_DL + 36, BEGIN(POINTS)); // start drawing points
    ft81x_write32(pHal, FT81X_RAM_CMD+40, BEGIN(POINTS), 4);
    //wr32(RAM_DL + 40, VERTEX2II(192, 133, 0, 0)); // red point
    ft81x_write32(pHal, FT81X_RAM_CMD+44, VERTEX2II(192, 133, 0, 0), 4);
    //wr32(RAM_DL + 44, END());
    ft81x_write32(pHal, FT81X_RAM_CMD+48,END(), 4);
    //wr32(RAM_DL + 48, DISPLAY()); // display the image
    ft81x_write32(pHal, FT81X_RAM_CMD+52,FT81X_END_OF_DISPLAY_LIST, 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + 56, FT81X_SWAP(), 4);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, 60&0xfff, 4);
#endif
    // Swap display list after frame
    //ft81x_write32(pHal, FT81X_REG_DLSWAP, FT81X_DLSWAP_FRAME, 1);

    // Set the DISP pin high to enable the display
    ft81x_enable_disp_pin(pHal);

    // Enable the pixel clock. After this, display is activated.
    // This programs the PCLK divisor, so 1 is highest frequency,
    // 0 is deactivated. See Table 4.11 in datasheet for details
    ft81x_write32(pHal, FT81X_REG_PCLK, 5, 1);

    return ft81x_success;
}

void ft81x_begin_display_list(hal_t *pHal){
    // Wait for the command buffer
    while(ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) != ft81x_read(pHal, FT81X_REG_CMD_READ, NULL, 3)){
        // Poll every 100 us
        hal_sleep(100);
    }
    uint32_t ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) & 0xFFF;
    uint32_t ft81x_dlReadAddr = ft81x_read(pHal, FT81X_REG_CMD_READ, NULL, 2);
    // Start the display list clear it.
    // THIS DOES NOT WORK?!?!?!? WTF?
    //ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr, FT81X_DLSTART(), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 0, FT81X_CLEAR(1,1,1), 4);
    // Clear
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 4, FT81X_CLEAR_COLOR_RGB(255,0,0), 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 8, FT81X_CLEAR(1,1,1), 4);
    // Swap screen
    //ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 16, FT81X_END_OF_DISPLAY_LIST, 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 12, FT81X_SWAP(), 4);
    //ft81x_write32(pHal, FT81X_REG_DLSWAP, FT81X_DLSWAP_FRAME, 1);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, 16&0xfff, 4);

    // Update the CMD WRITE register:

    while(1){
        ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 4) & 0x1FFF;
        ft81x_dlReadAddr = ft81x_read(pHal, FT81X_REG_CMD_READ, NULL, 2);
    }

}


uint8_t initBitmapHandleForFont(uint8_t font) {
    if (font > 31) {
        startCmd(ROMFONT());
        intermediateCmd(14);
        endCmd(font);
        return 14;
    }
    return font;
}
/*
void ft81x_drawText(const int16_t x, const int16_t y, const uint8_t font, const uint32_t color, const uint16_t options, const char text[]){
    uint8_t fontHandle = font;
    uint32_t ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) & 0xFFF;
    if (font > 31) {
        //startCmd(ROMFONT());
        ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr, ROMFONT(), 4);
        //intermediateCmd(14);
        ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 4, 14, 4);
        //endCmd(font);
        ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 8, fontHandle, 4);
        fontHandle = 14;
        ft81x_write32(pHal, FT81X_REG_CMD_WRITE, ft81x_dlAddr+8, 4);
    }
    ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) & 0xFFF;
    //startCmd(COLOR(color));
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr, COLOR(color), 4);
    //intermediateCmd(LOADIDENTITY());
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 4, LOADIDENTITY(), 4);
    //intermediateCmd(SCALE());
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 8, SCALE(), 4);
    //intermediateCmd(1 * 65536);
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 12, 1 * 65536, 4);
    //intermediateCmd(1 * 65536);
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 16, 1 * 65536, 4);
    //intermediateCmd(SETMATRIX());
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 20, SETMATRIX(), 4);
    //intermediateCmd(TEXT());
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 24, TEXT(), 4);
    //intermediateCmd(x | ((uint32_t)y << 16));
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 28, x | ((uint32_t)y << 16), 4);
    //intermediateCmd(fontHandle | ((uint32_t)options << 16));
    ft81x_write32(pHal, FT81X_RAM_CMD + ft81x_dlAddr + 32, fontHandle | ((uint32_t)options << 16), 4);
    sendText(text);
}
*/

void ft81x_draw_test_image(hal_t *pHal){
    //waitForCommandBuffer();
    while(ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) != ft81x_read(pHal, FT81X_REG_CMD_READ, NULL, 3)){
        // Poll every 100 us
        hal_sleep(100);
    }
    // Get the start address:
    uint32_t ft81x_dlAddr = ft81x_read(pHal, FT81X_REG_CMD_WRITE, NULL, 3) & 0xFFF;
    uint32_t cmdBuf[5];
    cmdBuf[0] = FT81X_DLSTART();
    cmdBuf[1] = FT81X_CLEAR_COLOR_RGB(0,255,0);
    cmdBuf[2] = FT81X_CLEAR(1,1,1);
    cmdBuf[3] = FT81X_END_OF_DISPLAY_LIST;
    cmdBuf[4] = FT81X_SWAP();
    ft81x_write(pHal, FT81X_RAM_CMD + ft81x_dlAddr, (uint8_t*)cmdBuf, 5*4);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, ft81x_dlAddr + 5*4, 4);
    /*
    // Begin display list
    ft81x_write32(pHal, FT81X_RAM_CMD + 0, FT81X_DLSTART(), 4);
    // Set background to black
    ft81x_write32(pHal, FT81X_RAM_CMD + 4, FT81X_CLEAR_COLOR_RGB(0,0,0), 4);

    // Clear all buffers to preset value
    ft81x_write32(pHal, FT81X_RAM_CMD + 8,FT81X_CLEAR(1,1,1), 4);

    // Close out display list
    ft81x_write32(pHal, FT81X_RAM_CMD + 12, FT81X_END_OF_DISPLAY_LIST, 4);
    ft81x_write32(pHal, FT81X_RAM_CMD + 16, FT81X_SWAP(), 4);
    ft81x_write32(pHal, FT81X_REG_CMD_WRITE, 20&0xfff, 4);
    */


}
