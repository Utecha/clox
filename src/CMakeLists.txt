message("Configuring ${LOX} v${LOX_VERSION} ...")

set(SOURCES
    debug.c
    main.c
    memory.c
    value.c
    vm.c
)

add_executable(${LOX} ${SOURCES})

set_target_properties(
    ${LOX} PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
    LIBRARY_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_TARGET}
)

target_compile_options(
    ${LOX} PRIVATE
    $<$<C_COMPILER_ID:MSVC>:
        /W4
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
    ${LOX} PRIVATE
    $<$<CONFIG:Debug>:DEBUG>
    NAN_TAGGING=$<IF:$<BOOL:${NAN_TAGGING}>,1,0>
)
