cmake_minimum_required(VERSION 3.11)

if(WIN32)
    include("${CMAKE_SOURCE_DIR}/cmake/windows.cmake")
else()
    include("${CMAKE_SOURCE_DIR}/cmake/linux.cmake")
endif()

project(
    lox
    VERSION 0.0.6
    DESCRIPTION "A class-based scripting language"
    LANGUAGES C CXX
)
message("${PROJECT_NAME} Version: ${CMAKE_PROJECT_VERSION}")

option(SNIPPETS "Enable compiling snippets (-Wno-unused-function)" OFF)
message(STATUS "Compile Snippets: ${SNIPPETS}")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build Type" FORCE)
    message(STATUS "Setting default build type to '${CMAKE_BUILD_TYPE}'")
endif()

add_executable(${PROJECT_NAME})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/src/vm)
add_subdirectory(src)

if(WIN32)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX /wd4100)

    if(SNIPPETS)
        target_compile_options(${PROJECT_NAME} PRIVATE /wd4505)
    endif()

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE /Z7 /DDEBUG)
    endif()
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Werror -Wpedantic -Wno-unused-parameter)

    if(SNIPPETS)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wno-unused-function)
    endif()

    if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        target_compile_options(${PROJECT_NAME} PRIVATE -ggdb -DDEBUG)
    endif()
    target_link_libraries(${PROJECT_NAME} readline)
endif()
