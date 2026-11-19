#include <stdio.h>

#include "lox_memory.h"
#include "lox_vm.h"

//--------------------------------------------------------------------------------------------------

/**
 * The default internal implementation of the `LoxReallocateFn`.
**/
static inline void *default_reallocate(void *ptr, ssize_t size);

//--------------------------------------------------------------------------------------------------

LoxVM *lox_new_vm(LoxReallocateFn reallocate)
{
    if (!reallocate) reallocate = default_reallocate;

    LoxVM *vm = (LoxVM *)reallocate(nullptr, sizeof(LoxVM));
    vm->bytes_allocated = 0;
    vm->reallocate = reallocate;

    return vm;
}

void lox_free_vm(LoxVM *vm)
{
    LOX_FREE(LoxVM, vm, vm);
}

//--------------------------------------------------------------------------------------------------

static inline void *default_reallocate(void *ptr, ssize_t size)
{
    if (size == 0)
    {
        free(ptr);
        return nullptr;
    }

    return realloc(ptr, (size_t)size);
}
