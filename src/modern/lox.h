#ifndef LOX_H
#define LOX_H

#include <stdarg.h>
#include <stddef.h> // For ptrdiff_t
#include <stdint.h>
#include <stdlib.h>

//--------------------------------------------------------------------------------------------------

/**
 * Defines the result types that are returned by the Lox runtime.
**/
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

#endif // LOX_H
