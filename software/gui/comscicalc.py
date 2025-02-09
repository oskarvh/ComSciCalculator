# Python bindings for comscicalc_lib. This binds the C-functions to a python class
"""
MIT License

Copyright (c) 2024 Oskar von Heideken

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
"""

import ctypes
from enum import Enum

# InputBase selector enum
class inputBase(Enum):
    # Base 10, i.e. decimal.
    inputBase_DEC = 0
    # Base 16, i.e. hexadecimal.
    inputBase_HEX = 1
    # Base 2, i.e. binary.
    inputBase_BIN = 2
    # Base not defined.
    inputBase_NONE = -1

inputBaseRepr = {
    inputBase.inputBase_DEC.value: "Decimal",
    inputBase.inputBase_HEX.value: "Hexadecimal",
    inputBase.inputBase_BIN.value: "Binary",
    inputBase.inputBase_NONE.value: "None",
}

inputFormatRepr = {
    0: "Integer",
    1: "Fixed Point",
    2: "Floating Point",
}

# Input modificator
class inputModStatus(Enum):
    # Function returned with success.
    inputModStatus_SUCCESS = 0
    # Error: Cursor is larger than number of list entries.
    inputModStatus_CURSOR_VALUE_LARGER_THAN_LIST_ENTRY = -2
    # Error: Pointer to list entry was NULL.
    inputModStatus_INPUT_LIST_NULL = -1

# Function statuses
class calc_funStatus(Enum):
    # Success:Function returned with success
    calc_funStatus_SUCCESS = 0
    # Error: Pointer to list entry was NULL
    calc_funStatus_INPUT_LIST_NULL = 1
    # Error: Pointer to calculator core was NULL.
    calc_funStatus_CALC_CORE_STATE_NULL = 2
    #  Error: Base was NONE, but tried to use it.
    calc_funStatus_INPUT_BASE_ERROR = 3
    # Error: Unrecognized input character.
    calc_funStatus_UNKNOWN_INPUT = 4
    # Error: Malloc not possible.
    calc_funStatus_ALLOCATE_ERROR = 6
    # Error: Pointer to string buffer was NULL.
    calc_funStatus_STRING_BUFFER_ERROR = 7
    # Error: Entry list is not healthy.
    calc_funStatus_ENTRY_LIST_ERROR = 8
    # Error: Teardown of calculator core incomplete. Memory leak exist.
    calc_funStatus_TEARDOWN_INCOMPLETE = 9
    # Warning: Could not solve expression.
    calc_funStatus_SOLVE_INCOMPLETE = 10
    # Error: Unknown other input
    calc_funStatus_UNKNOWN_PARAMETER = 11
    # Error: Format error (format is e.g. int, float, fixed)
    calc_funStatus_FORMAT_ERROR = 12

# Solver statuses
class calc_solveStatus(Enum):
    # Success:Function returned with success.
    calc_solveStatus_SUCCESS = 0
    # Error: Pointer to list entry was NULL.
    calc_solveStatus_INPUT_LIST_NULL = -1
    # Error: Bracket mismatch.
    calc_solveStatus_BRACKET_ERROR = -2
    #  Error: Error with input list.
    calc_solveStatus_INPUT_LIST_ERROR = -3
    # Error: Operator pointer was NULL.
    calc_solveStatus_OPERATOR_POINTER_ERROR = -4
    # Error: Calculation could not be made.
    calc_solveStatus_CALC_NOT_SOLVABLE = -5
    # Error: Invalid number of arguments.
    calc_solveStatus_INVALID_NUM_ARGS = -6
    # Error: Malloc not possible.
    calc_solveStatus_ALLOCATION_ERROR = -7
    # Error: There was arguments, but no operator
    calc_solveStatus_ARGS_BUT_NO_OPERATOR = -8
    # Error: Invalid arguements.
    calc_solveStatus_INVALID_ARGS = -9

