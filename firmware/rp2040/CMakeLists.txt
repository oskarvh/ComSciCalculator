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


set(FREERTOS_CFG_DIRECTORY "${CMAKE_SOURCE_DIR}/inc")
set(FREERTOS_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/FreeRTOS-Kernel")

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

# add_executable(comscicalc
#     src/main.c
#     )

# # pull in common dependencies
# target_link_libraries(
#     comscicalc 
#     PUBLIC 
#     pico_stdlib
#     FreeRTOS
# )
# # Enable/disable STDIO via USB and UART
# pico_enable_stdio_usb(comscicalc 1)
# pico_enable_stdio_uart(comscicalc 1)

# # Enable extra build products
# pico_add_extra_outputs(comscicalc)

add_subdirectory(src)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()
