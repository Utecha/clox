#ifndef LOX_MEMORY_H
#define LOX_MEMORY_H

#include "lox.h"

//--------------------------------------------------------------------------------------------------

#define LOX_ALLOC(Type, vm) \
    (Type *)lox_reallocate(vm, nullptr, 0, sizeof(Type))

#define LOX_ALLOCA(Type, vm, size) \
    (Type *)lox_reallocate(vm, nullptr, 0, sizeof(Type) * (size))

#define LOX_ALLOCF(MainType, ArrayType, vm, size) \
    (MainType *)lox_reallocate(vm, nullptr, 0, sizeof(MainType) + sizeof(ArrayType) * (size))

#define LOX_REALLOC(Type, vm, ptr, old_size, new_size) \
    (Type *)lox_reallocate(vm, ptr, sizeof(Type) * (old_size), sizeof(Type) * (new_size))

#define LOX_FREE(Type, vm, ptr) \
    lox_reallocate(vm, ptr, sizeof(Type), 0)

#define LOX_FREEA(Type, vm, ptr, size) \
    lox_reallocate(vm, ptr, sizeof(Type) * (size), 0)

#define LOX_FREEF(MainType, ArrayType, vm, ptr, size) \
    lox_reallocate(vm, ptr, sizeof(MainType) + sizeof(ArrayType) * (size), 0)

//--------------------------------------------------------------------------------------------------

size_t lox_next_power_of_two(size_t n);
void *lox_reallocate(LoxVM *vm, void *ptr, ssize_t old_size, ssize_t new_size);

#endif // LOX_MEMORY_H
