#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory.h"
#include "table.h"
#include "value.h"
#include "vm.h"

static void print_object(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_STRING:
        {
            printf("%s", AS_CSTRING(value));
        } break;
    }
}

void print_value(Value value)
{
    switch (value.type)
    {
        case VAL_BOOL:      printf(AS_BOOL(value) ? "true" : "false"); break;
        case VAL_NIL:       printf("nil"); break;
        case VAL_NUMBER:    printf("%g", AS_NUMBER(value)); break;
        case VAL_OBJ:       print_object(value); break;
        default:            UNREACHABLE();
    }
}

bool values_equal(Value a, Value b)
{
    if (a.type != b.type) return false;
    switch (a.type)
    {
        case VAL_BOOL:      return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:       return true;
        case VAL_NUMBER:    return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:       return AS_OBJ(a) == AS_OBJ(b);
        default:            UNREACHABLE();
    }
}

static void init_obj(LoxVM *vm, Obj *obj, ObjType type)
{
    obj->type = type;
    obj->next = vm->objects;
    vm->objects = obj;
}

static ObjString *allocate_string(LoxVM *vm, char *data, int length, uint32_t hash)
{
    ObjString *string = ALLOCATE(vm, ObjString);
    init_obj(vm, &string->obj, OBJ_STRING);
    string->length = length;
    string->hash = hash;
    string->data = data;
    set_to_table(vm, &vm->strings, string, NIL_VAL);
    return string;
}

static uint32_t hash_string(const char *data, int length)
{
    uint32_t hash = 2166136261U;

    for (int i = 0; i < length; i++)
    {
        hash ^= (uint8_t)data[i];
        hash *= 16777619;
    }

    return hash;
}

ObjString *copy_string(LoxVM *vm, const char *data, int length)
{
    uint32_t hash = hash_string(data, length);
    ObjString *interned = find_interned_string(&vm->strings, data, length, hash);
    if (interned) return interned;

    char *string = ALLOCATE_ARRAY(vm, char, length + 1);
    memcpy(string, data, length);
    string[length] = '\0';
    return allocate_string(vm, string, length, hash);
}

ObjString *take_string(LoxVM *vm, char *data, int length)
{
    uint32_t hash = hash_string(data, length);
    ObjString *interned = find_interned_string(&vm->strings, data, length, hash);
    if (interned)
    {
        DEALLOCATE_ARRAY(vm, char, data, length + 1);
        return interned;
    }

    return allocate_string(vm, data, length, hash);
}

bool string_equals_cstring(ObjString *string, const char *cstring, int length, uint32_t hash)
{
    return string->length == length &&
        string->hash == hash &&
        memcmp(string->data, cstring, length) == 0;
}

void init_value_array(ValueArray *array)
{
    array->count = 0;
    array->capacity = 0;
    array->values = NULL;
}

void free_value_array(LoxVM *vm, ValueArray *array)
{
    DEALLOCATE_ARRAY(vm, Value, array->values, array->capacity);
    init_value_array(array);
}

void write_value_array(LoxVM *vm, ValueArray *array, Value value)
{
    if (array->capacity < array->count + 1)
    {
        int capacity = GROW_CAPACITY(array->capacity);
        array->values = REALLOCATE(vm, Value, array->values, array->capacity, capacity);
        array->capacity = capacity;
    }

    array->values[array->count++] = value;
}
