set(CMAKE_SYSTEM_NAME Windows)
message(STATUS "Using Windows toolchain")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")
message(STATUS "C++ Extensions: ${CMAKE_CXX_EXTENSIONS}")

# I compile it as clang, so for myself and others I start
# by attempting to find it
find_program(CLANG clang)
find_program(CLANGPP clang++)

# Lets the user know what compilers will be used based on
# what is available on their system. If all else fails, everyone
# *should* have `cc/c++` to fall back on. The project *should* still
# compile perfectly fine with either `gcc/g++` or `cc/c++`.
if(CLANG OR CLANGPP)
    message(STATUS "Compiling using (clang/clang++)")
else()
    find_program(MSVCC cl)

    if(MSVCC)
        message(STATUS "Compiling using MSVC")
    else()
        message(FATAL_ERROR "Could not find a valid C/C++ compiler. \
            Double check your PATH.")
    endif()
endif()

# Begin the chain of determining what compilers to set. It's only
# really as complex as it is to prevent weirdness from a user having
# say, `clang` but not `clang++`, and then setting the C++ compiler
# to effectively NULL, or vice-versa.
if(CLANG AND NOT CLANGPP)
    set(CMAKE_C_COMPILER ${CLANG} CACHE STRING "C Compiler")
elseif(CLANGPP AND NOT CLANG)
    set(CMAKE_CXX_COMPILER ${CLANGPP} CACHE STRING "C++ Compiler")
elseif(CLANG AND CLANGPP)
    set(CMAKE_C_COMPILER ${CLANG} CACHE STRING "C Compiler")
    set(CMAKE_CXX_COMPILER ${CLANGPP} CACHE STRING "C++ Compiler")
else()
    # Windows does not have a 'system default' compiler like `cc/c++` on
    # Linux. As a result, when the initial calls to find_program are done,
    # if none result in a compiler, it is set to error out and report that
    # to the user. That means if we get here, MSVC is certainly found.
    set(CMAKE_C_COMPILER ${MSVCC} CACHE STRING "C Compiler")
    set(CMAKE_CXX_COMPILER ${MSVCC} CACHE STRING "C++ Compiler")
endif()
