cmake_minimum_required(VERSION 3.27)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)


option(BUILD_ALL "Build everything" OFF)
option(BUILD_FIRMWARE "Build the firmware" ON)
option(BUILD_WEBGUI "Build the web GUI" OFF)
option(BUILD_SOFTWARE "Build the software" OFF)
option(BUILD_TESTS "Build the tests" OFF)



if(BUILD_ALL OR BUILD_FIRMWARE)
    # Note: For some reason the pico sdk needs to be imported 
    # at a top level for the toolchain to work properly.
    # Not sure why, but it works.
    # message(STATUS "Building firmware")
    # set(PICO_SDK_FETCH_FROM_GIT TRUE)
    # set(PICO_SDK_FETCH_FROM_GIT_TAG "2.1.1")
    # include(${CMAKE_SOURCE_DIR}/firmware/pico_sdk_import.cmake)
    # message(STATUS "PICO_SDK_PATH: ${PICO_SDK_PATH}")
    project(comscicalc C CXX ASM)
    #add_subdirectory("firmware")

    # Build the common library as an external project, 
    # and build it with the arm-none-eabi toolchain to be included
    # in the firmware build.
    include(ExternalProject)
    ExternalProject_Add(common
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/common
        CMAKE_ARGS 
        -DCMAKE_C_COMPILER=arm-none-eabi-gcc
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
    )
    ExternalProject_Add(comscicalc_firmware
        DEPENDS common
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/firmware
        CMAKE_ARGS 
        -DCMAKE_C_COMPILER=arm-none-eabi-gcc
        -DCMAKE_TRY_COMPILE_TARGET_TYPE=STATIC_LIBRARY
    )
endif()

if(BUILD_ALL OR BUILD_SOFTWARE)
    message(STATUS "Building software")
    add_subdirectory("software")
endif()

if(BUILD_ALL OR BUILD_WEBGUI)
    message(STATUS "Building webgui")
    add_subdirectory("software/webgui")
endif()

if(BUILD_ALL OR BUILD_TESTS)
    message(STATUS "Building tests")
    add_subdirectory("tests")
endif()


