cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

## ---------- DEPENDENCIES ----------
# Pull in dependencies
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    ft81x_sdk
    GIT_REPOSITORY "https://github.com/oskarvh/FT800-FT813"
    GIT_TAG "5.x"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
FetchContent_Declare(
    freertos_kernel
    GIT_REPOSITORY "https://github.com/FreeRTOS/FreeRTOS-Kernel.git"
    GIT_TAG "V10.6.1"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)

#FetchContent_MakeAvailable(ft81x_sdk)
# Don't build the FreeRTOS kernel, just make populate the directory
FetchContent_Populate(freertos_kernel)
FetchContent_Populate(ft81x_sdk)

message(STATUS "ft81x_sdk source dir: ${ft81x_sdk_SOURCE_DIR}")
message(STATUS "freertos_kernel source dir: ${freertos_kernel_SOURCE_DIR}")
set(FT81X_SDK_PATH ${ft81x_sdk_SOURCE_DIR})

# Pull in the pico SDK, let it fetch itself.
# set(PICO_SDK_FETCH_FROM_GIT TRUE)
# include(${CMAKE_SOURCE_DIR}/firmware/pico_sdk_import.cmake)
# message(STATUS "PICO_SDK_PATH: ${PICO_SDK_PATH}")

## ---------- PROJECT ----------
# Define the project
project(comscicalc_firmware C CXX ASM)

# Check the pico sdk version
if (PICO_SDK_VERSION_STRING VERSION_LESS "1.3.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 1.3.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the pico SDK
pico_sdk_init()

## ---------- SETTINGS ----------
# Add debug
add_compile_definitions(DEBUG)
# Using the RP2040 chip
add_compile_definitions(RP2040)

# Using the FT81x with ADAFRUIT 1680 display
add_compile_definitions(EVE_FT810CB_ADAFRUIT_1680)
# Set logging level
add_compile_definitions(LOG_LEVEL=5) # 0 is the least logging

# FT81x Pin mapping
add_compile_definitions(EVE_CS=1)
add_compile_definitions(EVE_PDN=17)
add_compile_definitions(EVE_SCK=2)
add_compile_definitions(EVE_MOSI=3)
add_compile_definitions(EVE_MISO=0)
add_compile_definitions(EVE_BACKLIGHT_PWM=127)

## ---------- TARGETS ----------
# Add FreeRTOS as a library
add_library(freertos STATIC
    ${freertos_kernel_SOURCE_DIR}/event_groups.c
    ${freertos_kernel_SOURCE_DIR}/list.c
    ${freertos_kernel_SOURCE_DIR}/queue.c
    ${freertos_kernel_SOURCE_DIR}/stream_buffer.c
    ${freertos_kernel_SOURCE_DIR}/tasks.c
    ${freertos_kernel_SOURCE_DIR}/timers.c
    ${freertos_kernel_SOURCE_DIR}/portable/MemMang/heap_3.c
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM0/port.c
)
target_include_directories(freertos PUBLIC
    ${CMAKE_SOURCE_DIR}/firmware/
    ${freertos_kernel_SOURCE_DIR}/include/
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM0/
)

add_executable(
    firmware_main 
    ${CMAKE_SOURCE_DIR}/firmware/main.c
    ${CMAKE_SOURCE_DIR}/firmware/firmware_common.c
    ${CMAKE_SOURCE_DIR}/firmware/rp2040_utils.c
    ${CMAKE_SOURCE_DIR}/common/logger/logger.c
)
target_link_libraries(
    firmware_main 
    PUBLIC
    pico_stdlib
    hardware_spi
    freertos
)
target_include_directories(firmware_main PUBLIC 
    ${CMAKE_SOURCE_DIR}/firmware/
    ${CMAKE_SOURCE_DIR}/common/logger/
)

pico_enable_stdio_usb(firmware_main 1)
pico_enable_stdio_uart(firmware_main 0)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()

