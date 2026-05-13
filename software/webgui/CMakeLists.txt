cmake_minimum_required(VERSION 3.27)
project(webgui C)

set(CMAKE_C_STANDARD 99)

# ========================================================================
# SETUP
# ========================================================================
# Find system Python interpreter
find_package(Python3 REQUIRED COMPONENTS Interpreter)

# Paths
set(VENV_DIR "${CMAKE_SOURCE_DIR}/.venv")
set(VENV_PYTHON "${VENV_DIR}/bin/python")
set(REQUIREMENTS_FILE "${CMAKE_SOURCE_DIR}/software/webgui/requirements.txt")
set(SETUP_HTML_SCRIPT "${CMAKE_SOURCE_DIR}/software/webgui/setup_html.py")
if(NOT EXISTS "${REQUIREMENTS_FILE}")
    # If we're building from this directory, the path is different
    set(REQUIREMENTS_FILE "${CMAKE_SOURCE_DIR}/requirements.txt")
endif()
if(NOT EXISTS "${SETUP_HTML_SCRIPT}")
    # If we're building from this directory, the path is different
    set(SETUP_HTML_SCRIPT "${CMAKE_SOURCE_DIR}/requirements.txt")
endif()

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

# Step 3: Execute the setup_html.py script with the venv python
if(EXISTS "${SETUP_HTML_SCRIPT}")
    message(STATUS "Running setup_html.py with virtual environment python...")
    execute_process(
        COMMAND "${VENV_PYTHON}" "${SETUP_HTML_SCRIPT}"
        RESULT_VARIABLE SETUP_HTML_RESULT
        OUTPUT_VARIABLE SETUP_HTML_OUTPUT
        ERROR_VARIABLE SETUP_HTML_ERROR
    )
    if(NOT SETUP_HTML_RESULT EQUAL 0)
        message(FATAL_ERROR "setup_html.py failed: ${SETUP_HTML_ERROR}")
    else()
        message(STATUS "setup_html.py output: ${SETUP_HTML_OUTPUT}")
    endif()
else()
    message(WARNING "setup_html.py script not found at ${SETUP_HTML_SCRIPT}")
endif()

# ========================================================================
# DEPENDENCIES
# ========================================================================
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
FetchContent_Declare(
    clay
    GIT_REPOSITORY "https://github.com/oskarvh/clay.git"
    GIT_TAG        "ft81x_support"
    GIT_PROGRESS   TRUE
    GIT_SHALLOW    TRUE
)
FetchContent_Populate(clay)

set(COMMON_GUI_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/../../common/gui")
set(COMSCICALC_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/../../comSciCalcLib")
set(LOGGER_DIR        "${CMAKE_CURRENT_SOURCE_DIR}/../../common/logger")

# ========================================================================
# BUILD
# ========================================================================
# If the compiler isn't set to clang, skip the build
if(NOT CMAKE_C_COMPILER_ID STREQUAL "Clang")
    message(WARNING "C compiler is not Clang. Skipping webgui build.")
    return()
endif()

add_executable(webgui
    main.c
    ${COMMON_GUI_DIR}/gui.c
    ${COMSCICALC_DIR}/comscicalc.c
    ${COMSCICALC_DIR}/comscicalc_operators.c
    ${COMSCICALC_DIR}/print_utils.c
    ${LOGGER_DIR}/logger.c
)

set_target_properties(webgui PROPERTIES OUTPUT_NAME "index" SUFFIX ".wasm")

# Silence the logger entirely at compile time so vprintf / fd_write are DCE'd
target_compile_definitions(webgui PRIVATE "LOG_LEVEL=-1")

# comSciCalcLib has pre-existing unused-variable / format warnings — suppress
# them only for those files so -Werror doesn't break our build
set_source_files_properties(
    ${COMSCICALC_DIR}/comscicalc.c
    ${COMSCICALC_DIR}/comscicalc_operators.c
    ${COMSCICALC_DIR}/print_utils.c
    PROPERTIES COMPILE_FLAGS
        "-Wno-unused-variable -Wno-unused-value -Wno-format -Wno-unused-but-set-variable"
)

target_compile_options(webgui PUBLIC)
target_include_directories(webgui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${COMMON_GUI_DIR}
    ${COMSCICALC_DIR}
    ${LOGGER_DIR}
    ${clay_SOURCE_DIR}
)