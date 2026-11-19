option(NAN_TAGGING
    "If set to ON, enables the usage of NaN-tagged doubles as the core Value \
    representation. This provides a pretty significant boost to performance \
    and memory usage."
    OFF
)
message(STATUS "[Option] NaN Tagging: ${NAN_TAGGING}")

option(BUILD_ORIGINAL
    "If set to ON, builds the original project found in the `original` \
    directory. The binary output target will be named `clox` instead of `lox`."
    OFF
)
message(STATUS "[Option] Build Original: ${BUILD_ORIGINAL}")
