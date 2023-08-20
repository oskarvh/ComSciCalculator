#!/bin/sh
# Script to setup git repos and other things needed for this repo. 

# Parse args
help=false
clean=true
verbose=false

usage() { echo "Usage: $0 [-V verbose] [-d dont clean]" 1>&2; exit 1; }

while getopts ":Vd" flag
do
    case "${flag}" in
        V) verbose=true;;
        d) clean=false;;
        *) usage;;
    esac
done

if [ $clean == true ]; then
    # Setup software. 
    echo "Removing old executable"
    rm unit_tests/test_suite.exe

    echo "Compiling tests"
    flags=""
    if [ $verbose == true ]; then
        flags=-DVERBOSE
    fi
    gcc -W $flags unit_tests/test_suite.c unit_tests/unit_tests.c Unity/src/unity.c comSciCalc_lib/comscicalc.c comSciCalc_lib/comscicalc_operators.c comSciCalc_lib/uart_logger.c comSciCalc_lib/print_utils.c -o unit_tests/test_suite
fi

FILE=unit_tests/test_suite.exe
if [ -f "$FILE" ]; then
    echo "Running tests"
    ./unit_tests/test_suite.exe
else 
    echo "No software to run. Try script without -d flag"
fi