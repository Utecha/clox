set(CMAKE_SYSTEM_NAME Apple)
message(STATUS "Using Apple toolchain")

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")

set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_CXX_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")
message(STATUS "C++ Extensions: ${CMAKE_CXX_EXTENSIONS}")

# MacOS users who don't have clang will get prompted to install it if you
# attempt to use it. Therefore, no real checks are needed. It is at this point
# kind of the goto C/C++ compiler set for many including myself.
message(STATUS "Compiling using (clang/clang++)")
set(CMAKE_C_COMPILER clang CACHE STRING "C Compiler")
set(CMAKE_CXX_COMPILER clang++ CACHE STRING "C++ Compiler")
