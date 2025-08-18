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

#-------------------------------------------------------------------------------

add_executable(${PROJECT_NAME} ${SOURCES})

#-------------------------------------------------------------------------------

set_target_properties(
    ${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR}
)

target_compile_options(
    ${PROJECT_NAME} PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:
        -Wall
        -Wextra
        $<$<STREQUAL:$<CONFIG>,Debug>:-ggdb>
    >
    $<$<PLATFORM_ID:Windows>:
        /W2
        $<$<STREQUAL:$<CONFIG>,Debug>:/Zi>
    >
)

target_compile_definitions(
    ${PROJECT_NAME} PRIVATE
    $<$<STREQUAL:$<CONFIG>,Debug>:DEBUG>
    $<$<BOOL:${NAN_TAGGING}>:NAN_TAGGING>
)

if(NOT WIN32)
    target_link_libraries(${PROJECT_NAME} readline)
endif()