# Classes to handle input formats
class InputType(ctypes.Structure):
    _fields_ = [
        ("c", ctypes.c_char),
        ("typeFlag", ctypes.c_uint8),
        ("subresult", ctypes.c_int64),
    ]

class InputListEntry(ctypes.Structure):
    _fields_ = [
        ("pPrevious", ctypes.c_void_p),
        ("pNext", ctypes.c_void_p),
        ("entry", InputType),
        ("inputBase", ctypes.c_uint8),
        ("pFunEntry", ctypes.c_void_p),
    ]

class NumberFormat(ctypes.Structure):
    _fields_ = [
        ("numBits", ctypes.c_uint8),
        ("inputFormat", ctypes.c_uint8),
        ("outputFormat", ctypes.c_uint8),
        ("sign", ctypes.c_bool),
        ("inputBase", ctypes.c_uint8),
        ("fixedPointDecimalPlace", ctypes.c_uint8),
    ]

class CalcCoreState(ctypes.Structure):
    _fields_ = [
        ("pListEntrypoint", ctypes.c_void_p),
        ("cursorPosition", ctypes.c_uint8),
        ("allocCounter", ctypes.c_uint8),
        ("solved", ctypes.c_bool),
        ("result", ctypes.c_int64),
        ("numberFormat", NumberFormat),
    ]


