# wasm32-clang-toolchain.cmake

# Target platform
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

# Use clang
set(CMAKE_C_COMPILER clang)
set(CMAKE_C_FLAGS_INIT "--target=wasm32 -mbulk-memory -nostdlib -Wall -Werror -Os -DCLAY_WASM")
set(CMAKE_EXE_LINKER_FLAGS_INIT "--target=wasm32 -nostdlib \
  -Wl,--strip-all \
  -Wl,--export-dynamic \
  -Wl,--no-entry \
  -Wl,--export=__heap_base \
  -Wl,--export=ACTIVE_RENDERER_INDEX \
  -Wl,--initial-memory=6553600")

# Avoid standard library detection
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE NEVER)
