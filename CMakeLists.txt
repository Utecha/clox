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

project(
    lox
    VERSION 0.0.6
    DESCRIPTION "A class-based scripting language"
    LANGUAGES C CXX
)

message("${PROJECT_NAME} Version: ${CMAKE_PROJECT_VERSION}")

#-------------------------------------------------------------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
    message(STATUS "Setting default build type to '${CMAKE_BUILD_TYPE}'")
endif()

#-------------------------------------------------------------------------------

set(BINARY_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")

#-------------------------------------------------------------------------------

option(NAN_TAGGING
    "Enables the use of NaN tagged doubles as the Value representation, \
    leading to improved performance and memory usage."
    OFF
)

message(STATUS "NaN Tagging: ${NAN_TAGGING}")

#-------------------------------------------------------------------------------

add_subdirectory(src)
