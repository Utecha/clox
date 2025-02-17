#include "arrays.h"
#include "memory.h"

void initByteArray(ByteArray *array)
{
    array->capacity = 0;
    array->count = 0;
    array->data = NULL;
}

void freeByteArray(ByteArray *array)
{
    FREE(uint8_t, array->data, array->capacity);
    initByteArray(array);
}

void writeByteArray(ByteArray *array, uint8_t byte)
{
    if (array->capacity < array->count + 1)
    {
        int capacity = GROW_CAPACITY(array->capacity);
        array->data = REALLOCATE(uint8_t, array->data, array->capacity, capacity);
        array->capacity = capacity;
    }

    array->data[array->count++] = byte;
}

void initLineArray(LineArray *array)
{
    array->capacity = 0;
    array->count = 0;
    array->data = NULL;
}

void freeLineArray(LineArray *array)
{
    FREE(Line, array->data, array->capacity);
    initLineArray(array);
}

void writeLineArray(LineArray *array, Line line)
{
    if (array->capacity < array->count + 1)
    {
        int capacity = GROW_CAPACITY(array->capacity);
        array->data = REALLOCATE(Line, array->data, array->capacity, capacity);
        array->capacity = capacity;
    }

    array->data[array->count++] = line;
}

void initValueArray(ValueArray *array)
{
    array->capacity = 0;
    array->count = 0;
    array->data = NULL;
}

void freeValueArray(ValueArray *array)
{
    FREE(Value, array->data, array->capacity);
    initValueArray(array);
}

void writeValueArray(ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int capacity = GROW_CAPACITY(array->capacity);
        array->data = REALLOCATE(Value, array->data, array->capacity, capacity);
        array->capacity = capacity;
    }

    array->data[array->count++] = value;
}
