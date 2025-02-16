# Cmake lists for comscicalc_lib
# Written by Oskar von Heideken
cmake_minimum_required(VERSION 3.15)


project(logger)

add_library(logger STATIC
    ${CMAKE_SOURCE_DIR}/common/logger/logger.c
)
target_link_libraries(logger
    PUBLIC
    pico_stdlib
)
pico_enable_stdio_usb(logger 1)
target_include_directories(logger
    PUBLIC
    ${CMAKE_SOURCE_DIR}/common/logger/
    ${CMAKE_SOURCE_DIR}/firmware/
)