# CMakeLists for building the unit tests. 
# Require Unity test framework and GCC 
# Build using the build shell script in this directory. 

cmake_minimum_required(VERSION 3.15)

project(comscicalc_unit_tests
	VERSION 1.0.0
	LANGUAGES C
)

set(CMAKE_C_COMPILER gcc)
set(CMAKE_LINKER gcc)
# Add the comcscicalc library
add_library(comscicalc_lib STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../comSciCalc_lib/comscicalc.c
    ${CMAKE_CURRENT_LIST_DIR}/../comSciCalc_lib/comscicalc_operators.c
    ${CMAKE_CURRENT_LIST_DIR}/../comSciCalc_lib/print_utils.c
    ${CMAKE_CURRENT_LIST_DIR}/../comSciCalc_lib/uart_logger.c
)

target_include_directories(comscicalc_lib PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../comSciCalc_lib/
)

# Add the unity library
add_library(unity STATIC
    ${CMAKE_CURRENT_LIST_DIR}/../unity/src/unity.c
)

target_include_directories(unity PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../unity/src/
)

# Add test source files
file(GLOB src_files
    ${CMAKE_CURRENT_LIST_DIR}/test_suite.c
    ${CMAKE_CURRENT_LIST_DIR}/unit_tests.c
)

#add_definitions("-DVERBOSE")
add_compile_options("-finput-charset=ascii")
add_executable(comscicalc_unit_tests ${src_files})
target_include_directories(
    comscicalc_unit_tests PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/
)

target_link_libraries(
    comscicalc_unit_tests
    comscicalc_lib
    unity
)