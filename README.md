# Computer Scientist Calculator

The computer scientist calculator is a modern take on the HP 16C. 
It was born out of frustration of the terrible windows calculator, 
and since it's aimed at EE's, it also has to have hardware. 

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
## Developer log.
**March 6, 2023**

For preceding operators, such as NOT, have a single operand,
but can still have a long list of internal calculations.
This is kind of the same as brackets/parenthesis.
There are situations where these are stacked, e.g.
NOT(NOT(...)) or ((12+4)*2).
These situations require a lot of extra edge case handling
if handled in a single list entry.
After protoyping in Python, I found that if the brackets
spawn a new list entry with an increase in depth of 1 per new entry,
it becomes a lot easier to work with.
Since there are a lot of commonalities with preceding/single entry operators,
I think it makes sense to do the same. However, there is a point in keeping
the opThis (i.e. preceding operator) for printing purposes. Although, it's a
bit confusing having both preceding operator and normal operator in one
entry.

Conclusion is that there shall only be one operator per entry.
so a NOT() operator shall always have a next list with one higher depth.
Same goes for parenthesis. Do this and all tests of getInputListEntry passes

**March 14(pi day):**

I'm starting to think that the setup is a bit too advanced.
I don't remember the reason why I had separated the string entry and
the operator. I think it was due to the solver (which hasn't been written
yet) but I'm starting to think that 400 lines of code just to add a char
to a list is a bit on the heavy side. Therefore, I think I'll re-structure
the list entry so that it either takes an operator, function, bracket or
char, and then just handle the solver later. One thing that is nice with
having a string per entry is that solving will probably be easier, but at
the same same time, that will most likely not be the main issue, as I would
have to convert everything to string after all.
Another nice thing with the entry containing an entire string is that
handling base changes (hex/dec/bin) is easier.
It's a tradeoff though. I'll try it out using linker switches first though.


**March 15:**

I spent a whole lot of time thinking this through, and in the spirit of
keeping things simple, I decided to act on last nights decision.
Tests that the old code was failing is now passing. Although a first
pass didn't test cursor != 0, they are at least passing the random input
tests. I'm feeling happier with this solution, as it is simpler. I don't
think that the solver would have been a whole lot easier with the last
approach, but changing input base would. That being said, it is not
impossible to do. Come to think of it, the approach would be the same except
finding the start and end of the string is less efficient now, but not
harder.

**March 23:**

I'm currently working on the solver, and I think I have a good enough idea
on how to handle that, but I have identified a few things that needs to be
addressed:
1. It would be super nice with a pointer in the operator to a documentation
   string, so I added that.
2. I'm not sure how I want to handle the bitwise operators. I was first
   planning to have them as OP(arg1, arg2), but I'm starting to think
   that it's cleaner to have it as arg1->OP->arg2. It should be enough
   to just change the operator to non-depth increasing for this.
3. However, we do need to add a comma ',' for the special functions.
   This would be the argument separator. Should be easy enough to add
   as it would be handled almost the same as brackets.
4. At some point, I need to add support for floating point and fixed
   point. I can't say that I'm looking forward to this though...

Other than that, it's progressing nicely. Since the last update,
most of my time had gone into moving from python to the C unity test
framework, with a quick pitstop at CFFI in python.
I have concluded that there isn't a nice way to integrate this code into
a python based test framework, so I have gone with a C based test
framework instead.
At the moment, adding, removing and printing the buffer works fine,
those have static tests, so no randomized input yet.
But I was able to squash some bugs there. Moreover, the tests for
finding the deepest bracket, along with converting the list entries
from char to int is working nicely. The choice of having duplicate
lists for input buffer and solving buffer is needed so that we have
the input buffer to print.

**March 25:**

The expression solver is working for basic stuff like 123+456 and
123+456*789, and I'm working on depth increasing expressions.
I have thought at bit about how operators and functions are to be
handled, and I have set the normal bitwise operators to non-depth
increasing.
I think the expression solver will handle ignoring non-increasing
non-number types, such as a comma, but I have not tested that yet.
Moreover, I'm starting to think about what features makes sense.
I want to leave it as open to expansions with new functionality as I can
in that I want the have any type of function in there.
At the same time, I have been thinking about imaginary numbers and
variables, if that could be something that the calculator should support.
I think imaginary numbers would be awesome.
The difference in operators and custom functions are starting to
be very very small, so I might as well remove the custom function option
and just have operators. Really the only difference I can see is if
a custom function should be dynamically linked at runtime. But I cannot
see the need for that right now..
After some thought I removed the custom function, it's now merged into
operators.

**March 26**

I made some progress with the code itself, but mostly pulling in doxygen and clang formatter, 
along with updating the readme file, moving some stuff out of the lengthy C file. 
In general, minor fixes here and there, and I think the baseline is pretty much done. 
That being said, a lot more effort needs to go into the tests, since I'm not happy with 
the test coverage at all. With that said, it now solves basic expressions, and the depth 
increasing has been tested. But I would like to cover negative tests as well, where I try 
to break it by giving it bad input. 

Moreover, since moving from Python I have pretty much given up on randomized input, but I 
think that would be nice to bring back, and just have it run through heaps of randomized 
input data and try to calculate the results. That requires some other parser though, which 
will be tricky to build in C (it's more or less what I'm already doing). The 'eval' function 
in python was perfect for this. 