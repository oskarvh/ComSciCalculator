from setuptools import setup
from setuptools import Extension

setup(
    name = "comSciCalc-lib", 
    version = "0.1", 
    ext_modules = [Extension("_comSciCalc", ["comSciCalc_lib/comSciPythonModule.c"])],
)