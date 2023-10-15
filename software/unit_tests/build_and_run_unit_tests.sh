#!/bin/sh
# Script for building the comscicalc_lib
#!/bin/sh
# Script to build the project
rm -r build/*
root_dir=`pwd`
cmake  -S$root_dir -B$root_dir/build -G "MinGW Makefiles" -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cd build
cmake --build .
./comscicalc_unit_tests.exe
cd ..