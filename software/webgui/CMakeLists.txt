cmake_minimum_required(VERSION 3.27)
project(webgui C)



set(CMAKE_C_STANDARD 99)

add_executable(webgui main.c)

target_compile_options(webgui PUBLIC)
target_include_directories(webgui PUBLIC .)