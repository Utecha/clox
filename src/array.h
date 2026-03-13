#ifndef LOX_ARRAY_H
#define LOX_ARRAY_H

#define DECLARE_ARRAY(Type, name)                                               \
    typedef struct                                                              \
    {                                                                           \
        uint32_t capacity;                                                      \
        uint32_t length;                                                        \
        Type *data;                                                             \
    } name##Array;                                                              \
    void init##name##Array(name##Array *array);                                 \
    void clear##name##Array(LoxVM *vm, name##Array *array);                     \
    void fill##name##Array(LoxVM *vm, name##Array *array, Type data,            \
        uint32_t count);                                                        \
    void write##name##Array(LoxVM *vm, name##Array *array, Type data)

#define DEFINE_ARRAY(Type, name)                                                \
    void init##name##Array(name##Array *array)                                  \
    {                                                                           \
        array->capacity = 0;                                                    \
        array->length = 0;                                                      \
        array->data = NULL;                                                     \
    }                                                                           \
    void clear##name##Array(LoxVM *vm, name##Array *array)                      \
    {                                                                           \
        reallocate(vm, array->data, sizeof(Type) * array->capacity, 0);         \
        init##name##Array(array);                                               \
    }                                                                           \
    void fill##name##Array(LoxVM *vm, name##Array *array, Type data,            \
        uint32_t count)                                                         \
    {                                                                           \
        if (array->capacity < array->length + count)                            \
        {                                                                       \
            uint32_t capacity = nextPowerOfTwo(array->length + count);          \
            array->data = (Type *)reallocate(vm, array->data,                   \
                sizeof(Type) * array->capacity, sizeof(Type) * capacity);       \
            array->capacity = capacity;                                         \
        }                                                                       \
                                                                                \
        for (uint32_t i = 0; i < count; i++)                                    \
        {                                                                       \
            array->data[array->length++] = data;                                \
        }                                                                       \
    }                                                                           \
    void write##name##Array(LoxVM *vm, name##Array *array, Type data)           \
    {                                                                           \
        fill##name##Array(vm, array, data, 1);                                  \
    }

#endif // LOX_ARRAY_H
