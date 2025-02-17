#ifndef lox_memory_h
#define lox_memory_h

#include "common.h"

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define ALLOCATE(type) \
    ((type *)reallocate(NULL, 0, sizeof(type)))

#define ALLOCATE_ARRAY(type, size) \
    ((type *)reallocate(NULL, 0, sizeof(type) * (size)))

#define ALLOCATE_FLEX(mainType, arrayType, size) \
    ((mainType *)reallocate(NULL, 0, \
        sizeof(mainType) + sizeof(arrayType) * (size)))

#define REALLOCATE(type, ptr, oldSize, newSize) \
    ((type *)reallocate(ptr, sizeof(type) * (oldSize), \
        sizeof(type) * (newSize)))

#define FREE(type, ptr, oldSize) \
    (reallocate(ptr, sizeof(type) * (oldSize), 0))

#define DEALLOCATE(ptr) (reallocate(ptr, 0, 0))

void *reallocate(void *ptr, size_t oldSize, size_t newSize);
void freeObjects(LoxVM *vm);

#endif // lox_memory_h
