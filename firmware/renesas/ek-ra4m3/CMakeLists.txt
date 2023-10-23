# Written by Oskar von Heideken
# CMakeLists file for building the comscicalc FW with Renesas EL-RA4M3 dev kit.

# Project minimum required CMake version
cmake_minimum_required(VERSION 3.16.4)

# Project configuration
project(renesas_EK_RA4M3_comscicalc
	VERSION 1.0.0
	LANGUAGES C CXX ASM
)
set (CMAKE_C_STANDARD 11)

# Set variables for root directories, used below
set(PROJECT_ROOT ${CMAKE_CURRENT_LIST_DIR}/..)
set(GIT_ROOT ${CMAKE_CURRENT_LIST_DIR}/../../..)

#-------------DISPLAY-------------
# Add the display files. Depends on  other libraries, 
# so add those to the compile list.
file(GLOB display_files 
    ${GIT_ROOT}/software/display/display.c
    ${GIT_ROOT}/software/display/fonts/font_library.c
)
file(GLOB ft_810_files
    ${GIT_ROOT}/software/FT800-FT813/*.c
)
file(GLOB comscicalc_files
    ${GIT_ROOT}/software/comSciCalc_lib/*.c
)
file(GLOB freertos_files
    #${CMAKE_CURRENT_LIST_DIR}/inc/FreeRTOSConfig.h
    ${GIT_ROOT}/software/FreeRTOS-kernel/*.c
    #${GIT_ROOT}/software/FreeRTOS-Kernel/portable/GCC/ARM_CM33_NTZ/non_secure/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/rm_freertos_port/*.c
    ${GIT_ROOT}/software/FreeRTOS-kernel/portable/MemMang/heap_3.c
    #
)
file(GLOB fsp_files
    ${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/mcu/all/*.c
    ${PROJECT_ROOT}/fsp/ra/board/ra4m3_ek/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_ioport/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_sci_uart/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_sci_spi/*.c
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_gpt/*.c
    # Include the FreeRTOS port file to properly config the interrupts
    #${PROJECT_ROOT}/fsp/ra/fsp/src/rm_freertos_port/*.c
)



#-------------Source files-------------
file(GLOB_RECURSE source_files 
    # Firmware source files, incl. generated sources
    ${CMAKE_CURRENT_LIST_DIR}/src/*.c
    ${CMAKE_CURRENT_LIST_DIR}/inc/*.c
    ${GIT_ROOT}/firmware/common/*.c
)
SET(ALL_FILES 
    ${source_files}
    ${fsp_files} 
    ${display_files} 
    ${freertos_files} 
    ${comscicalc_files} 
    ${ft_810_files}
)

#-------------ADD EXECUTABLE AND DEFINTIONS-------------
add_executable(${PROJECT_NAME}.elf ${ALL_FILES})
# Add defintions for the display used:
add_definitions(-DEVE_FT810CB_ADAFRUIT_1680)
# Add definition for RA4M3 for FT810 driver
add_definitions(-D__RA4M3__)
add_definitions(-DEK_RA4M3)
add_definitions(-DVERBOSE)
add_definitions(-DLOG_LEVEL=1)

target_include_directories(${PROJECT_NAME}.elf PUBLIC
    # Firmware
    ${GIT_ROOT}/firmware/common/
    ${CMAKE_CURRENT_LIST_DIR}/inc/
    ${CMAKE_CURRENT_LIST_DIR}/inc/fsp_cfg/
    ${CMAKE_CURRENT_LIST_DIR}/inc/fsp_cfg/bsp/
    ${CMAKE_CURRENT_LIST_DIR}/src/
    # ARM CMSIS:
    ${PROJECT_ROOT}/CMSIS_5/CMSIS/Core/Include
    # FSP
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/instances/
    ${PROJECT_ROOT}/fsp/ra/fsp/inc/api/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/bsp/mcu/all/
    ${PROJECT_ROOT}/fsp/ra/board/ra4m3_ek/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_ioport/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_sci_uart/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_sci_spi/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/r_gpt/
    # Display:
    ${GIT_ROOT}/software/display/
    ${GIT_ROOT}/software/display/fonts/
    # FT810 driver
    ${GIT_ROOT}/software/FT800-FT813/
    ${GIT_ROOT}/software/FT800-FT813/EVE_target/
    # FreeRTOS
    ${GIT_ROOT}/software/FreeRTOS-kernel/include/
    #${GIT_ROOT}/software/FreeRTOS-kernel/portable/GCC/ARM_CM33_NTZ/non_secure/
    ${PROJECT_ROOT}/fsp/ra/fsp/src/rm_freertos_port/
    # comscicalc
    ${GIT_ROOT}/software/comSciCalc_lib/
    
)

set(ARM_OPTIONS -mcpu=cortex-m33 -O0 -mfloat-abi=hard -mfpu=fpv5-sp-d16)
set(COMPILER_WARNINGS -Wall)
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
    #--specs=nano.specs 
    #--specs=rdimon.specs
    ${COMPILER_WARNINGS}
    #-u_printf_float
    #-u_scanf_float
    #-nostartfiles
    -T ${FSP_LINKER_SCRIPT}
    -L ${CMAKE_CURRENT_SOURCE_DIR}/script
)