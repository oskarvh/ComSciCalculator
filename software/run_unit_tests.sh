#!/bin/bash
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
    rm unit_tests/test_suite*

    echo "Compiling tests"
    flags=""
    extra_linker=""
    if [ $verbose == true ]; then
        flags=-DVERBOSE
        if [[ "$OSTYPE" == "linux-gnu"* ]]; then
            extra_linker=" -lm"
        fi
    fi
    gcc -W $flags unit_tests/test_suite.c unit_tests/unit_tests.c Unity/src/unity.c comSciCalc_lib/comscicalc.c comSciCalc_lib/comscicalc_operators.c comSciCalc_lib/uart_logger.c comSciCalc_lib/print_utils.c -o unit_tests/test_suite $extra_linker
fi

if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    FILE=unit_tests/test_suite
elif [[ "$OSTYPE" == "cygwin" ]]; then
    FILE=unit_tests/test_suite.exe
elif [[ "$OSTYPE" == "msys" ]]; then
    FILE=unit_tests/test_suite.exe
else
    echo "$OSTYPE is not supported"
    return 1
fi

if [ -f "$FILE" ]; then
    echo "Running tests"
    ./$FILE
else 
    echo "No software to run, could not find $FILE. Try script without -d flag"
fi