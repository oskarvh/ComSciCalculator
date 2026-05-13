# wasm32-wasi-toolchain.cmake — uses wasi-sdk for full libc support

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

set(CMAKE_C_COMPILER /opt/wasi-sdk/bin/wasm32-wasi-clang)

# -nostartfiles: skip crt1.o entry-point machinery; libc (malloc, sprintf…) is still linked
set(CMAKE_C_FLAGS "-DCLAY_WASM -Os -Wall -Werror"
    CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS
    "-nostartfiles \
     -Wl,--no-entry \
     -Wl,--export-dynamic \
     -Wl,--export=__wasm_call_ctors \
     -Wl,--strip-all \
     -Wl,--initial-memory=12582912"
    CACHE STRING "" FORCE)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
