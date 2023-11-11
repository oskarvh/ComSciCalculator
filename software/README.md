# Computer Scientist Calculator Software

This readme file contains descriptions on what the software purpose is, 
some detail on the philosophy behind the software, how to add features and
how to maintain it. 

## Committing to the repo
There is a commit-hook calling the clang-formatter, which is intended to block
a contributor from committing code that doesn't follow the coding standard. 

The coding standard file and the code formatter script can be found in the top
level folder of this repo, and I have based the standard on the LLVM coding 
standard, with some minor modifications. 

It's not required to test the additions before commit, but it's highly suggested. 
Moreover, any pull requests MUST include either a unit test (if applicable), 
a HIL (feature coming soon) test, or otherwise a strong argument as to why testing
isn't possible. This is not enforced, but I aim to set up a continous integration 
flow, and regular automated tests on unit tests, HIL tests and if applicable SIL tests. 


## comSciCalc_lib
The comSciCalc_lib library contains the calculator core code, and is responsible for the core functionality of the calculations. This includes everything between saving incoming data (except collecting the input, which is platform dependent), to solving the expression, converting the data into a readable format and so on. It aims to be a fully hardware agnostic library, so hardware specific code should NEVER be put in here. 
If for some reason there is need to access hardware accelerators for certain operators, this should be done with much care. 

The library consists of two parts: the core and the operators. The operators is a smaller file containing the operator definitions. This is kept separate to aid the addition of new operators. 

A new operator can be added in the comscicalc_operators.[c,h] files, by adding a 
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
the function must return ```format_not_supported```.

A more in depth instruction on how to add an operator to the computer scientist calculator is to come, but for now just follow the existing codebase. 

## Firmware
The firmware constitutes the agnostic and non-agnostic hardware code running on the target itself. This includes the FreeRTOS implementation along with the hardware interaction. As of now, the following platforms are supported:
* Texas Instruments TM4C123GH6PM



## Unit tests
The unit tests are small, function oriented unit tests that test the calculator core using the unity test framework (https://github.com/ThrowTheSwitch/Unity). Unity itself is fetched via a git submodule. 

The unit tests are designed to test as much as possible of the comSciCalc_lib, and is done so independently of the hardware platform. To run the unit tests after cloning and initializing the repo, along with installing the necessary compiler (GCC), the following script can be run to execute the unit tests. 
```bash
./software/run_unit_tests.sh
```
To run the script in verbose mode, add the -V flag. To not compile the source, 
use flag -d. 
Incorrect flags will prompt the usage message:
```bash
$ ./run_unit_tests.sh -h
Usage: ./run_unit_tests.sh [-V verbose] [-d dont clean]
```
This will compile the comSciCalc_lib library and the unit test code (using Unity test framework) and run the tests. 

This very same library is intended to work on an embedded system, along with (hopefully) a 
PC GUI of sorts, so the code does not implement any fancy features. 

NOTE, the test code uses some GCC specific pre-processor routines to initialize some of 
the arrays. 

## Test coverage
CGOV is used as the test coverage tool for those unit tests, and the aim is to get as close to 100% on the unit tests as possible. 
This is to ensure that most of the code is covered, although some NULL pointer checks are most likely not going to be covered. 

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

## Software dependencies
On Windows 10, I am using :
* gcc (MinGW.org GCC-6.3.0-1) 6.3.0
* Clang version 12.0.0
* Doxygen 1.9.6
* Git bash

for the comSciCalc_lib and 
* TI Code Composer Studio v 11.2.0 
* TI TivaWare_C_Series-2.2.0.295
* FreeRTOS-kernel (https://github.com/FreeRTOS/FreeRTOS-Kernel) as a submodule
* FT800-FT813 5.x forked from https://github.com/RudolphRiedel/FT800-FT813 with added support for TM4C as a submodule

for the embedded firmware. 

To pull the submodules, run the following git command. 
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

## Adding new fonts
The FT81x IC supports custom fonts. To convert the .tff file to the supported raw format, use the EVE asset builder (https://brtchip.com/eab/), and add this to the custom fonts file. See more details in `utils/README.md`. 

## TODO list:
There is a lot.. And even more since I need to add this here as well. 
