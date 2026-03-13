set(CMAKE_SYSTEM_NAME Apple)
message(STATUS "Using Apple toolchain")

set(CMAKE_C_STANDARD 99)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")

set(CMAKE_C_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")

# Clang is preferred so we check for it first
find_program(CLANG clang)

if(CLANG)
    set(CMAKE_C_COMPILER ${CLANG} CACHE FILEPATH "C Compiler")
    message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
else()
    # If clang isn't found, fall back to gcc.
    find_program(GCC gcc)

    if(GCC)
        set(CMAKE_C_COMPILER ${GCC} CACHE FILEPATH "C Compiler")
        message(STATUS "Compiler: ${CMAKE_C_COMPILER}")
    else()
        # On MacOS, like Linux, we could theoretically fall back to something
        # like tcc, however it is significantly more rare for any user to have
        # so instead we error out here.
        message(FATAL_ERROR
            "Could not find a supported C compiler. "
            "Ensure that you have either `gcc` or `clang` installed and "
            "available in your PATH!"
        )
    endif()
endif()
