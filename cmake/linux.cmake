set(CMAKE_SYSTEM_NAME Linux)
message(STATUS "Using Linux toolchain")

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
    find_program(GCC gcc)
    find_program(GPP g++)

    if(GCC OR GPP)
        message(STATUS "Compiling using (gcc/g++)")
    else()
        message(STATUS "Compiling using the system default compiler (cc/c++)")
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
    if(GCC AND NOT GPP)
        set(CMAKE_C_COMPILER ${GCC} CACHE STRING "C Compiler")
    elseif(GPP AND NOT GCC)
        set(CMAKE_CXX_COMPILER ${GPP} CACHE STRING "C++ Compiler")
    elseif(GCC AND GPP)
        set(CMAKE_C_COMPILER ${GCC} CACHE STRING "C Compiler")
        set(CMAKE_CXX_COMPILER ${GPP} CACHE STRING "C++ Compiler")
    else()
        set(CMAKE_C_COMPILER cc CACHE STRING "C Compiler")
        set(CMAKE_CXX_COMPILER c++ CACHE STRING "C++ Compiler")
    endif()
endif()
