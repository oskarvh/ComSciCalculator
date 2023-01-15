# Written by Oskar von Heideken
# The aim of this script is to get an overview of how the computer scientists calculator 
# would work, in the GUI parts but also to check what functions would be nice. 
# It uses Tkinter as the GUI interface. 
from time import sleep
import sys
import argparse
from tkinter import *
import tkinter as tk #HACK! 
#from tkinter.ttk import *
import logging

# Class for handling logging with colors and proper formatting. 
# Borrowed from https://stackoverflow.com/questions/14097061/easier-way-to-enable-verbose-logging
class CustomFormatter(logging.Formatter):
    grey = "\x1b[38;20m"
    yellow = "\x1b[33;20m"
    red = "\x1b[31;20m"
    bold_red = "\x1b[31;1m"
    reset = "\x1b[0m"
    format = "%(asctime)s - %(name)s - %(levelname)s - %(message)s (%(filename)s:%(lineno)d)"

    FORMATS = {
        logging.DEBUG: grey + format + reset,
        logging.INFO: grey + format + reset,
        logging.WARNING: yellow + format + reset,
        logging.ERROR: red + format + reset,
        logging.CRITICAL: bold_red + format + reset
    }

    def format(self, record):
        log_fmt = self.FORMATS.get(record.levelno)
        formatter = logging.Formatter(log_fmt)
        return formatter.format(record)

class Brackets:
    def __init__(self, start, end, color):
        self.start = start
        self.end = end
        self.color = color

