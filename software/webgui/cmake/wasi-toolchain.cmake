# wasi-toolchain.cmake

set(CMAKE_SYSTEM_NAME WASI)
set(CMAKE_SYSTEM_PROCESSOR wasm32)

set(WASI_SDK_ROOT "${CMAKE_SOURCE_DIR}/external/wasi-sdk")

set(CMAKE_C_COMPILER "${WASI_SDK_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${WASI_SDK_ROOT}/bin/clang++")

set(CMAKE_AR "${WASI_SDK_ROOT}/bin/llvm-ar")
set(CMAKE_LINKER "${WASI_SDK_ROOT}/bin/wasm-ld")
set(CMAKE_RANLIB "${WASI_SDK_ROOT}/bin/llvm-ranlib")

set(CMAKE_C_FLAGS "")
set(CMAKE_CXX_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "")

add_compile_options(
  --target=wasm32-wasi
  --sysroot=${WASI_SDK_ROOT}/share/wasi-sysroot
)

add_link_options(
  --target=wasm32-wasi
  --sysroot=${WASI_SDK_ROOT}/share/wasi-sysroot
)

# Tell CMake to only try compiling in tests, not linking/running
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
