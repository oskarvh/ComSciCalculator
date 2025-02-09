cmake_minimum_required(VERSION 3.14)

project(comscicalc_gui C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TRY_COMPILE_TARGET_TYPE DYNAMIC_LIBRARY)

add_compile_definitions(LOG_LEVEL=5) # 0 is the least logging

set(COMSCICALC_SRC_DIRECTORY "${CMAKE_SOURCE_DIR}/../../software/comSciCalc_lib")

# Import comscicalc_lib
add_library(comscicalc_lib SHARED
    ${COMSCICALC_SRC_DIRECTORY}/comscicalc.c
    ${COMSCICALC_SRC_DIRECTORY}/comscicalc_operators.c
    ${COMSCICALC_SRC_DIRECTORY}/print_utils.c
    ${COMSCICALC_SRC_DIRECTORY}/uart_logger.c
)

add_compile_options(-Wall
        -Wno-format          # int != int32_t as far as the compiler is concerned because gcc has int32_t as long int
        -Wno-unused-function # we have some for the docs that aren't called
        )
if (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-maybe-uninitialized)
endif()