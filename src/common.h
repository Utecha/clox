#ifndef LOX_COMMON_H
#define LOX_COMMON_H

// Used to clearly mark a flexible array member in a struct.
#define FLEXIBLE

/*
 * If true (1), Lox uses a NaN-tagged double for its core Value representation.
 * Otherwise, it uses a tagged union (struct + enum type tag).
 *
 * The former is significantly faster and more compact, however the latter is
 * useful for debugging and is more portable.
 *
 * Defaults to enabled.
*/
#ifndef NAN_TAGGING
    #define NAN_TAGGING 0
#endif // WREN_NAN_TAGGING

/*
 * Below are various debugging flags that are useful when hacking on the VM.
 *
 * Even if set to true (1), they are only truly enabled when compiling in
 * debug mode.
*/

#define DEBUG_DUMP_CODE 0
#define DEBUG_DUMP_TOKENS 0
#define DEBUG_STRESS_GC 0
#define DEBUG_TRACE_GC 0
#define DEBUG_TRACE_MEMORY 0
#define DEBUG_TRACE_EXECUTION 0

/*
 * Defines a limit constant for the number of constants that may be defined in
 * a single script.
 *
 * This is hard coded in the VM by the 16-bit argument for constant
 * instructions.
*/
#define MAX_CONSTANTS (1 << 16)

// Debugging
#ifdef DEBUG
    #include <stdio.h>

    /*
     * Assertions are used to validate program invariants. They indicate things
     * that the program expects to be true about its internal state while
     * executing. If an assertion fails, there is a bug in Wren.
     *
     * Assertions add significant overhead, so they are only enabled in debug
     * builds.
    */
    #define ASSERT(condition, message)                                          \
        do                                                                      \
        {                                                                       \
            if (!(condition))                                                   \
            {                                                                   \
                fprintf(stderr, "{ %s : %d } Assertion failed in %s(): %s\n",   \
                    __FILE__, __LINE__, __func__, message);                     \
                abort();                                                        \
            }                                                                   \
        } while (false)

    /*
     * Indicates that we know execution should never reach this point in the
     * program. In debug mode, we assert this because reaching that point is a
     * bug.
     *
     * In release mode, we use compiler-specific built in functions in order
     * to tell the compiler that the code can't be reached. This avoids any
     * warnings for "missing return" and, in some cases, also lets the
     * compiler perform certain optimizations by assuming that the code is
     * unreachable.
    */
    #define UNREACHABLE()                                                       \
        do                                                                      \
        {                                                                       \
            fprintf(stderr, "{ %s : %d } Unreachable code reached in %s()\n",   \
                __FILE__, __LINE__, __func__);                                  \
            abort();                                                            \
        } while (false)
#else
    #define ASSERT(condition, message) do {} while (false)

    // Tell the compiler that this point in the code is unreachable.
    #if defined(_MSC_VER)
        #define UNREACHABLE() __assume(0)
    #elif ((__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 5)) || defined(__clang__))
        // Newer versions of clang include the same built in function, so use it
        // in either case.
        #define UNREACHABLE() __builtin_unreachable()
    #else
        #define UNREACHABLE()
    #endif // UNREACHABLE
#endif // DEBUG

#endif // LOX_COMMON_H
