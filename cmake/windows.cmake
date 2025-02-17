## Platform Name
set(CMAKE_SYSTEM_NAME Windows)
message(STATUS "Using Windows toolchain")

## C Standard
set(CMAKE_C_STANDARD 99)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")

## C Extensions
set(CMAKE_C_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")

## Find appropriate compilers
# On Windows, for now, this defaults to MSVC
find_program(MSVC_COMPILER cl)

if(MSVC_COMPILER)
    set(CMAKE_C_COMPILER ${MSVC_COMPILER} CACHE STRING "C Compiler")
    set(CMAKE_CXX_COMPILER ${MSVC_COMPILER} CACHE STRING "C++ Compiler")
    message(STATUS "Compiling using MSVC")
else()
    message(FATAL_ERROR "Could not find the MSVC compiler. Double check that it's installed.")
endif()
