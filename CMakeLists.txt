cmake_minimum_required(VERSION 3.15)

#-------------------------------------------------------------------------------

if(WIN32)
    include("${CMAKE_SOURCE_DIR}/cmake/windows.cmake")
elseif(APPLE)
    include("${CMAKE_SOURCE_DIR}/cmake/macos.cmake")
elseif(LINUX)
    include("${CMAKE_SOURCE_DIR}/cmake/linux.cmake")
endif()

#-------------------------------------------------------------------------------

set(CLOX clox)
set(CLOX_VERSION 0.1.0)

set(LOX lox)
set(LOX_VERSION 0.0.1)

project(
    ${LOX}
    VERSION ${LOX_VERSION}
    DESCRIPTION
        "A fast, object-oriented C-style scripting language implemented in C."
    LANGUAGES C
)

#-------------------------------------------------------------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
    message(STATUS "No build type set. Defaulting to '${CMAKE_BUILD_TYPE}'.")
endif()

#-------------------------------------------------------------------------------

# Normalizes the output location for the `clox` binary.
#
# For example, if you're compiling in Debug mode on Windows using MSVC, the
# location of `CMAKE_CURRENT_BINARY_DIR` would be `build/Debug`. If compiling
# with anything other than MSVC on Windows, or if you're on any other OS, the
# compilers default output location is directly in `build`.
#
# This ensures that if you're not specifically on Windows and compiling with
# MSVC, the binary is always output to the `build/Debug` directory (or
# `build/Release` on release builds.)
if(WIN32)
    if("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC")
        set(BINARY_OUTPUT_TARGET "${CMAKE_CURRENT_BINARY_DIR}")
    else()
        set(BINARY_OUTPUT_TARGET
            "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}"
        )
    endif()
else()
    set(BINARY_OUTPUT_TARGET "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()

#-------------------------------------------------------------------------------

option(NAN_TAGGING
    "If set to ON, enables the usage of NaN-tagged doubles as the core Value \
    representation. This provides a pretty significant boost to performance \
    and memory usage."
    OFF
)
message(STATUS "[Option] NaN Tagging: ${NAN_TAGGING}")

option(BUILD_ORIGINAL
    "If set to ON, builds the original project found in the `original` \
    directory. The binary output target will be named `clox` instead of `lox`."
    ON
)
message(STATUS "[Option] Build Original: ${BUILD_ORIGINAL}")

#-------------------------------------------------------------------------------

add_subdirectory(src)

if(BUILD_ORIGINAL)
    add_subdirectory(original)
endif()
