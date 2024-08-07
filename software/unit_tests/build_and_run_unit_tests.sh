#!/bin/bash
# Script for building the comscicalc_lib
rm -r build/*
root_dir=`pwd`
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    cmake  -S$root_dir -B$root_dir/build -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
elif [[ "$OSTYPE" == "cygwin" ]]; then
    cmake  -S$root_dir -B$root_dir/build -G "MinGW Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
elif [[ "$OSTYPE" == "msys" ]]; then
    cmake  -S$root_dir -B$root_dir/build -G "MinGW Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
else
    echo "$OSTYPE is not supported"
    return 1
fi
cd build
cmake --build .

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    FILE=comscicalc_unit_tests
elif [[ "$OSTYPE" == "cygwin" ]]; then
    FILE=comscicalc_unit_tests.exe
elif [[ "$OSTYPE" == "msys" ]]; then
    FILE=comscicalc_unit_tests.exe
else
    echo "$OSTYPE is not supported"
    return 1
fi
./$FILE
cd ..