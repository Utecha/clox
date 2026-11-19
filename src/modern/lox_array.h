#ifndef LOX_ARRAY_H
#define LOX_ARRAY_H

#include "lox_memory.h"

//--------------------------------------------------------------------------------------------------

/**
 * Macro which defines the struct for a dynamic array of type [data_type], using [name] to name the
 * struct with the pattern `[name]Array`.
**/
#define lox_define_array_t(data_type, name)                                                         \
    typedef struct                                                                                  \
    {                                                                                               \
        ssize_t capacity;                                                                           \
        ssize_t length;                                                                             \
        data_type *data;                                                                            \
    } name##Array

/**
 * Initializes a dynamic array struct that was defined using `lox_define_array_t`.
**/
#define lox_init_array(array)                                                                       \
    do                                                                                              \
    {                                                                                               \
        (array)->capacity = 0;                                                                      \
        (array)->length = 0;                                                                        \
        (array)->data = nullptr;                                                                    \
    } while (false)

/**
 * Releases the memory held by [array] and clears (zeros) the pointer.
**/
#define lox_clear_array(vm, array)                                                                  \
    do                                                                                              \
    {                                                                                               \
        lox_reallocate(vm, (array)->data, sizeof(typeof(*(array)->data)) * (array)->capacity, 0);   \
        lox_init_array(array);                                                                      \
    } while (false)

/**
 * Resizes the given [array] by the [expected] number of elements to add.
**/
#define lox_resize_array(vm, array, expected)                                                       \
    do                                                                                              \
    {                                                                                               \
        if ((array)->capacity < (array)->length + (expected))                                       \
        {                                                                                           \
            ssize_t capacity = (ssize_t)lox_next_power_of_two((array)->length + (expected));        \
            (array)->data = (typeof((array)->data))lox_reallocate(vm, (array)->data,               \
                sizeof(typeof(*(array)->data)) * (array)->capacity,                                 \
                sizeof(typeof(*(array)->data)) * capacity);                                         \
            (array)->capacity = capacity;                                                           \
        }                                                                                           \
    } while (false)

/**
 * Fills the given [array] with the given [value] by [count] times.
**/
#define lox_array_fill(vm, array, value, count)                                                     \
    do                                                                                              \
    {                                                                                               \
        lox_resize_array(vm, array, count);                                                         \
        for (ssize_t i = 0; i < (ssize_t)(count); i++)                                              \
        {                                                                                           \
            (array)->data[(array)->length++] = (value);                                             \
        }                                                                                           \
    } while (false)

/**
 * Appends a single [value] to the given [array].
**/
#define lox_array_append(vm, array, value)                                                          \
    do                                                                                              \
    {                                                                                               \
        lox_resize_array(vm, array, 1);                                                             \
        (array)->data[(array)->length++] = (value);                                                 \
    } while (false)

#endif // LOX_ARRAY_H
