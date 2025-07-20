cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# ========================================================================
# DEPENDECIES
# ========================================================================
# --------------------------------- LIBS ---------------------------------
## Clay with FT81x support
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    clay
    GIT_REPOSITORY "https://github.com/oskarvh/clay.git"
    GIT_TAG "ft81x_support"
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
)
# Fetch the content without building it
FetchContent_Populate(clay)

# --------------------------------- FONTS --------------------------------
## MONOCRAFT FONT
set(MONOCRAFT_URL "https://github.com/IdreesInc/Monocraft/releases/download/v3.0/Monocraft.ttf")
set(MONOCRAFT_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/Monocraft.ttf")

if(NOT EXISTS "${MONOCRAFT_FONT}")
    message(STATUS "Downloading Monocraft font...")
    file(DOWNLOAD
        "${MONOCRAFT_URL}"
        "${MONOCRAFT_FONT}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
        LOG DOWNLOAD_LOG
    )
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${MONOCRAFT_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

## Comic mono
set(COMIC_MONO_URL "https://github.com/dtinth/comic-mono-font/raw/refs/heads/master/ComicMono.ttf")
set(COMIC_MONO_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/ComicMono.ttf")
if(NOT EXISTS "${COMIC_MONO_FONT}")
    message(STATUS "Downloading Comic Mono font...")
    file(DOWNLOAD
        "${COMIC_MONO_URL}"
        "${COMIC_MONO_FONT}"
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS
        LOG DOWNLOAD_LOG
    )
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${COMIC_MONO_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

## Fira Code
set(FIRA_CODE_URL "https://github.com/tonsky/FiraCode/raw/master/distr/ttf/FiraCode-Regular.ttf")
set(FIRA_CODE_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/FiraCode-Regular.ttf")
if(NOT EXISTS "${FIRA_CODE_FONT}")
    message(STATUS "Downloading Fira Code font...")
    file(DOWNLOAD "${FIRA_CODE_URL}" "${FIRA_CODE_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${FIRA_CODE_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

## JetBrains Mono
set(JETBRAINS_MONO_URL "https://github.com/JetBrains/JetBrainsMono/raw/master/fonts/ttf/JetBrainsMono-Regular.ttf")
set(JETBRAINS_MONO_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/JetBrainsMono-Regular.ttf")
if(NOT EXISTS "${JETBRAINS_MONO_FONT}")
    message(STATUS "Downloading JetBrains Mono font...")
    file(DOWNLOAD "${JETBRAINS_MONO_URL}" "${JETBRAINS_MONO_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${JETBRAINS_MONO_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

## Source Code Pro
set(SOURCE_CODE_PRO_URL "https://github.com/adobe-fonts/source-code-pro/raw/release/TTF/SourceCodePro-Regular.ttf")
set(SOURCE_CODE_PRO_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/SourceCodePro-Regular.ttf")
if(NOT EXISTS "${SOURCE_CODE_PRO_FONT}")
    message(STATUS "Downloading Source Code Pro font...")
    file(DOWNLOAD "${SOURCE_CODE_PRO_URL}" "${SOURCE_CODE_PRO_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${SOURCE_CODE_PRO_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

## IBM Plex Mono
set(IBM_PLEX_MONO_URL "https://github.com/IBM/plex/raw/refs/tags/v6.4.2/IBM-Plex-Mono/fonts/complete/ttf/IBMPlexMono-Regular.ttf")
set(IBM_PLEX_MONO_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/IBMPlexMono-Regular.ttf")
if(NOT EXISTS "${IBM_PLEX_MONO_FONT}")
    message(STATUS "Downloading IBM Plex Mono font...")
    file(DOWNLOAD "${IBM_PLEX_MONO_URL}" "${IBM_PLEX_MONO_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${IBM_PLEX_MONO_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

# Cascadia Code
set(CASCADIA_CODE_URL "https://github.com/microsoft/cascadia-code/raw/main/fonts/ttf/CascadiaCode-Regular.ttf")
set(CASCADIA_CODE_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/CascadiaCode-Regular.ttf")
if(NOT EXISTS "${CASCADIA_CODE_FONT}")
    message(STATUS "Downloading Cascadia Code font...")
    file(DOWNLOAD "${CASCADIA_CODE_URL}" "${CASCADIA_CODE_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${CASCADIA_CODE_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

# Inconsolata
set(INCONSOLATA_URL "https://github.com/google/fonts/raw/main/ofl/inconsolata/static/Inconsolata-Regular.ttf")
set(INCONSOLATA_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/Inconsolata-Regular.ttf")
if(NOT EXISTS "${INCONSOLATA_FONT}")
    message(STATUS "Downloading Inconsolata font...")
    file(DOWNLOAD "${INCONSOLATA_URL}" "${INCONSOLATA_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${INCONSOLATA_FONT}")  # Remove the empty or incomplete file
    endif()
endif()


# Hack
set(HACK_FONT_URL "https://github.com/source-foundry/Hack/raw/master/build/ttf/Hack-Regular.ttf")
set(HACK_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/Hack-Regular.ttf")
if(NOT EXISTS "${HACK_FONT}")
    message(STATUS "Downloading Hack font...")
    file(DOWNLOAD "${HACK_FONT_URL}" "${HACK_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${HACK_FONT}")  # Remove the empty or incomplete file
    endif()
endif()

# Victor Mono
set(VICTOR_MONO_URL "https://github.com/rubjo/victor-mono/raw/refs/heads/master/src/assets/VictorMono-Regular.ttf")
set(VICTOR_MONO_FONT "${CMAKE_SOURCE_DIR}/common/gui/fonts/VictorMono-Regular.ttf")
if(NOT EXISTS "${VICTOR_MONO_FONT}")
    message(STATUS "Downloading Victor Mono font...")
    file(DOWNLOAD "${VICTOR_MONO_URL}" "${VICTOR_MONO_FONT}" SHOW_PROGRESS STATUS DOWNLOAD_STATUS LOG DOWNLOAD_LOG)
    list(GET DOWNLOAD_STATUS 0 DOWNLOAD_RESULT)
    if(NOT DOWNLOAD_RESULT EQUAL 0)
        message(WARNING "Download failed: ${DOWNLOAD_LOG}")
        file(REMOVE "${VICTOR_MONO_FONT}")  # Remove the empty or incomplete file
    endif()
endif()


# ========================================================================
# SETUP
# ========================================================================
# Find system Python interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Paths
set(VENV_DIR "${CMAKE_SOURCE_DIR}/common/gui/fonts/.venv")
set(VENV_PYTHON "${VENV_DIR}/bin/python")
set(REQUIREMENTS_FILE "${CMAKE_SOURCE_DIR}/common/gui/fonts/requirements.txt")
set(FONT_CONVERTER_SCRIPT "${CMAKE_SOURCE_DIR}/common/gui/fonts/fontConverter.py")

# Step 1: Create venv if it doesn't exist
if(NOT EXISTS "${VENV_PYTHON}")
    message(STATUS "Creating Python virtual environment at ${VENV_DIR}")
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -m venv "${VENV_DIR}"
        RESULT_VARIABLE VENV_CREATE_RESULT
    )
    if(NOT VENV_CREATE_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to create Python virtual environment.")
    endif()
endif()

# Step 2: Install requirements.txt
if(EXISTS "${REQUIREMENTS_FILE}")
    message(STATUS "Installing Python dependencies from ${REQUIREMENTS_FILE}")
    execute_process(
        COMMAND "${VENV_PYTHON}" -m pip install --upgrade pip
        RESULT_VARIABLE PIP_UPGRADE_RESULT
    )
    if(NOT PIP_UPGRADE_RESULT EQUAL 0)
        message(WARNING "Failed to upgrade pip.")
    endif()

    execute_process(
        COMMAND "${VENV_PYTHON}" -m pip install -r "${REQUIREMENTS_FILE}"
        RESULT_VARIABLE PIP_INSTALL_RESULT
    )
    if(NOT PIP_INSTALL_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to install Python packages from requirements.txt.")
    endif()
else()
    message(WARNING "No requirements.txt file found at ${REQUIREMENTS_FILE}")
endif()

# Step 3: Execute the fontConverter.py script with the venv python
if(EXISTS "${FONT_CONVERTER_SCRIPT}")
    message(STATUS "Running fontConverter.py with virtual environment python...")
    execute_process(
        COMMAND "${VENV_PYTHON}" "${FONT_CONVERTER_SCRIPT}"
        RESULT_VARIABLE FONT_CONVERTER_RESULT
        OUTPUT_VARIABLE FONT_CONVERTER_OUTPUT
        ERROR_VARIABLE FONT_CONVERTER_ERROR
    )
    if(NOT FONT_CONVERTER_RESULT EQUAL 0)
        message(FATAL_ERROR "fontConverter.py failed: ${FONT_CONVERTER_ERROR}")
    else()
        message(STATUS "fontConverter.py output: ${FONT_CONVERTER_OUTPUT}")
    endif()
else()
    message(WARNING "fontConverter.py script not found at ${FONT_CONVERTER_SCRIPT}")
endif()

# ========================================================================
# BUILD
# ========================================================================
# Add the library for the gui
add_library(gui STATIC 
    ${CMAKE_SOURCE_DIR}/common/gui/gui.c
)
# pull in dependencies into the demo
target_link_libraries(
    gui
    PUBLIC
)
target_include_directories(gui PUBLIC 
    ${clay_SOURCE_DIR}/
    ${CMAKE_SOURCE_DIR}/common/gui/
)