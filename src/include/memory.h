#ifndef CLOX_MEMORY_H
#define CLOX_MEMORY_H

#include "common.h"
#include "object.h"

#define ALLOCATE(type, count)   \
    (type *)reallocate(NULL, 0, sizeof(type) * (count))

#define FREE(type, ptr) reallocate(ptr, sizeof(type), 0)

#define FREE_ARRAY(type, ptr, old_count)    \
    reallocate(ptr, sizeof(type) * (old_count), 0)

#define GROW_CAPACITY(capacity)     \
    ((capacity) < 8 ? 8 : (capacity) * 2)

#define GROW_ARRAY(type, ptr, old_count, new_count)     \
    (type *)reallocate(ptr, sizeof(type) * (old_count), \
        sizeof(type) * (new_count))

void *reallocate(void *ptr, size_t old_size, size_t new_size);
void mark_object(Obj *object);
void mark_value(Value value);
void collect_garbage();
void free_objects();

#endif // CLOX_MEMORY_H
