message("Configuring ${CLOX} v${CLOX_VERSION} ...")

set(SOURCES
    chunk.c
    compiler.c
    debug.c
    main.c
    memory.c
    object.c
    scanner.c
    table.c
    value.c
    vm.c
)

add_executable(${CLOX} ${SOURCES})

set_target_properties(
    ${CLOX} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
)

target_compile_options(
    ${CLOX} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:
        /W2
        $<$<CONFIG:Debug>:/Zi>
    >
    $<$<C_COMPILER_ID:GNU>:
        -Wall
        -Wextra
        $<$<NOT:$<PLATFORM_ID:Windows>>:
            $<$<CONFIG:Debug>:-ggdb>
        >
    >
    $<$<C_COMPILER_ID:Clang>:
        -Wall
        -Wextra
        $<$<NOT:$<PLATFORM_ID:Windows>>:
            $<$<CONFIG:Debug>:-glldb>
        >
    >
)

target_compile_definitions(
    ${CLOX} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    NAN_TAGGING=$<IF:$<BOOL:${NAN_TAGGING}>,1,0>
)

# The `readline` library technically exists for Windows, but it's not exactly
# straight forward to install and make available for use, so it is omitted.
# It is otherwise available for both Linux and MacOS and on many distros is
# installed by default.
if(NOT WIN32)
    target_link_libraries(${CLOX} PRIVATE readline)
endif()