class ComSciCalc():
    """
    Class that wraps the ComSciCalc C functions to get a pythonesque 
    interface. 
    """
    def __init__(self):
        """
        Initializes the class. Adds the C library and defines the C function calls. 
        """
        # Add the library
        self.comscicalc_core = ctypes.CDLL("build/libcomscicalc_lib.so")

        # Add the function definitions
        self.comscicalc_core.calc_coreInit.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_coreBufferTeardown.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_addInput.argtypes = [ctypes.c_void_p, ctypes.c_char]
        self.comscicalc_core.calc_removeInput.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_printBuffer.argtypes = [
            ctypes.c_void_p, 
            ctypes.c_char_p,
            ctypes.c_uint16,
            ctypes.c_void_p,
        ]
        self.comscicalc_core.calc_solver.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_getCursorLocation.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_updateBase.argtypes = [ctypes.c_void_p]
        self.comscicalc_core.calc_updateInputFormat.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
        self.comscicalc_core.calc_updateOutputFormat.argtypes = [ctypes.c_void_p, ctypes.c_uint8]
        self.comscicalc_core.convertResult.argtypes = [ctypes.c_char_p, ctypes.c_int64, ctypes.c_void_p, ctypes.c_uint8]
        self.comscicalc_core.getEffectiveFixedPointDecimalPlace.argtypes = [ctypes.c_void_p]

        # Create state to keep track of the state. 
        self.calcCoreState = CalcCoreState()
        self.calcCoreState.numberFormat.numBits = 64
        self.calcCoreState.numberFormat.fixedPointDecimalPlace = 32


    ##################################################
    ### Function Wrappers
    ##################################################
    def calc_coreInit(self, inputBase:inputBase = inputBase.inputBase_DEC) -> None:
        """
        Initializes the calculator core state.
        :param inputBase: inputBase 
        :return: None
        """

        # Initialize the calculator core
        status = self.comscicalc_core.calc_coreInit(ctypes.pointer(self.calcCoreState))
        assert status == 0, "Unable to initialize comscicalc core"
        # Set the input base to decimal:
        self.calcCoreState.numberFormat.inputBase = ctypes.c_int8(inputBase.value)
    
    def calc_coreBufferTeardown(self) -> None:
        """
        Tears down the internal structures of the calc core.
        :return: None
        """
        status = self.comscicalc_core.calc_coreBufferTeardown(ctypes.pointer(self.calcCoreState))
        assert status == 0, "Unable to initialize comscicalc core"
        # Set the input base to None to reflect that it's been torn down:
        self.calcCoreState.numberFormat.inputBase = ctypes.c_int8(inputBase.inputBase_NONE.value)

    def calc_addInput(self, input:int|str) -> str:
        """
        Adds a number, string or char to the calculator core
        Note: Since there is an input filter, it's not entirely certain 
        that the number or character corresponds with the input format. 
        E.g. if the input format is dec, but 'a' is added, then 'a' is simply ignored. 
        :param input: <str or int> Input to be added
        :return: <str> The char(s) or int that were added. 
        """
        if isinstance(input, int):
            # Convert the integer to a byte char and add it
            input_chars = bytes(str(input))
        else:
            input_chars = bytes(input, 'utf-8')
        written_chars = ""
        for input_char in input_chars:
            nums = ctypes.c_char(input_char)
            status = self.comscicalc_core.calc_addInput(ctypes.pointer(self.calcCoreState), nums)
            if status == calc_funStatus.calc_funStatus_SUCCESS.value:
                written_chars += chr(input_char)
        return written_chars
    
    def calc_removeInput(self) -> bool:
        """
        Removes (backspacing) input at current cursor location. 
        :return: <bool> True if successful, False if not.
        """
        status = self.comscicalc_core.calc_removeInput(ctypes.pointer(self.calcCoreState))
        if status == calc_funStatus.calc_funStatus_SUCCESS.value:
            return True
        return False
    
    def calc_printBuffer(self) -> tuple:
        """
        Print the buffers in a human readable format.
        :return: Tuple of (string, syntax issue position)
        """
        # Allocate a large buffer for the string that the C function can write too. 
        maxStringLen = 100
        pInputString = ctypes.create_string_buffer(maxStringLen)
        pResDecimalString = ctypes.create_string_buffer(maxStringLen)
        pResHexString = ctypes.create_string_buffer(maxStringLen)
        pResBinString = ctypes.create_string_buffer(maxStringLen)

        syntaxIssuePos = ctypes.c_int16(-1)
        pSyntaxIssuePos = ctypes.pointer(syntaxIssuePos)
        # Call the C function
        self.comscicalc_core.calc_printBuffer(
            ctypes.pointer(self.calcCoreState), 
            pInputString, 
            maxStringLen, 
            pSyntaxIssuePos
        )
        self.comscicalc_core.convertResult(
            pResDecimalString, 
            ctypes.c_int64(self.calcCoreState.result),
            ctypes.pointer(self.calcCoreState.numberFormat),
            ctypes.c_uint8(inputBase.inputBase_DEC.value),
        )
        self.comscicalc_core.convertResult(
            pResHexString, 
            ctypes.c_int64(self.calcCoreState.result),
            ctypes.pointer(self.calcCoreState.numberFormat),
            ctypes.c_uint8(inputBase.inputBase_HEX.value),
        )
        self.comscicalc_core.convertResult(
            pResBinString, 
            ctypes.c_int64(self.calcCoreState.result),
            ctypes.pointer(self.calcCoreState.numberFormat),
            ctypes.c_uint8(inputBase.inputBase_BIN.value),
        )

        return (pInputString.value, pResDecimalString.value, pResHexString.value, pResBinString.value, syntaxIssuePos.value)
    
    def calc_solver(self):
        """
        Wrapper function to call the solver. 
        :return: <bool> True if successful, otherwise false
        """
        status = self.comscicalc_core.calc_solver(ctypes.pointer(self.calcCoreState))
        if status == calc_funStatus.calc_funStatus_SUCCESS.value:
            return True
        return False
    

    ##################################################
    ### Helper functions
    ##################################################
    def moveCursor(self, steps:int) -> int:
        """
        Function to move the cursor by n steps: positive values moves it right, 
        negative values moves the cursor right. 
        Note that this steps the number of operators, meaning that even if 
        an operator is several chars long, this will step over all those. 
        :param steps: <int> How many steps to move the cursor
        :return: <int> How many steps was the cursor actually moved.
        """
