# Computer Scientist Calculator

The computer scientist calculator (comSciCalc) is a modern take on the HP 16C. 
It was born out of frustration of the terrible windows calculator, and the lack of nice alternatives. 

The comSciCalc consists of both hardware and software, where each component can be used independently or together; The hardware can act as a controller for the software, in which case the software is simply acting as a bigger display. 

## Setup
A list of the dependencies are shown below, divided into common dependencies, software (PC) dependencies and firmware dependencies for building, testing and running.

Apart from the non-source-code (e.g. compilers, python etc), all dependencies are pulled in either via git submodules (`git clone --recurse-submodules git@github.com:oskarvh/ComSciCalculator.git`), or at build time using CMake's `FetchContent` module.

### Common Dependencies
- CMake version 3.27.7 or newer
- clang-format version 14.0.0 (formatting)
- Doxygen 1.9.6 (building docs)
### Firmware Dependencies

- [RP2040 Pico-SDK library](https://github.com/raspberrypi/pico-sdk) (included as a submodule)
- arm-none-eabi-gcc: Arm GNU Toolchain 13.3.1 or other compatible with rp2040
- [Clay](https://github.com/oskarvh/clay) Forked Clay (C Layout) library with support for FT81x (included as submodule). Used both for PC and embedded GUI
- [FT800-FT813 library](https://github.com/RudolphRiedel/FT800-FT813) (included as a submodule)
- ComSciCalc Hardware
### Software Dependencies
- [Clay](https://github.com/oskarvh/clay) Forked Clay (C Layout) library with support for FT81x (included as submodule). Used both for PC and embedded GUI
- COMPILER TBD
### Test Dependencies
- Pytest
- [Unity](git@github.com:ThrowTheSwitch/Unity.git) Unity test tool (to be deprecated)
## BUILD
There are several targets that can be built in this repository:
- Firmware
- Software
- Tests
## Directory Structure
A brief overview of the directory structure can be found in this section. 

### `comSciCalcLib/`
This directory contains the comSciCalc library. This is technically a common library used here, but it has its own directory for clarity, since this is an essential part of the project. 

### `firmware/`
The firmware directory contains code required to build the firmware running on the comSciCalc hardware.
Note that this pulls in common dependencies as well. 
### `software/`
The software directory contains the code required to build the PC software. This uses some of the dependencies common to the firmware as well. 
### `test/`
The test directory contains all tests used both with firmware and software. 
Tests are divided into several categories.
#### `test/unit_tests/`
This directory contains unit tests, aimed to test granular components of the comSciCalc. 

If a new calculation function is added to the comSciCalc, then a unit test shall be written for that function. 
#### `test/feature_tests/`
This directory contains feature tests. These tests are aimed to test higher level functions of the comSciCalc, such as the solver and formatters. 

If a new high level feature is added in comSciCalc is created, then a feature test is expected.
#### `test/integration_tests/`
This is the highest level tests, where hardware and/or software is expected in the loop. 

If a new hardware revision is introduced which update interfaces, or if new high level functionality is created, a new integration test shall be written/existing test case updated.

### `utils/`
This is the utilities directory, and contain code which is not compiled/run/used at normal build time. For example, for creating bit-maps of fonts for the firmware, or other utilities which output is aimed at being checked in rather than built.

This directory is included as a developer help tool. 

### `hardware/`
This directory contain all the hardware source files, such as PCB design, mechanical design etc.

### `common/`
This directory contains code used in both `firmware/` and `software/`, and is placed outside either directory to have a common source, such that changes are reflected in both the firmware and the software. 

An example of this is the GUI, the link layer protocols etc. 


### `html/`
This directory contains the generated Doxygen HTML files. This is updated on a release basis, since the doxygen can be generated. Hence, it's not checked in regularly. 

## Contributing
Contributions are welcome, and this section aims to help a developer to add functionality to the comSciCalc.

There are several parts to the comSciCalc, and it's built with the aim of easily being able to add new features, such as new mathmatical operators.

### Adding an operator
An operator is any mathmatical function that can be called by the calculator, e.g. `+` or `AND()` etc.
An operator can have one or several arguments. 


# TODO list
This section contains stuff that needs doing. 
- Finish the documentation
- Restructure the repo to align with the structure in this readme.
- Pull in CLay and switch out the old GUI to that. 


---
# OLD STUFF (KEPT FOR REFERENCE, TO BE DELETED)
---
## Status of project
The software is mostly being worked on at the moment. A few iterations 
of the hardware has been made, but nothing has been ordered yet because some components are hard to get at the moment, along with the continuous change of heart when it comes to the hardware specs. 

The software is in a semi-stable phase at the moment. The core of the 
calculator, the software/comSciCalc_lib has had the ground work made. 
Fluffy features such as [doxygen](https://www.doxygen.nl/) and 
[clang formatter](https://clang.llvm.org/docs/ClangFormat.html) has been added as well. 

Unit testing has begun using the [Unity framework](https://github.com/ThrowTheSwitch/Unity). 

## Software

The software folder consists of two major code bases: comSciCalc_lib and unit_tests. 

The comSciCalc_lib is a library, currently compiled with GCC, which contains all the 
functions needed to input data, calculate and output result and formatted input. To 
compile and run the unit tests, run the following script:
```bash
./software/run_unit_tests.sh
```
To run the script in verbose mode, add the -V flag. To not compile the source, 
use flag -d. 
Incorrext flags will prompt the usage message:
```bash
$ ./run_unit_tests.sh -h
Usage: ./run_unit_tests.sh [-V verbose] [-d dont clean]
```
This will compile the comSciCalc_lib library and the unit test code (using Unity test framework) and run the tests. 

This very same library is intended to work on an embedded system, along with (hopefully) a 
PC GUI of sorts, so the code does not implement any fancy features. 

However, the test code uses some GCC specific preprocessor routines to initialize some of 
the arrays. 
## Software dependencies
On Windows 10, I am using :
* gcc (MinGW.org GCC-6.3.0-1) 6.3.0
* Clang version 12.0.0
* Doxygen 1.9.6
* Git bash

To pull unity into the repo, run:
```bash
git submodule update --init
```
## Documentation
To generate the docs, run the following:
```bash
cd software # Go to the software dir. 
doxygen doxygen_config
```
And the docs can be found in software/html/index.html

## Technical jargon
The core functionality of the calculator is: 
1. Handling input (valid checks, insertion)
2. Printing input
3. Converting from text to something that can be solved. 

Going through the list above, the input handling is done using a doubly linked
list, where the incoming char will be placed. Each entry has a pointer to 
the previous and a pointer to the next entry, along with data and meta data 
for that entry. The data is the actual input character, where the accepted chars
are a union between numerical and non-numerical input [0-9,a-f,(,),','] and
chars specific to the operators, found in comscicalc_operators.c. 

The meta data consists of e.g. base (i.e. hex, dec, bin), type of entry, pointer
to operator function (see below) and some more. See the docs for more info. 

Each operator has a unique char associated with it, along with some meta data of
its own, and a pointer to the function implementing the operator. 
The function is linked via function pointer that are casted left and right, 
but due to the way C let's a code handle pointer willy-nilly, this works out fine. 

When an input list is solved, it will re-use the entry list type above, but 
then each entry has a number associated with it, instead of a char. 
The solver will then go through this list and solve based on the function 
pointers and numbers entered. The next section will explain how to add 
a new operator. 

## Adding new operators
A new operator is added in the comscicalc_operators.[c,h] files, by adding a 
new entry to the ```operators``` table. The entry must consist of the following:
```
.inputChar = /* Associated operator character , e.g. '+'*/,
.opString = /* Null terminated string to be displayed during printing.  "+\0"*/,
.solvPrio = /* Solving priority, 0 is the highest. (see docs) */,
.bIncDepth = /* Boolean indicating if operators increase depth (=should it invoke a bracket) */,
.numArgs = /* Number of arguments. -1 if variable. Must be 2 if not depth increasing.  */,
.pFun = /* Pointer to the function associated with the operator */
```
Moreover, the operator function needs to be added to the comscicalc_operators.c 
and comscicalc_operators.h file as well. The operator function must follow this
 format:
 ```
int8_t calc_<NAME>(SUBRESULT_UINT *pResult, inputFormat_t inputFormat, int num_args, SUBRESULT_UINT *args);
 ```
Ideally the operator should support fixed and floating point, along with signed
and unsigned integer formats. But if for some reason this is impossible, then 
the function must return ```format_not_supported```

## Test coverage
The ```software/test_coverage.sh``` script will run a gcov analysis of the two main
library files: ```comscicalc.c``` and ```comscicalc_operators.c``` when running the test suite
in order to see what the code coverage is during the tests. This should ideally 
be close to 100% as it would indicate the code coverage is complete. 

Running this script will make a temporary folder under software called 
test_coverage, where the build files, gcov files and copies of the source files
are stored. 

Moreover, the script will print the code coverage for the two files being analyzed. 

To check which lines are being covered, run the ```test_coverage.sh``` script, go into the test_coverage directory and take a look in the ```*.c.gcov``` files. 
Each line with a ```#####``` mark has not been executed. 

With that information, additional tests can be added to the test suite to
ensure as good test coverage as possible. 

