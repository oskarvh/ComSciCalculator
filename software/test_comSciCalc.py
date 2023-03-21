# Written by Oskar von Heideken
# Copyright, 2023
# Script to test the computer scientist calculator, using c extensions

# This script is based on the unit test described by A. Steffen on YT:
# https://www.youtube.com/watch?v=zW_HyDTPjO0
#
# It uses the unittest module in conjuction with CFFI to compile
# and test functions.  

import unittest
import uuid
import subprocess
import cffi
import importlib
from parameterized import parameterized

# Function to call the GCC preprocessor to get the function
# definitions and variables. 
def preprocess(source):
    return subprocess.run(['gcc', '-E', '-P', '-'],
                          input=source, stdout=subprocess.PIPE,
                          universal_newlines=True, check=True).stdout

# Function to exclude the include statements in a header file
def readHeaderFileAndExcludeIncludes(filename):
    processed_include = ""
    with open(filename) as include_file:
        for line in include_file:
            if not ("#include" in line):
                processed_include += line
    return processed_include

# Function to load the C library. 
# Caution: there are some hacks in there!
def loadComSciCalc_lib():
    # load source code
    source_files = ["comSciCalc_lib/comscicalc.c", \
    	"comSciCalc_lib/comscicalc_operators.c"]

    # Generate a random name to avoid cashing. 
    # A bit of a hack but I can live with that for now. 
    name = "comSciCalc_" + uuid.uuid4().hex

    # Handle preprocessor directives. Ingore header includes. 
    includes = preprocess(readHeaderFileAndExcludeIncludes("comSciCalc_lib/comscicalc.h"))
    includes += preprocess(readHeaderFileAndExcludeIncludes("comSciCalc_lib/comscicalc_operators.h"))
    ffibuilder = cffi.FFI()
    ffibuilder.cdef(includes)
    ffibuilder.set_source(name, 
        '''#include "comSciCalc_lib/comscicalc.h"''', 
        sources = source_files)
    ffibuilder.compile()

    module = importlib.import_module(name)
    return module.lib

ACCEPTED_INPUT = [\
'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', \
#'a', 'b', 'c', 'd', 'e', 'f', \
'+', '-', '*', '/', \
'&', 'n', '|', '^', \
'~', \
')', \
'(', \
]

TRANSLATED_INPUT = [ \
['&','AND(' ], \
['n','NAND('], \
['|','OR('  ], \
['^','XOR(' ], \
['~','NOT(' ], \
]

# Function to replace the input chars to output strings. 
# e.g. '&' shall be printed as AND( etc. 
def replace_input(rawString):
    for tr in TRANSLATED_INPUT:
        rawString = rawString.replace(tr[0], tr[1])
    return rawString

# Class that collects the unit test cases
class ComSciCalcTestMethods(unittest.TestCase):
    # Setup function. This is ran before each test case
    def setUp(self):
    	# Compile and load the library
    	self.comSciCalc_module = loadComSciCalc_lib()

    # Function to get the buffer from the entry. 
    # Note: the embedded function will free itself, 
    # so teardown should not be called after this. 
    def getBuffer(self):
        resStr = ""
        c = "placeholder"
        while c != '\0':
            c = self.comSciCalc_module.UT_calc_printBuffer()
            c = c.decode("utf-8")
            resStr += c

        # return all but the last NULL char. 
        return resStr[:-1]
    @parameterized.expand([
            ["1234", "1234", ],
            ["12+45", "12+45"],
        ])
    # Test function: simple add input function
    def test_adding_input(self, input_string, result_string):
        self.comSciCalc_module.UT_calc_coreInit()
        self.comSciCalc_module.UT_calc_setBase(self.comSciCalc_module.inputBase_DEC)
        for c in input_string:
            self.comSciCalc_module.UT_calc_addInput(c.encode())
        self.assertEqual(self.getBuffer(), result_string)


if __name__ == '__main__':
    unittest.main()