# Written by Oskar von Heideken
# CMakeLists file for building the comscicalc FW with Renesas EL-RA4M3 dev kit.

# Project minimum required CMake version
cmake_minimum_required(VERSION 3.16.4)

# Project configuration
project(renesas_EK_RA4M3_comscicalc
	VERSION 1.0.0
	LANGUAGES C CXX ASM
)

# Set variables for root directories, used below
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(GIT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)



# Add the software dependencies
include_directories(${GIT_ROOT}/software/comscicalc_lib)
include_directories(${GIT_ROOT}/software/display)
include_directories(${GIT_ROOT}/software/FreeRTOS-kernel)
include_directories(${GIT_ROOT}/software/FT800-FT813)

#-------------FREE-RTOS-------------
# Build FreeRTOS as an interface library
set(FREERTOS_PORT "GCC_ARM_CM33 CACHE STRING")
add_library(freertos_lib INTERFACE)
# Add the freertos config file as an include dir.
target_include_directories(freertos_lib SYSTEM
    INTERFACE
    "${CMAKE_CURRENT_LIST_DIR}/inc/FreeRTOSConfig.h"
) 
target_compile_definitions(freertos_lib
    INTERFACE
    projCOVERAGE_TEST=0
)

#-------------COMSCICALC-------------
# Adding comscicalc as a static library
add_library(comscicalc_lib STATIC
    ${GIT_ROOT}/software/comSciCalc_lib/comscicalc.c
    ${GIT_ROOT}/software/comSciCalc_lib/comscicalc_operators.c
    ${GIT_ROOT}/software/comSciCalc_lib/print_utils.c
    ${GIT_ROOT}/software/comSciCalc_lib/uart_logger.c
)
target_include_directories(comscicalc_lib PUBLIC
    ${GIT_ROOT}/software/comSciCalc_lib/
)

#-------------FT810 DISPLAY DRIVER-------------
# Adding FT810 display driver as a static library
add_library(ft_810_lib STATIC
    ${GIT_ROOT}/software/FT800-FT813/EVE_commands.c
    ${GIT_ROOT}/software/FT800-FT813/EVE_target.c
)
target_include_directories(ft_810_lib PUBLIC
    ${GIT_ROOT}/software/FT800-FT813/
    ${GIT_ROOT}/software/FT800-FT813/EVE_target/
)
target_compile_definitions(freertos_lib
    INTERFACE
    projCOVERAGE_TEST=0
)

#-------------DISPLAY-------------
# Add the display files. Depends on  other libraries, 
# so add those to the compile list.
file(GLOB display_files 
    ${GIT_ROOT}/software/display/display.c
   ${GIT_ROOT}/software/display/fonts/font_library.c
)
#add_library(display_lib SHARED
#    ${GIT_ROOT}/software/display/display.c
#    ${GIT_ROOT}/software/display/fonts/font_library.c
#)
#target_include_directories(display_lib PUBLIC
#    ${GIT_ROOT}/software/display/
#    ${GIT_ROOT}/software/display/fonts/
#)
#target_link_libraries(display_lib PUBLIC freertos_lib)


#-------------Source files-------------
file(GLOB_RECURSE source_files 
    # Firmware source files, incl. generated sources
    "${CMAKE_CURRENT_LIST_DIR}/src/*.c"
    "${CMAKE_CURRENT_LIST_DIR}/inc/*.c"
    # Include FSP files, which are static
    "${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/*.c"
    "${PROJECT_ROOT}/fsp/ra/fsp/src/r_ioport/*.c"
    "${PROJECT_ROOT}/fsp/ra/board/ra4m3_ek/*.c"
)
SET(ALL_FILES ${source_files})# ${display_files})

#-------------ADD EXECUTABLE AND DEFINTIONS-------------
add_executable(${PROJECT_NAME}.elf ${ALL_FILES})
# Add defintions for the display used:
add_definitions(-DEVE_FT810CB_ADAFRUIT_1680)
# TBD: Needed? 
#add_definitions(-D__RA4M3__)

target_include_directories(${PROJECT_NAME}.elf PUBLIC
    # Firmware
    ${CMAKE_CURRENT_LIST_DIR}/inc
    ${CMAKE_CURRENT_LIST_DIR}/inc/fsp_cfg
    ${CMAKE_CURRENT_LIST_DIR}/inc/fsp_cfg/bsp
    ${CMAKE_CURRENT_LIST_DIR}/src
    # ARM CMSIS:
    ${PROJECT_ROOT}/CMSIS_5/CMSIS/Core/Include
    # FSP
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/instances/
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/api/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_ioport/
    ${PROJECT_ROOT}/fsp/ra/board/ra4m3_ek/
    # Display:
    #${GIT_ROOT}/software/display/
    #${GIT_ROOT}/software/display/fonts/

    # These below seems to be required by display... which to me 
    # indicates that this should probably be a shared library
    # ComSciCalc header files
    
)

# Link freertos as an interface library
target_link_libraries(
    ${PROJECT_NAME}.elf INTERFACE
    freertos_lib
)

# Link the static libraries
target_link_libraries(
    comscicalc_lib
    ft_810_lib
)


set(ARM_OPTIONS -mcpu=cortex-m33 -mfloat-abi=hard -mfpu=fpv5-sp-d16)
set(COMPILER_WARNINGS -Wunused -Wuninitialized -Wall -Wextra -Wmissing-declarations -Wconversion -Wpointer-arith -Wshadow -Wlogical-op -Waggregate-return -Wfloat-equal)
target_compile_options(
    ${PROJECT_NAME}.elf
    PUBLIC
    -g
    ${ARM_OPTIONS}
    -fmessage-length=0
    -funsigned-char
    -ffunction-sections
    -fdata-sections
    -mthumb
    ${COMPILER_WARNINGS}
    -MMD
    -MP
)

# Add the script subdirectory to the linked directory:
target_link_directories(
    ${PROJECT_NAME}.elf
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/script/
)
# Set the linker script path:
set(FSP_LINKER_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/script/fsp.ld)

# Link options:
target_link_options(
    ${PROJECT_NAME}.elf
    PUBLIC
    -g
    ${ARM_OPTIONS}
    -fmessage-length=0
    -funsigned-char
    -ffunction-sections
    -fdata-sections
    -mthumb
    -Xlinker
    --gc-sections 
    -Wl,-Map=${PROJECT_NAME}.map 
    --specs=nano.specs 
    --specs=rdimon.specs
    ${COMPILER_WARNINGS}
    #-u_printf_float
    #-u_scanf_float
    #-nostartfiles
    -T ${FSP_LINKER_SCRIPT}
)