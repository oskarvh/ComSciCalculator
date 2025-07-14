# ComSciCalc Web GUI
This directory contains the build files for the webgui for ComSciCalc. 

The web gui has a few core functions: 
- A standalone version of the Computer Scientist Calculator. 
- A synchronized front end to the hardware (optinal): If the hardware is connected to the computer, the webgui will synchronize with the hardware, and the hardware can be used to control the webgui.
- An interface to patch/add functions without having to build the entire firmware. 


## Setup
Run the setup script: `./script/setup.sh`. This adds the [wasi-sdk](https://github.com/WebAssembly/wasi-sdk/) to `external`. 

Note that this will fetch the compiler and linker required by the build.

### Dependencies
- **CMake** (version 3.20 or higher)
- **curl** (for downloading the WASI SDK)
- A POSIX-compatible shell (Linux/macOS). Windows users can use Git Bash or WSL.
- **Python3** (for running the web server)


## Build

To build and compile the webassembly file:
```bash
cmake -B build -DCMAKE_TOOLCHAIN_FILE=cmake/wasi-toolchain.cmake
cmake --build build
```

## Run
The ComSciCalc is compiled into webassembly, and in order to run the webassembly application, a local http server is required. 

Start the server:
```bash
python3 -m http.server 8000
```

Open [`http://localhost:8000/index.html`](http://localhost:8000/index.html) in your browser. 