# Custom text class to handle the input text widget. 
# This is becoming very Frankensteined, I most likely do not need it. 
class CustomText:
    colorWheel = ["white", "deep sky blue", "cyan", "green", "yellow", "pink", "purple3"]
    colorWheelIndex = 0
    inputMode = "Decimal"
    inputTextWidget = None
    statusWidget = None
    inputString = ""
    
    def __init__(self, inputWidget, inputWidgetName):
        self.inputMode = "Decimal"
        self.inputTextWidget = inputWidget
        self.statusWidget = inputWidgetName
        self.statusWidget.config(text="Input mode: "+self.inputMode)

    # This function gets the text from the input and places it in the queue
    def inputText(self, event):

        #logger.debug(event.keysym)
        inputChar = event.char
        # Filter the characters and do different string manipulations where needed. 
        if event.keysym in ("BackSpace", "Delete"):
            # Handle deletion. 
            logger.debug("Delete char.")
            self.inputString = self.inputString[:-1]
        if event.keysym in ("Up", "Down", "Left", "Right"):
            # Handle moving about
            logger.debug("Move cursor " + event.keysym)
        if event.keysym in ("0", "1") and self.inputMode is "Binary":
            logger.debug("Binary input")
            self.inputString += inputChar
        if event.keysym in ("0", "1", "2", "3", "4", "5", "6", "7", "8", "9") and self.inputMode is "Decimal":
            logger.debug("Decimal input")
            self.inputString += inputChar
        if event.keysym.lower() in ("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "a", "b", "c", "d", "e", "f") and self.inputMode is "Hex":
            logger.debug("Hexadecimal input")
            self.inputString += inputChar
        # Chars that should always be allowed:
        if event.char in ("(", ")", "&", "^", "|", "~", "*", "+", "-"):
            # Add that to the string. 
            self.inputString += inputChar

        #logger.debug("Input string: " + self.inputString)
        # Update the display once it's done. 
        self.displayText()

    # This function updates the display
    def displayText(self):
        # First, enable the state again in order to write to it:
        self.inputTextWidget.config(state=tk.NORMAL)
        # Write the string:
        self.inputTextWidget.delete("1.0", tk.END)
        self.inputTextWidget.insert(tk.END,self.inputString)
        
        # Loop through the test string and look for opening and closing brackets:
        index = 0 # this is the index of the character
        depth = 0
        openingBrackets = []
        formattingTags = []
        for c in self.inputString:
            # If we found an opening bracket, we should change the color of the text. 
            # That means adding a new tag, where we need to save the index we're at right now
            if c is "(":
                depth += 1
                color = self.colorWheel[depth%len(self.colorWheel)]
                openingBrackets.append(Brackets("1." + str(index), tk.END, color))

            # If we find a closing bracket, we need to match that to the latest 
            # opening bracket
            if c is ")":
                # If there is any opening brackets
                if len(openingBrackets) > 0:
                    # then close the bracket and save the formatting:
                    closedBracket = openingBrackets.pop()
                    formattingTags.append(Brackets(closedBracket.start, "1." + str(index+1), closedBracket.color))
                    depth -= 1
                else:
                    # no opening brackets available, mark the closing bracket as red, as it's
                    # not matching. 
                    formattingTags.append(Brackets("1."+str(index), "1."+str(index+1), "red"))

            index += 1
        # Check if any open brackets are left:
        for bracket in reversed(openingBrackets):
            closedBracket = openingBrackets.pop()
            formattingTags.append(Brackets(closedBracket.start, tk.END, closedBracket.color))

        index = 0
        for bracket in reversed(formattingTags):
            logger.debug("start = " + bracket.start)
            logger.debug("end = " + bracket.end)
            logger.debug("Color = " + bracket.color)
            logger.debug(" ")
            self.inputTextWidget.tag_add(str(index), bracket.start, bracket.end)
            self.inputTextWidget.tag_config(str(index), background=backGroundColor, foreground=bracket.color)
            index += 1
        # Disable the widget so that no one can write to it. 
        self.inputTextWidget.config(state=tk.DISABLED)



#---------------------------------------------------------------------
#--------------------------MAIN---------------------------------------
#---------------------------------------------------------------------
# Parse arguments
parser = argparse.ArgumentParser(description="Computer scientists calculator")
parser.add_argument(
    '-d', '--debug',
    help="Print lots of debugging statements",
    action="store_const", dest="loglevel", const=logging.DEBUG,
    default=logging.WARNING,
)
parser.add_argument(
    '-v', '--verbose',
    help="Be verbose",
    action="store_const", dest="loglevel", const=logging.INFO,
)
args = parser.parse_args()

# instanciate logger
logger = logging.getLogger("ComSciCalc")
logger.setLevel(logging.DEBUG)
# create console handler with a higher log level
ch = logging.StreamHandler()
ch.setLevel(level=args.loglevel)
ch.setFormatter(CustomFormatter())

logger.addHandler(ch)
# Test: 
logger.debug("debug message") # Displayed for debug flag
logger.info("info message") # Displayed for debug and verbose flag
logger.warning("warning message") # Always displayed
logger.error("error message") # Always displayed
logger.critical("critical message") # Always displayed

# GUI
root = Tk()
root.geometry("800x480")
root.title(" Computer Scientists Calculator ")
backGroundColor = "#000000" #"#26242f"
foreGroundColor = "#ffffff"
root.config(bg=backGroundColor) #26242f
# We need one input window and three output windows:
inputWidgetName = Label(text = "Input", fg=foreGroundColor, bg=backGroundColor, font="Courier 10")
inputWidget = Text(root, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
#inputWidget = CustomText(root, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
#inputWidget.bind("<<TextModified>>",inputWidget.getText)
#inputWidget.config(fg="white")
inputWidget.config(state=DISABLED)
# hex widget:
hexWidgetName = Label(text = "Hex", fg=foreGroundColor, bg=backGroundColor, font="Courier 10")
hexWidget = Text(root, height = 3, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
hexWidget.config(state=DISABLED)
#decimal:
decWidgetName = Label(text = "Dec", fg=foreGroundColor, bg=backGroundColor, font="Courier 10")
decWidget = Text(root, height = 3, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
decWidget.config(state=DISABLED)
# float:
floatWidgetName = Label(text = "IEEE 754 floating point", fg=foreGroundColor, bg=backGroundColor, font="Courier 10")
floatWidget = Text(root, height = 3, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
floatWidget.config(state=DISABLED)
#Binary:
binWidgetName = Label(text = "Binary", fg=foreGroundColor, bg=backGroundColor, font="Courier 10")
binWidget = Text(root, height = 3, bg = backGroundColor, fg = foreGroundColor, font="Courier 15")
binWidget.config(state=DISABLED)
# Pack the different windows:
inputWidgetName.grid(row=0, column=0,  columnspan=2)
inputWidget.grid(row=1, column=0, columnspan=2)
hexWidgetName.grid(row=3, column=0)
decWidgetName.grid(row=3, column=1)
hexWidget.grid(row=4, column=0)
decWidget.grid(row=4, column=1)
floatWidgetName.grid(row=5, column=0,  columnspan=2)
floatWidget.grid(row=6, column=0, columnspan=2)
binWidgetName.grid(row=7, column=0, columnspan=2)
binWidget.grid(row=8, column=0, columnspan=2)
root.columnconfigure(0, weight=1)
root.columnconfigure(1, weight=1)
root.rowconfigure(1, weight=1)


# re-bind the keypress:
inputHandler = CustomText(inputWidget, inputWidgetName)
root.bind("<KeyPress>", inputHandler.inputText)

# Start the GUI main loop
root.mainloop()