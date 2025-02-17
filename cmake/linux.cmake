## Platform Name
set(CMAKE_SYSTEM_NAME Linux)
message(STATUS "Using Linux toolchain")

## C Standard
set(CMAKE_C_STANDARD 99)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")

## C Extensions
set(CMAKE_C_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")

## Find appropriate compilers
# This *attempts* to default to clang, but falls back to either gcc
# or the default cc, if necessary.
find_program(CLANG_C_COMPILER clang)
find_program(GCC_C_COMPILER gcc)

if(CLANG_C_COMPILER)
    message(STATUS "Compiling using clang")
    set(CMAKE_C_COMPILER ${CLANG_C_COMPILER} CACHE STRING "C Compiler")
elseif(GCC_C_COMPILER)
    message(STATUS "Compiling using gcc")
    set(CMAKE_C_COMPILER ${GCC_C_COMPILER} CACHE STRING "C Compiler")
else()
    message(STATUS "Compiling using the system default compiler (cc)")
endif()
