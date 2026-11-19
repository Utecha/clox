cmake_minimum_required(VERSION 3.25)

# Add the 'cmake' directory to the CMake module path for easier includes.
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Declare the project.
# NOTE: The project version is set/re-used within the respective subdirectories
# for the main ("modern") project and the original.
project(
  lox
  VERSION 0.0.1
  DESCRIPTION
    "A fast, object-oriented C-style scripting language implemented in C."
  LANGUAGES C
)

#-------------------------------------------------------------------------------

# Export 'compile_commands.json' for use with LSP's.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set a default build type if one is not provided.
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
  message(STATUS "No build type set. Defaulting to '${CMAKE_BUILD_TYPE}'.")
endif()

# Normalize the output directory by platform and toolchain.
if(WIN32 AND ("${CMAKE_C_COMPILER_ID}" STREQUAL "MSVC"))
  set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
else()
  set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")
endif()

#-------------------------------------------------------------------------------

# Include project specific options.
# >> (cmake/options.cmake)
include(options)

# Add the original source directories' CMakeLists if `BUILD_ORIGINAL` is set
# to 'ON'.
if(BUILD_ORIGINAL)
  add_subdirectory(src/original)
endif()

# Add the modern source directories' CMakeLists.
add_subdirectory(src/modern)
