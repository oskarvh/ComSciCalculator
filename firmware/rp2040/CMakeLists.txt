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

add_compile_definitions(RP2040)
add_compile_definitions(EVE_FT810CB_ADAFRUIT_1680)
add_compile_definitions(LOG_LEVEL=0) # 0 is the highest level of logging

set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/inc")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/FreeRTOS-Kernel")
set(FT81X_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/FT800-FT813")
set(COMMON_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../common")
set(COMSCICALC_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/comSciCalc_lib")
set(DISPLAY_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/display")
set(HW_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/hardware")


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
    #${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/port.c
)

# Build FreeRTOS
target_include_directories(FreeRTOS PUBLIC
    ${FREERTOS_CFG_DIRECTORY}/
    ${FREERTOS_SRC_DIRECTORY}/include
    ${FREERTOS_SRC_DIRECTORY}/portable/GCC/ARM_CM0
    #${FREERTOS_SRC_DIRECTORY}/portable/ThirdParty/GCC/RP2040/include
)
# target_link_libraries(
#     FreeRTOS 
#     PUBLIC 
#     pico_stdlib
#     hardware
# )

# FT81x Pin mapping, done here instead of in pin mapping file to link properly
add_compile_definitions(EVE_CS=1)
add_compile_definitions(EVE_PDN=17)
add_compile_definitions(EVE_SCK=2)
add_compile_definitions(EVE_MOSI=3)
add_compile_definitions(EVE_MISO=0)
add_compile_definitions(EVE_BACKLIGHT_PWM=127)

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
)
target_include_directories(RP2040_HW PUBLIC 
    ${HW_SRC_DIRECTORY}/
    ${COMMON_SRC_DIRECTORY}/
)

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
    RP2040_HW # Used for uart
)
target_include_directories(comscicalc_lib PUBLIC 
    ${COMSCICALC_SRC_DIRECTORY}/
)




# Add target for the display library:
add_library(DISPLAY STATIC 
    ${DISPLAY_SRC_DIRECTORY}/display.c
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

# Add Firmware common as a library
# This is the entry point for the FreeRTOS links to EVE and ComSciCalcLibs
add_library(FIRMWARE_COMMON STATIC 
    ${COMMON_SRC_DIRECTORY}/firmware_common.c
)
target_link_libraries(
    FIRMWARE_COMMON 
    PUBLIC 
    comscicalc_lib
    FreeRTOS
    FT81X
    DISPLAY
    RP2040_HW
)
target_include_directories(FIRMWARE_COMMON PUBLIC 
    ${COMMON_SRC_DIRECTORY}/
    ${COMSCICALC_SRC_DIRECTORY}/
)

add_subdirectory(src)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()
