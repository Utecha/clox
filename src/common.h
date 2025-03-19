#ifndef lox_common_h
#define lox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Compiler Compiler;
typedef struct LoxVM LoxVM;

// Debug Flags
#ifdef DEBUG
    #define DEBUG_DUMP_CODE 0
    #define DEBUG_DUMP_TOKENS 0
    #define DEBUG_TRACE_INSTRUCTIONS 1
    #define DEBUG_LOG_GC 1
    #define DEBUG_LOG_MEMORY 1
    #define DEBUG_STRESS_GC 0
#endif // DEBUG

// Internal Debugging Defines (stolen from Wren <3)
#ifdef DEBUG
    #include <stdio.h>

    #define ASSERT(condition, msg)                                          \
        do                                                                  \
        {                                                                   \
            fprintf(stderr, "{ %s : %d } Assertion failed in %s(): %s\n",   \
                    __FILE__, __LINE__, __func__, msg);                     \
            abort();                                                        \
        } while (false)

    #define UNREACHABLE()                                                       \
        do                                                                      \
        {                                                                       \
            fprintf(stderr, "{ %s : %d } Unreachable code reached in %s()\n",   \
                    __FILE__, __LINE__, __func__);                              \
            abort();                                                            \
        } while (false)

    #define UNUSED(x)   ((void)x)
#else // DEBUG
    #define ASSERT(condition, msg) do {} while (false)

    #if defined (_MSC_VER)
        #define UNREACHABLE() __assume(0)
    #elif (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5))
        #define UNREACHABLE() __builtin_unreachable()
    #else
        #define UNREACHABLE()
    #endif // _MSC_VER / __GNUC__

    #define UNUSED(x)
#endif // DEBUG

#endif // lox_common_h
