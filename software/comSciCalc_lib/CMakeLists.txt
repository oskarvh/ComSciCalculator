# Cmake lists for comscicalc_lib
# Written by Oskar von Heideken
cmake_minimum_required(VERSION 3.15)


project(comscicalc_lib)

add_library(comscicalc_lib STATIC
    comscicalc.c
    comscicalc_operators.c
    print_utils.c
    uart_logger.c
)

target_include_directories(comscicalc_lib
    PUBLIC
    .
)

#target_link_libraries(comscicalc_lib
#    PUBLIC
#
#)