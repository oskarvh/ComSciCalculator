cmake_minimum_required(VERSION 3.27)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Pull in dependencies
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