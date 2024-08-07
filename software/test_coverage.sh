#!/bin/sh
# Script for compiling and running test coverage. 
# based on this brilliant video: https://www.youtube.com/watch?v=UOGMNRcV9-4

cflags=" -W -DVERBOSE=true "
files=" ../unit_tests/test_suite.c ../unit_tests/unit_tests.c ../Unity/src/unity.c ../comSciCalc_lib/comscicalc.c ../comSciCalc_lib/comscicalc_operators.c"
gcovflags=" -fprofile-arcs -ftest-coverage "
rm -r test_coverage
mkdir test_coverage
cd test_coverage

#compile
gcc $cflags $gcovflags $files -o tc.out

# Copy source files over to this dir.
cp $files .

# run the .out file
touch testrun.txt
./tc.out >> testrun.txt

gcov comscicalc.c
gcov comscicalc_operators.c

echo "Coverage analysis done. See test_coverage/*.c.gcov for details"
cd ..