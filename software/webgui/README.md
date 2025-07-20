# ComSciCalc Web GUI
This directory contains the build files for the webgui for ComSciCalc. 

The web gui has a few core functions: 
- A standalone version of the Computer Scientist Calculator. 
- A synchronized front end to the hardware (optinal): If the hardware is connected to the computer, the webgui will synchronize with the hardware, and the hardware can be used to control the webgui.
- An interface to patch/add functions without having to build the entire firmware. 


## Setup
Install the dependencies, and run the build instructions from the build section.

### Dependencies
- **CMake** (version 3.20 or higher)
- A POSIX-compatible shell (Linux/macOS). Windows users can use Git Bash or WSL.
- **Python3** (for running the web server)
- **clang** (version 18 or higher) Compiler for wasm. Install via e.g., `apt install clang`
- **lld** (version 18 or higher) Linker to use alongside clang. Install via e.g., `apt install lld`


## Build

To build and compile the webassembly file:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/wasm-toolchain.cmake
cmake --build build
```

## Run
The ComSciCalc is compiled into webassembly, and in order to run the webassembly application, a local http server is required. 

Start the server:
```bash
python3 -m http.server 8000
```

Open [`http://localhost:8000/index.html`](http://localhost:8000/index.html) in your browser. 