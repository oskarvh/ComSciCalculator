# Cmake lists for comscicalc_lib
# Written by Oskar von Heideken
cmake_minimum_required(VERSION 3.15)


project(comscicalc_lib)

#include(${CMAKE_SOURCE_DIR}/common/logger/CMakeLists.txt)

add_library(comscicalc_lib STATIC
    ${CMAKE_SOURCE_DIR}/comSciCalcLib/comscicalc.c
    ${CMAKE_SOURCE_DIR}/comSciCalcLib/comscicalc_operators.c
    ${CMAKE_SOURCE_DIR}/comSciCalcLib/print_utils.c
)
target_link_libraries(comscicalc_lib
    PUBLIC
    logger
)
target_include_directories(comscicalc_lib
    PUBLIC
    ${CMAKE_SOURCE_DIR}/comSciCalcLib/
)