#include <stdio.h>
#include <string.h>
#include "include/memory.h"
#include "include/object.h"
#include "include/value.h"

bool values_equal(Value a, Value b)
{
#ifdef NAN_BOXING
    return a == b;
#else
    if (a.type != b.type) return false;
    switch (a.type) {
        case VL_BOOL:
            return AS_BOOL(a) == AS_BOOL(b);
        case VL_NIL:
            return true;
        case VL_NUMBER:
            return AS_NUMBER(a) == AS_NUMBER(b);
        case VL_OBJ:
            return AS_OBJ(a) == AS_OBJ(b);
        default:
            return false; // Unreachable
    }
#endif
}

void init_value_array(ValueArray *array)
{
    array->capacity = 0;
    array->count = 0;
    array->values = NULL;
}

void free_value_array(ValueArray *array)
{
    FREE_ARRAY(Value, array->values, array->capacity);
    init_value_array(array);
}

void write_value_array(ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1) {
        int old = array->capacity;
        array->capacity = GROW_CAPACITY(old);
        array->values = GROW_ARRAY(Value, array->values, old, array->capacity);
    }

    array->values[array->count] = value;
    array->count++;
}

void print_value(Value value)
{
#ifdef NAN_BOXING
    if (IS_BOOL(value)) {
        printf(AS_BOOL(value) ? "true" : "false");
    } else if (IS_NIL(value)) {
        printf("nil");
    } else if (IS_NUMBER(value)) {
        printf("%.32g", AS_NUMBER(value));
    } else if (IS_OBJ(value)) {
        print_object(value);
    }
#else
    switch (value.type) {
        case VL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VL_NIL:
            printf("nil");
            break;
        case VL_NUMBER:
            printf("%.32g", AS_NUMBER(value));
            break;
        case VL_OBJ:
            print_object(value);
            break;
    }
#endif
}