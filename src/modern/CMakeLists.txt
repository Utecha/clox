set(LOX_MODERN lox)
set(LOX_MODERN_VERSION ${CMAKE_PROJECT_VERSION})

add_executable(${LOX_MODERN})
message("Configuring ${LOX_MODERN} Version ${LOX_MODERN_VERSION}")

target_sources(
  ${LOX_MODERN} PRIVATE
  main.c
)

set_target_properties(
  ${LOX_MODERN} PROPERTIES
  VERSION ${LOX_MODERN_VERSION}
  C_STANDARD 23
  C_STANDARD_REQUIRED ON
  C_EXTENSIONS OFF
  ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
  LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
  RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY}
)

target_compile_options(
  ${LOX_MODERN} PRIVATE
  $<$<C_COMPILER_ID:Clang,GNU>:
    -Wall
    -Wextra
    -Wpedantic
    -Werror
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
    /WX
    $<$<CONFIG:Debug>:/Zi>
  >
)

target_compile_definitions(
  ${LOX_MODERN} PRIVATE
  $<$<CONFIG:Debug>:DEBUG>
  NAN_TAGGING=$<IF:$<BOOL:${NAN_TAGGING}>,1,0>
)
