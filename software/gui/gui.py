# Python GUI using C bindings
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
import time
from comscicalc import ComSciCalc, inputBaseRepr, inputFormatRepr
import tkinter


class GUI():
    def __init__(self):
        self.root = tkinter.Tk()
        self.core = ComSciCalc()
        self.backspace = '\x08'
        # (char, label, row, column)
        self.buttonConfig =[
            ("0", "0", 6, 8), 
            ("1", "1", 5, 8), 
            ("2", "2", 5, 9), 
            ("3", "3", 5, 10), 
            ("4", "4", 4, 8), 
            ("5", "5", 4, 9), 
            ("6", "6", 4, 10), 
            ("7", "7", 3, 8), 
            ("8", "8", 3, 9), 
            ("9", "9", 3, 10), 
            ("a", "A", 2, 8), 
            ("b", "B", 2, 9), 
            ("c", "C", 2, 10), 
            ("d", "D", 1, 8), 
            ("e", "E", 1, 9), 
            ("f", "F", 1, 10), 
            (".", ".", 6, 9), 
            ("ce", "CE", 6, 10), 
            (self.backspace, "BKSP", 7, 10), 
            ("&", "&", 7, 0), 
            ("+", "+", 7, 1), 
            ("-", "-", 7, 2), 
            ("f3", "F3", 7, 3), 
            ("f4", "F4", 7, 4), 
            ("f5", "F5", 7, 5), 
            ("f6", "F6", 7, 6), 
            ("f7", "F7", 7, 7), 
            ("f8", "F8", 7, 8), 
            ("f9", "F9", 7, 9), 
            ("f10", "F10", 8, 0), 
            ("f11", "F11", 8, 1), 
            ("f12", "F12", 8, 2), 
            ("f13", "F13", 8, 3), 
            ("f14", "F14", 8, 4), 
            ("f15", "F15", 8, 5), 
            ("f16", "F16", 8, 6), 
            ("f17", "F17", 8, 7), 
            ("f18", "F18", 8, 8), 
            ("f19", "F19", 8, 9), 
            ("f20", "F20", 8, 10)
        ]
        # Construct the parts:
        self.buttons = {}
        for key, label, row, column in self.buttonConfig:
            self.buttons[key] = tkinter.Button(self.root, text=label, width=2, height=2, command=lambda key=key: self.buttonCallback(key), bg='black', fg="white", activebackground='black', activeforeground='white', font="Courier") 
            self.buttons[key].grid(row=row, column=column, columnspan=1, sticky="NSEW")
    
        # The GUI has 11 columns and 9 rows.
        self.status_bar = tkinter.Label(self.root, text = "Status bar", bg="black", fg="white", font="Courier")
        self.status_bar.grid(row=0, column=0, columnspan=11, rowspan=1, sticky="nsew")

        self.input_field = tkinter.Label(self.root, text="Input Field", bg="black", fg="white", font="Courier")
        self.input_field.grid(row=1, column=0, columnspan=8, rowspan=2, sticky="nsew")

        self.binary_field = tkinter.Label(self.root, text="Binary Field", bg="black", fg="white", font="Courier")
        self.binary_field.grid(row=3, column=0, columnspan=8, rowspan=2, sticky="nsew")

        self.decimal_field = tkinter.Label(self.root, text="Decimal Field", bg="black", fg="white", font="Courier")
        self.decimal_field.grid(row=5, column=0, columnspan=4, rowspan=2, sticky="nsew")

        self.hexadecimal_field = tkinter.Label(self.root, text="Hexadecimal Field", bg="black", fg="white", font="Courier")
        self.hexadecimal_field.grid(row=5, column=4, columnspan=4, rowspan=2, sticky="nsew")

        # Configure the rows and columns to have the same weight for resizing
        for row in range(0,9):
            self.root.grid_rowconfigure(row, weight = 1)
        for column in range (0,11):
            self.root.grid_columnconfigure(column, weight = 1)

        # Bind key-press to another callback
        self.root.bind("<Key>", self.buttonPress)
    
    def mainloop(self):
        self.updateScreens()
        self.root.mainloop()

    def buttonCallback(self, event):
        print(f"Got callback event: {event=}")
        if event == self.backspace:
            # Remove an input instead
            res = self.core.calc_removeInput()
        else:
            # Add the event as an input to the comscicalc
            res = self.core.calc_addInput(event.lower())
        res = self.core.calc_solver()
        # Update the screen
        self.updateScreens()

    def buttonPress(self, key):
        # Trigger the corresponding button to also be pressed.
        if not hasattr(key, "char"):
            print(f"{key} does not have char attribute")
            return
        buttonKey = key.char
        if buttonKey in self.buttons.keys():
            self.buttons[buttonKey].config(relief = "sunken")
            self.root.update_idletasks()
            self.buttons[buttonKey].invoke()
            time.sleep(0.1)
            self.buttons[buttonKey].config(relief = "raised")
        else:
            print(f"{key}:{buttonKey} is not a defined key and does not have a corresponding button")
    def updateScreens(self):
        """
        Function to update the screens based on the comscicalc state.
        """
        status=self.getStatus()
        self.status_bar.config(text=status)
        inputBuffer, decRes, hexRes, binRes,  issuePos = self.core.calc_printBuffer()
        self.input_field.config(text=inputBuffer)
        # get the different results and print them:
        self.binary_field.config(text=binRes) # TODO: Wrap text
        self.hexadecimal_field.config(text=hexRes)
        self.decimal_field.config(text=decRes)
        
    
    def getStatus(self):
        """
        Function to return the status bar from the comscicalc core. 
        Returns a string with 
        input base: [dec/hex/bin], 
        input format [int, float, fixed]
        output format [int, float, fixed]
        """
        numberFormat = self.core.calcCoreState.numberFormat
        #print(f"{dir(numberFormat)=}")
        numBits = numberFormat.numBits
        inputFormat = numberFormat.inputFormat
        outputFormat = numberFormat.outputFormat
        sign = numberFormat.sign
        inputBase = numberFormat.inputBase
        fixedPointDecimalPlace = numberFormat.fixedPointDecimalPlace
        #print(f"{numBits=}, {inputFormat=}, {outputFormat=}, {sign=}, {inputBase=}, {fixedPointDecimalPlace=}")
        numBitsFormatted = f"{numBits}"
        if inputBaseRepr[inputBase] in ["Fixed Point"]:
            # If fixed point, then calculate that:
            numBitsFormatted = f"TODO"
        return f"{inputBaseRepr[inputBase]}  Bits: {numBitsFormatted}  Input: {inputFormatRepr[inputFormat]}  Output: {inputFormatRepr[outputFormat]}"
        

        
if __name__ == '__main__':

    gui = GUI()
    gui.mainloop()
    