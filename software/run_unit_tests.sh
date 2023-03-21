#!/bin/sh
# Script to setup git repos and other things needed for this repo. 

# Setup software. 
echo "Removing old executable"
rm unit_tests/test_suite.exe

echo "Compiling tests"
gcc -W unit_tests/test_suite.c unit_tests/unit_tests.c Unity/src/unity.c comSciCalc_lib/comscicalc.c comSciCalc_lib/comscicalc_operators.c -o unit_tests/test_suite

FILE=unit_tests/test_suite.exe
if [ -f "$FILE" ]; then
    echo "Running tests"
    ./unit_tests/test_suite.exe
fi