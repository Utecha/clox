#ifndef lox_utils_h
#define lox_utils_h

#include "common.h"

#define DECLARE_ARRAY(name, type)                               \
    typedef struct                                              \
    {                                                           \
        int capacity;                                           \
        int count;                                              \
        type *data;                                             \
    } name##Array;                                              \
    void init##name##Array(name##Array *array);                 \
    void free##name##Array(name##Array *array);                 \
    void write##name##Array(name##Array *array, type data)

#define DEFINE_ARRAY(name, type)                                                        \
    void init##name##Array(name##Array *array)                                          \
    {                                                                                   \
        array->capacity = 0;                                                            \
        array->count = 0;                                                               \
        array->data = NULL;                                                             \
    }                                                                                   \
    void free##name##Array(name##Array *array)                                          \
    {                                                                                   \
        FREE(type, array->data, array->capacity);                                       \
        init##name##Array(array);                                                       \
    }                                                                                   \
    void write##name##Array(name##Array *array, type data)                              \
    {                                                                                   \
        if (array->capacity < array->count + 1)                                         \
        {                                                                               \
            int capacity = GROW_CAPACITY(array->capacity);                              \
            array->data = REALLOCATE(type, array->data, array->capacity, capacity);     \
            array->capacity = capacity;                                                 \
        }                                                                               \
                                                                                        \
        array->data[array->count++] = data;                                             \
    }

typedef struct
{
    int number;
    int offset;
} Line;

DECLARE_ARRAY(Byte, uint8_t);
DECLARE_ARRAY(Line, Line);

#endif // lox_utils_h
