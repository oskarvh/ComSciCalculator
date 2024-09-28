cmake_minimum_required(VERSION 3.14)

# Pull in SDK (must be before project)
include(pico-sdk/pico_sdk_init.cmake)

project(comscicalc C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# NOTE: DEBUG requires a connection to a valid USB host. 
# Comment this out if the device is to be used elsewhere. 
add_compile_definitions(DEBUG)

add_compile_definitions(RP2040)
add_compile_definitions(EVE_FT810CB_ADAFRUIT_1680)
add_compile_definitions(LOG_LEVEL=5) # 0 is the least logging

set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/inc")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/FreeRTOS-Kernel")
set(FT81X_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/FT800-FT813")
set(COMMON_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../common")
set(COMSCICALC_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/comSciCalc_lib")
set(DISPLAY_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/display")
set(HW_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/hardware")
set(COMMS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/comms")


# Initialize the SDK
pico_sdk_init()


# Add FreeRTOS as a library
add_library(FreeRTOS STATIC
    ${FREERTOS_SRC_DIRECTORY}/event_groups.c
    ${FREERTOS_SRC_DIRECTORY}/list.c
    ${FREERTOS_SRC_DIRECTORY}/queue.c
    ${FREERTOS_SRC_DIRECTORY}/stream_buffer.c
    ${FREERTOS_SRC_DIRECTORY}/tasks.c
    ${FREERTOS_SRC_DIRECTORY}/timers.c
    ${FREERTOS_SRC_DIRECTORY}/portable/MemMang/heap_3.c
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0/port.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
)


# FT81x Pin mapping, done here instead of in pin mapping file to link properly
add_compile_definitions(EVE_CS=1)
add_compile_definitions(EVE_PDN=17)
add_compile_definitions(EVE_SCK=2)
add_compile_definitions(EVE_MOSI=3)
add_compile_definitions(EVE_MISO=0)
add_compile_definitions(EVE_BACKLIGHT_PWM=127)

# Add the comms library
add_library(comscicalc_comms STATIC
    ${COMMS_SRC_DIRECTORY}/comscicalc_comms.c
)
target_link_libraries(
    comscicalc_comms 
    PUBLIC 
)
target_include_directories(comscicalc_comms PUBLIC 
    ${COMMS_SRC_DIRECTORY}/
)

# Add FT81x/EVE display driver as a library
add_library(FT81X STATIC 
    ${FT81X_SRC_DIRECTORY}/EVE_commands.c
    ${FT81X_SRC_DIRECTORY}/EVE_target.c
)
target_link_libraries(
    FT81X 
    PUBLIC 
    pico_stdlib
    hardware_spi
)
target_include_directories(FT81X PUBLIC 
    ${FT81X_SRC_DIRECTORY}/
    ${FT81X_SRC_DIRECTORY}/EVE_target/
    ${HW_SRC_DIRECTORY}/
)
pico_enable_stdio_usb(FT81X 1)
pico_enable_stdio_uart(FT81X 0)

# Add hardware specific libs:
add_library(RP2040_HW STATIC 
    ${HW_SRC_DIRECTORY}/rp2040_utils.c
)
target_link_libraries(
    RP2040_HW 
    PUBLIC 
    pico_stdlib
    hardware_spi
    hardware_uart
    FreeRTOS
)
target_include_directories(RP2040_HW PUBLIC 
    ${HW_SRC_DIRECTORY}/
    ${COMMON_SRC_DIRECTORY}/
    ${COMSCICALC_SRC_DIRECTORY}/
)
pico_enable_stdio_usb(RP2040_HW 1)
pico_enable_stdio_uart(RP2040_HW 0)


# Import comscicalc_lib
add_library(comscicalc_lib STATIC 
    ${COMSCICALC_SRC_DIRECTORY}/comscicalc.c
    ${COMSCICALC_SRC_DIRECTORY}/comscicalc_operators.c
    ${COMSCICALC_SRC_DIRECTORY}/print_utils.c
    ${COMSCICALC_SRC_DIRECTORY}/uart_logger.c
)
target_link_libraries(
    comscicalc_lib
    PUBLIC 
    RP2040_HW
    FreeRTOS
)
target_include_directories(comscicalc_lib PUBLIC 
    ${COMSCICALC_SRC_DIRECTORY}/
)
pico_enable_stdio_usb(comscicalc_lib 1)
pico_enable_stdio_uart(comscicalc_lib 0)




# Add target for the display library:
add_library(DISPLAY STATIC 
    ${DISPLAY_SRC_DIRECTORY}/display.c
    ${DISPLAY_SRC_DIRECTORY}/menu.c
    ${DISPLAY_SRC_DIRECTORY}/fonts/font_library.c
)
target_link_libraries(
    DISPLAY 
    PUBLIC 
    FT81X
    comscicalc_lib
    FreeRTOS
)
target_include_directories(DISPLAY PUBLIC 
    ${DISPLAY_SRC_DIRECTORY}/
    ${DISPLAY_SRC_DIRECTORY}/fonts/
)
pico_enable_stdio_usb(DISPLAY 1)
pico_enable_stdio_uart(DISPLAY 0)

# Add Firmware common as a library
# This is the entry point for the FreeRTOS links to EVE and ComSciCalcLibs
add_library(FIRMWARE_COMMON STATIC 
    ${COMMON_SRC_DIRECTORY}/firmware_common.c
)
target_link_libraries(
    FIRMWARE_COMMON 
    PUBLIC 
    FreeRTOS
    comscicalc_lib
    FT81X
    DISPLAY
    RP2040_HW
)
target_include_directories(FIRMWARE_COMMON PUBLIC 
    ${COMMON_SRC_DIRECTORY}/
    ${COMSCICALC_SRC_DIRECTORY}/
)
pico_enable_stdio_usb(FIRMWARE_COMMON 1)
pico_enable_stdio_uart(FIRMWARE_COMMON 0)

add_subdirectory(src)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()
