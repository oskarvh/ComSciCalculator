/*
 * Copyright (c) 2023
 * Oskar von Heideken. 
 *
 * Wrapper to glue the computer scientist calculator (comsci) C code to
 * to a Python module, using the python.h library. 
 * 
 * The python code can either act as a test harness, or host a GUI
 * but with the codebase for the calculator core being common with the
 * embedded system. 
 * 
 * 
 * ------------------- DEV LOGS -------------------
 * March 5, 2023
 * There are two obvious ways to do this:
 * 1. Python creates a thread with an instance of the computer scientist calculator
 *    and uses input and output streams to communicate between python and the calculator core. 
 *    Python then hosts the GUI, and relays the input to the calculator core, and 
 *    displays the output from the calculator core. 
 *    The instance of the calculator core would handle all input parsing and data storing
 *    while the instance is alive. 
 *    This is the preferred approach. 
 * 2. Python creates a temporary instance of the calculator, passing 
 *    the user input as a string of keyboard inputs. 
 *    The calculator would then parse that input string and directly calculate 
 *    the result. 
 *    For this approach, python would have to handle some of the string parsing, such as deletion
 *    and thereby cursor position as well. 
 * 
 */

/* ----------------- DEFINES ----------------- */
#define PY_SSIZE_T_CLEAN
/* ----------------- HEADERS ----------------- */

// Python extended library - used to glue the C codebase to python
#include <Python.h>

// comsci header file - to link the comsci functions here
#include "comscicalc.h"


/* ----------------- MAIN -------------------- */
// Function callable from python
static PyObject * test_comSciCalc(PyObject *self, PyObject *args){
	// String object: PyUnicode_FromString("String")
}

static struct PyMethodDef methods[] {
	{"comSciPython", (PyCFunction)test_comSciCalc, METH_VARARGS}, 
	{NULL, NULL}
};

static struct PyModuleDef module {
	PyModuleDef _HEAD_INIT, 
	"comSciPython", 
	NULL, 
	-1, 
	methods
};

PyMODINIT_FUNC PyInit_comSciPython(void){
	return PyModule_Create(&module)
}