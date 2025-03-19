#ifndef lox_memory_h
#define lox_memory_h

#include "common.h"

#define GROW_CAPACITY(capacity)                                                         \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define ALLOCATE(vm, type)                                                              \
    ((type *)reallocate(vm, NULL, 0, sizeof(type)))

#define ALLOCATE_ARRAY(vm, type, size)                                                  \
    ((type *)reallocate(vm, NULL, 0, sizeof(type) * (size)))

#define REALLOCATE(vm, type, ptr, old_size, new_size)                                   \
    ((type *)reallocate(vm, ptr, sizeof(type) * (old_size), sizeof(type) * (new_size)))

#define DEALLOCATE(vm, type, ptr)                                                       \
    (reallocate(vm, ptr, sizeof(type), 0))

#define DEALLOCATE_ARRAY(vm, type, ptr, old_size)                                       \
    (reallocate(vm, ptr, sizeof(type) * (old_size), 0))

void *reallocate(LoxVM *vm, void *ptr, size_t old_size, size_t new_size);
void free_objects(LoxVM *vm);

#endif // lox_memory_h
