#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

DEFINE_ARRAY(Value, Value);

static void initObj(LoxVM *vm, Obj *obj, ObjType type)
{
    obj->type = type;
    obj->isDark = false;
    obj->next = vm->objects;
    vm->objects = obj;
}

ObjFn *newFunction(LoxVM *vm)
{
    ObjFn *function = ALLOCATE(ObjFn);
    initObj(vm, &function->obj, OBJ_FUNCTION);
    function->arity = 0;
    initChunk(&function->chunk);
    function->name = NULL;
    // initByteArray(&function->code);
    // initValueArray(&function->constants);
    // function->debug.name = NULL;
    // initLineArray(&function->debug.lines);

    return function;
}

ObjNative *newNative(LoxVM *vm, NativeFn function, int arity)
{
    ObjNative *native = ALLOCATE(ObjNative);
    initObj(vm, &native->obj, OBJ_NATIVE);
    native->function = function;
    native->arity = arity;
    return native;
}

static ObjString *allocateString(LoxVM *vm, size_t length)
{
    ObjString *string = ALLOCATE_FLEX(ObjString, char, length + 1);
    initObj(vm, &string->obj, OBJ_STRING);
    string->length = (uint32_t)length;
    string->value[length] = '\0';
    return string;
}

// static void hashString(ObjString *string)
// {
//     uint32_t hash = 2166136261u;

//     for (uint32_t i = 0; i < string->length; i++)
//     {
//         hash ^= string->value[i];
//         hash *= 16777619;
//     }

//     string->hash = hash;
// }

static uint32_t hashCString(const char *chars, uint32_t length)
{
    uint32_t hash = 2166136261u;

    for (uint32_t i = 0; i < length; i++)
    {
        hash ^= chars[i];
        hash *= 16777619;
    }

    return hash;
}

// TODO: Come up with a better way to manage interning strings so that
// there is no need to even have, let alone call, 'hashCString'.
// Unless '-DSNIPPETS=ON' is provided to the cmake config, 'hashString'
// must be commented out in order to compile now.
ObjString *newStringLength(LoxVM *vm, const char *chars, size_t length)
{
    uint32_t hash = hashCString(chars, (uint32_t)length);
    ObjString *interned = tableFindString(&vm->strings, chars, (uint32_t)length, hash);
    if (interned != NULL) return interned;

    ObjString *string = allocateString(vm, length);

    if (length > 0 && chars != NULL)
        { memcpy(string->value, chars, length); }

    string->hash = hash;
    tableSet(&vm->strings, string, NIL_VAL);
    return string;
}

ObjString *newString(LoxVM *vm, char *chars)
{
    return newStringLength(vm, chars, strlen(chars));
}

static void printFunction(ObjFn *function)
{
    if (function->name == NULL)
    {
        printf("<fn main>");
        return;
    }

    printf("<fn %s>", function->name->value);
}

static void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
        case OBJ_FUNCTION:
            printFunction(AS_FUNCTION(value));
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}

void printValue(Value value)
{
    switch (value.type)
    {
        case VAL_BOOL:
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL:
            printf("nil");
            break;
        case VAL_NUMBER:
            printf("%.127g", AS_NUMBER(value));
            break;
        case VAL_OBJ:
            printObject(value);
            break;
    }
}

bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

bool valuesEqual(Value a, Value b)
{
    if (a.type != b.type) return false;
    switch (a.type)
    {
        case VAL_BOOL:      return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:       return true;
        case VAL_NUMBER:    return AS_NUMBER(a) == AS_NUMBER(b);
        case VAL_OBJ:       return AS_OBJ(a) == AS_OBJ(b);
        default:            return false; // Unreachable
    }
}
