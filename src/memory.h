#ifndef LOX_MEMORY_H
#define LOX_MEMORY_H

#include "lox.h"

//------------------------------------------------------------------------------

#define ALLOCATE(Type, vm)                                                      \
    (Type *)reallocate(vm, NULL, 0, sizeof(Type))

#define ALLOCATE_ARRAY(Type, vm, size)                                          \
    (Type *)reallocate(vm, NULL, 0, sizeof(Type) * (size))

#define ALLOCATE_FLEX(MainType, ArrayType, vm, size)                            \
    (MainType *)reallocate(vm, NULL, 0, sizeof(MainType) + sizeof(ArrayType) *  \
        (size))

#define REALLOCATE(Type, vm, ptr, old_size, new_size)                           \
    (Type *)reallocate(vm, ptr, sizeof(Type) * (old_size),                      \
        sizeof(Type) * (new_size))

#define FREE(Type, vm, ptr)                                                     \
    reallocate(vm, ptr, sizeof(Type), 0)

#define FREE_ARRAY(Type, vm, ptr, size)                                         \
    reallocate(vm, ptr, sizeof(Type) * (size), 0)

#define FREE_FLEX(MainType, ArrayType, vm, ptr, array_size)                     \
    reallocate(vm, ptr, sizeof(MainType) + sizeof(ArrayType) * (array_size), 0)

//------------------------------------------------------------------------------

uint32_t nextPowerOfTwo(uint32_t n);
void *reallocate(LoxVM *vm, void *ptr, size_t old_size, size_t new_size);

#endif // LOX_MEMORY_H
