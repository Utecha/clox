#ifndef LOX_H
#define LOX_H

#include <stdarg.h>
#include <stddef.h> // For ptrdiff_t
#include <stdint.h>
#include <stdlib.h>

/* Used to stringify the argument passed into the macro. */
#ifndef _STR
    #define _STR(x) #x
    #define _MKSTR(x) _STR(x)
#endif // _STR

/* Defines the current Lox API semantic versioning components. */
#define LOX_VERSION_MAJOR 0
#define LOX_VERSION_MINOR 0
#define LOX_VERSION_PATCH 1

/* Defines the current Lox API semantic version as a string. */
#define LOX_VERSION_STRING \
    _MKSTR(LOX_VERSION_MAJOR) "." _MKSTR(LOX_VERSION_MINOR) "." _MKSTR(LOX_VERSION_PATCH)

/**
 * Defines a monotonic representation of the current Lox API semantic version.
 *
 * This can be used to do range checks over different versions.
**/
#define LOX_VERSION                 \
    (LOX_VERSION_MAJOR * 1000000) + \
    (LOX_VERSION_MINOR * 1000) +    \
    LOX_VERSION_PATCH

/**
 * The following defines handle the detection of the current platform.
**/

#ifndef LOX_PLATFORM_WIN32
    #if defined(_WIN32) || defined(__WIN32__) || defined(__MINGW32__)
        #define LOX_PLATFORM_WIN32 1
    #else
        #define LOX_PLATFORM_WIN32 0
    #endif // _WIN32
#endif // LOX_PLATFORM_WIN32

#ifndef LOX_PLATFORM_APPLE
    #ifdef __APPLE__
        #define LOX_PLATFORM_APPLE 1
    #else
        #define LOX_PLATFORM_APPLE 0
    #endif // __APPLE__
#endif // LOX_PLATFORM_APPLE

#ifndef LOX_PLATFORM_LINUXBSD
    #if defined(__linux__) || defined(__gnu_linux__)
        #define LOX_PLATFORM_LINUXBSD 1
    #elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
        #define LOX_PLATFORM_LINUXBSD 1
    #else
        #define LOX_PLATFORM_LINUXBSD 0
    #endif // __linux__
#endif // LOX_PLATFORM_LINUXBSD

/**
 * We require the use of the `ssize_t` type just as often if not more than we need any of the
 * other types defined in `stdint.h`. This handles its inclusion in a cross-platform manner.
**/
#if LOX_PLATFORM_WIN32
    #include <BaseTsd.h>
    typedef SSIZE_T ssize_t;
#else
    #include <sys/types.h>
#endif // _MSC_VER

/**
 * Handles the definition of the `LOX_API` tag in a cross-platform manner.
**/
#ifndef LOX_API
    #if LOX_PLATFORM_WIN32
        #if defined(LOX_API_DLL_EXPORT)
            #if defined(__GNUC__) || defined(__clang__)
                #define LOX_API __attribute__ ((dllexport))
            #else
                #define LOX_API __declspec (dllexport)
            #endif // __GNUC__
        #else
            #if defined(__GNUC__) || defined(__clang__)
                #define LOX_API __attribute__ ((dllimport))
            #else
                #define LOX_API __declspec (dllimport)
            #endif // __GNUC__
        #endif // LOX_API_DLL_EXPORT
    #else
        #if defined(__GNUC__) || defined(__clang__) && defined(LOX_API_DLL_EXPORT)
            #define LOX_API __attribute__ ((visibility ("default")))
        #else
            #define LOX_API
        #endif // __GNUC__
    #endif // LOX_PLATFORM_WIN32
#endif // LOX_API

//--------------------------------------------------------------------------------------------------

/* Defines the result types that are returned by the Lox runtime. */
typedef enum
{
    LOX_RESULT_SUCCESS,
    LOX_RESULT_COMPILE_ERROR,
    LOX_RESULT_RUNTIME_ERROR
} LoxRuntimeResult;

/**
 * A single instance of a Lox Virtual Machine.
 *
 * Lox (the current implementation) does not have any global state. All state for a running virtual
 * machine lives within this structure.
**/
typedef struct LoxVM LoxVM;

/**
 * Defines the type for a pointer to a generic allocation function which handles the explicit memory
 * management within a Lox VM.
**/
typedef void *(*LoxReallocateFn)(void *ptr, ssize_t size);

//--------------------------------------------------------------------------------------------------

/**
 * Returns a pointer to an allocated and initialized Lox VM instance, using the optional
 * [reallocate] parameter to configure the allocator for the VM.
 *
 * A default implementation is provided if [reallocate] is passed a null pointer.
**/
LOX_API LoxVM *lox_new_vm(LoxReallocateFn reallocate);

/**
 * Releases the memory held by a Lox VM instance, including itself.
**/
LOX_API void lox_free_vm(LoxVM *vm);

#endif // LOX_H
