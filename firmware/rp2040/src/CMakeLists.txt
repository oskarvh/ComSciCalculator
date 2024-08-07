cmake_minimum_required(VERSION 3.14)

add_compile_definitions(APP_NAME="comscicalc")
add_compile_definitions(APP_VERSION="0")
add_compile_definitions(BUILD_NUM="${BUILD_NUMBER}")


add_executable(
    main
    main.c
)
# Enable/disable STDIO via USB and UART
pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

# pull in common dependencies
target_link_libraries(
    main 
    PUBLIC 
    FIRMWARE_COMMON
    FreeRTOS
    RP2040_HW
)

# Enable extra build products
pico_add_extra_outputs(main)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()
