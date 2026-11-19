set(LOX_ORIGINAL clox)
set(LOX_ORIGINAL_VERSION 0.1.0)

add_executable(${LOX_ORIGINAL})
message("Configuring ${LOX_ORIGINAL} Version ${LOX_ORIGINAL_VERSION}")

target_sources(
  ${LOX_ORIGINAL} PRIVATE
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

set_target_properties(
  ${LOX_ORIGINAL} PROPERTIES
  VERSION ${LOX_ORIGINAL_VERSION}
  C_STANDARD 99
  C_STANDARD_REQUIRED OFF
  C_EXTENSIONS OFF
  ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
)

target_compile_options(
  ${LOX_ORIGINAL} PRIVATE
  $<$<C_COMPILER_ID:Clang,GNU>:
    -Wall
    -Wextra
    -Wno-unused-parameter
  >
  $<$<C_COMPILER_ID:Clang>:
    $<$<NOT:$<PLATFORM_ID:Windows>>:
      $<$<CONFIG:Debug>:-glldb>
    >
  >
  $<$<C_COMPILER_ID:GNU>:
    $<$<NOT:$<PLATFORM_ID:Windows>>:
      $<$<CONFIG:Debug>:-ggdb>
    >
  >
  $<$<C_COMPILER_ID:MSVC>:
    /W4
    $<$<CONFIG:Debug>:/Zi>
  >
)

target_compile_definitions(
  ${LOX_ORIGINAL} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  NAN_TAGGING=$<IF:$<BOOL:${NAN_TAGGING}>,1,0>
)

# The `readline` library technically exists for Windows, but it's not exactly
# straight forward to install and make available for use, so it is omitted.
# It is otherwise available for both Linux and MacOS and on many distros is
# installed by default.
if(NOT WIN32)
  target_link_libraries(${LOX_ORIGINAL} PRIVATE readline)
endif()
