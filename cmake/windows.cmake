set(CMAKE_SYSTEM_NAME Windows)
message(STATUS "Using Windows toolchain")

set(CMAKE_C_STANDARD 99)
message(STATUS "C Standard: ${CMAKE_C_STANDARD}")

set(CMAKE_C_EXTENSIONS OFF)
message(STATUS "C Extensions: ${CMAKE_C_EXTENSIONS}")

# From here, if MSVC is not installed, CMake will let the user know or
# will otherwise automatically detect it.
