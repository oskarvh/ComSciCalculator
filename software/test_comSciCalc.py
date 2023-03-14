# Written by Oskar von Heideken
# Copyright, 2023
# Script to test the computer scientist calculator, using c extensions

# TODO: add support for different input formats

# Standard Python
import _comSciCalc
from enum import Enum
import random

# Extended python
import pytest
import numpy as np

INPUTBASE_DEC = 0
INPUTBASE_HEX = 1
INPUTBASE_BIN = 2
INPUTBASE_NONE = -1

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

# Pytest fixture to generate random input values. 
# Returns dict with random value, base, cursor values and cursor positions
@pytest.fixture
def randomize_input():
	# Create a random length between 0 and 100
	randStrLen = random.randint(0,20)

	randString = ""
	for i in range(0,randStrLen):
		randIndex = random.randint(0,len(ACCEPTED_INPUT)-1)
		randString += ACCEPTED_INPUT[randIndex]

	# TODO: create random cursor position
	return randString


# Test simple input, without changing the cursor
@pytest.mark.parametrize(
	"test_input, input_base", [
		("123+567",INPUTBASE_DEC ), 
		("+123", INPUTBASE_DEC),
		("123+", INPUTBASE_DEC),
		("+++---", INPUTBASE_DEC),
		("+n+n+n", INPUTBASE_DEC),
		("nnnn", INPUTBASE_DEC),
		("(n123))", INPUTBASE_DEC),
		("()())))((", INPUTBASE_DEC)
	])
def test_basicInput_cursorFixed(test_input, input_base):
	# input the string into the calculator core
	translated_string = replace_input(test_input)
	resultString = _comSciCalc.comSciCalc(test_input, input_base)
	print("Result = " + resultString + ". Expected = " + translated_string)
	assert resultString == translated_string


@pytest.mark.parametrize("num_times", range(5))
# Test with randomized input, fixed cursor
def test_randomInput_cursorFixed(num_times, randomize_input):
	# input the string into the calculator core
	test_input = randomize_input
	translated_string = replace_input(test_input)
	resultString = _comSciCalc.comSciCalc(test_input, INPUTBASE_DEC)
	print("Result = " + resultString + ". Expected = " + translated_string)
	assert resultString == translated_string
	