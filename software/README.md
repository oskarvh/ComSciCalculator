# ComSciCalculator Library
This folder contains the C code for running the computer science calculator. 

It's designed to have a shared codebase between embedded and the test harness written in python, using extension modules. 


# Dependencies
Code Composer Studio, tirtos_tivac_2_16_00_08, TI GRLIB, KiCAD, FreeCAD, python 3.6 (for test harness). 
See separate requirements.txt for python 3.6 dependencies. 

# Run python test harness
First, the c code needs to be compiled and installed, which can be done by 'pip install .' in this directory. 
The code can also be compiled with 'python setup build'. This requires a build chain to be setup. 

Then, the test is run with 'pytest test_comSciCalc.py'. 

# Compile C code and test using GCC
GCC can be used to compile the C code in this directory: ' gcc comSciCalc_lib/comscicalc_ctest.c comSciCalc_lib/comscicalc.c comSciCalc_lib/comscicalc_operators.c -o comSciCalc_lib/comsciscalc_ctest'.
The C test harness can then be ran with './comSciCalc_lib/comsciscalc_ctest.exe '