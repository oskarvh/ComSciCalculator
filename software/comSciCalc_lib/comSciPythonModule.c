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
#define MAX_LEN_RESULT_BUFFER 255
/* ----------------- HEADERS ----------------- */

// Python extended library - used to glue the C codebase to python
#include <Python.h>
// Standard lib
#include <string.h>
// comsci header file - to link the comsci functions here
#include "comscicalc.h"
#include "comscicalc_operators.h"

// Make a global instance of the calculator core
// Might be a bit of a hack, and I'm not sure if this method is leaky, 
// but it seems to work. This needs to be investigated though. 
calcCoreState_t comSciCalc_core;

/* ----------------- MAIN -------------------- */
// Function callable from python
static PyObject * _comSciCalc_Init(PyObject *self){
	// Initialize the core state
	if(calc_coreInit(&comSciCalc_core) != calc_funStatus_SUCCESS){
		// Initialization failed
		return Py_BuildValue("s", "Error: Initialization failed");
	}
	
	return Py_BuildValue("");	
}

static PyObject * _comSciCalc_Teardown(PyObject *self){
	// Initialize the core state
	// Teardown core
	if(calc_coreBufferTeardown(&comSciCalc_core) != calc_funStatus_SUCCESS){
		return Py_BuildValue("s", "Error: Teardown failed");
	}
	
	return Py_BuildValue("");	
}

static PyObject * _comSciCalc_PrintBuffer(PyObject *self){
	// Allocate buffer to print. 
	char pResString[MAX_LEN_RESULT_BUFFER] = {0};
	// Get the string from the calculator core
	if(calc_printBuffer(&comSciCalc_core, pResString, MAX_LEN_RESULT_BUFFER)){
		return Py_BuildValue("s", "Error: Print buffer failed");
	}

	return Py_BuildValue("s", pResString);	
}

static PyObject * _comSciCalc_AddInput(PyObject *self, PyObject *args){
	char *inputString;
	int inputBase;
	int cursorPos;
	if(!PyArg_ParseTuple(args, "sii", &inputString, &inputBase, &cursorPos)){
		return NULL;
	}

	// Set the input base
	comSciCalc_core.inputBase = inputBase;
	comSciCalc_core.cursorPosition = cursorPos;

	// Loop through the input string, and add input at cursor
	while( *inputString != '\0'){
		// Add input. 
		calc_funStatus_t inputStatus = calc_addInput(&comSciCalc_core, *inputString);
		if(inputStatus != calc_funStatus_SUCCESS){
			printf("Warning: adding c=[%c], dec=[%i] at cursorPos %d failed. Status: %i\r\n", 
				*inputString, *inputString, cursorPos, inputStatus);
		}
		inputString++;
	}
}

static PyObject * _comSciCalc_DeleteInput(PyObject *self, PyObject *args){
	int cursorPos;
	if(!PyArg_ParseTuple(args, "i", &cursorPos)){
		return NULL;
	}

	comSciCalc_core.cursorPosition = cursorPos;

	calc_funStatus_t inputStatus = inputStatus = calc_removeInput(&comSciCalc_core);
	if(inputStatus != calc_funStatus_SUCCESS){
		printf("Warning: Removing char at cursor %d failed. Status: %i\r\n", 
			cursorPos, inputStatus);
	}
}


static PyObject * _comSciCalc(PyObject *self, PyObject *args){
	// Read string from python. Note, python will allocate space, 
	// just need a pointer to the string. 
	char *inputString;
	int inputBase;
	char *cursorPos;
	if(!PyArg_ParseTuple(args, "si", &inputString, &inputBase)){
		return NULL;
	}

	char pResString[MAX_LEN_RESULT_BUFFER] = {0};

	

	// Initialize the core state
	if(calc_coreInit(&comSciCalc_core) != calc_funStatus_SUCCESS){
		// Initialization failed
		return Py_BuildValue("s", "Error: Initialization failed");
	}
	comSciCalc_core.inputBase = inputBase;

	// Loop through the input string, and either add or remove entries
	while( *inputString != '\0'){
		calc_funStatus_t inputStatus = calc_funStatus_SUCCESS;
		if(*inputString == 8){
			// 8 is backspace in ascii. 
			inputStatus = calc_removeInput(&comSciCalc_core, *inputString);
		}
		else {
			// If not backspace then add input. 
			inputStatus = calc_addInput(&comSciCalc_core, *inputString);
		}
		if(inputStatus != calc_funStatus_SUCCESS){
			printf("Warning: adding/removing c=[%c], dec=[%i] failed. Status: %i\r\n", 
				*inputString, *inputString, inputStatus);
		}

		inputString++;
	}

	// Get the string from the calculator core
	if(calc_printBuffer(&comSciCalc_core, pResString, MAX_LEN_RESULT_BUFFER)){
		return Py_BuildValue("s", "Error: Print buffer failed");
	}

	// Teardown core
	if(calc_coreBufferTeardown(&comSciCalc_core) != calc_funStatus_SUCCESS){
		return Py_BuildValue("s", "Error: Teardown failed");
	}

	return Py_BuildValue("s", pResString);
}


static struct PyMethodDef methods[] = {
	{"comSciCalc", (PyCFunction)_comSciCalc, METH_VARARGS}, 
	{"comSciCalc_Init", (PyCFunction)_comSciCalc_Init, METH_NOARGS},
	{"comSciCalc_Teardown", (PyCFunction)_comSciCalc_Teardown, METH_NOARGS},
	{"comSciCalc_PrintBuffer", (PyCFunction)_comSciCalc_PrintBuffer, METH_NOARGS},
	{"comSciCalc_AddInput", (PyCFunction)_comSciCalc_AddInput, METH_VARARGS},
	{"comSciCalc_DeleteInput", (PyCFunction)_comSciCalc_DeleteInput, METH_VARARGS},
	// Add move functions here? 
	{NULL, NULL}
};


static struct PyModuleDef module = {
	PyModuleDef_HEAD_INIT, 
	"_comSciCalc", // __name__
	NULL,  //__doc__
	-1, 
	methods,
};


PyMODINIT_FUNC PyInit__comSciCalc(void){
	return PyModule_Create(&module);
}