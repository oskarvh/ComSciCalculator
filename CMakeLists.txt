cmake_minimum_required(VERSION 3.27)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


option(BUILD_ALL "Build everything" OFF)
option(BUILD_FIRMWARE "Build the firmware" ON)
option(BUILD_SOFTWARE "Build the software" OFF)
option(BUILD_TESTS "Build the tests" OFF)



if(BUILD_ALL OR BUILD_FIRMWARE)
    # Note: For some reason the pico sdk needs to be imported 
    # at a top level for the toolchain to work properly.
    # Not sure why, but it works.
    message(STATUS "Building firmware")
    set(PICO_SDK_FETCH_FROM_GIT TRUE)
    include(${CMAKE_SOURCE_DIR}/firmware/pico_sdk_import.cmake)
    message(STATUS "PICO_SDK_PATH: ${PICO_SDK_PATH}")
    project(comscicalc C CXX ASM)
    add_subdirectory("firmware")
endif()

if(BUILD_ALL OR BUILD_SOFTWARE)
    message(STATUS "Building software")
    add_subdirectory("software")
endif()

if(BUILD_ALL OR BUILD_TESTS)
    message(STATUS "Building tests")
    add_subdirectory("tests")
endif()


