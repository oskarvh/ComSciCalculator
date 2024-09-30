# Cmake lists for comscicalc_comms
# Written by Oskar von Heideken
cmake_minimum_required(VERSION 3.15)


project(comscicalc_comms)

add_library(comscicalc_comms STATIC
    comscicalc_comms.c
    crc.c
)

target_include_directories(comscicalc_comms
    PUBLIC
    .
)