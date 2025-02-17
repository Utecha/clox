#include <stdio.h>
#include <string.h>

#include "memory.h"
#include "value.h"
#include "vm.h"

static void initObj(LoxVM *vm, Obj *obj, ObjType type)
{
    obj->type = type;
    obj->isDark = false;
    obj->next = vm->objects;
    vm->objects = obj;
}

static ObjString *allocateString(LoxVM *vm, size_t length)
{
    ObjString *string = ALLOCATE_FLEX(ObjString, char, length + 1);
    initObj(vm, &string->obj, OBJ_STRING);
    string->length = (uint32_t)length;
    string->value[length] = '\0';
    return string;
}

void hashString(ObjString *string)
{
    uint32_t hash = 2166136261u;

    for (uint32_t i = 0; i < string->length; i++)
    {
        hash ^= string->value[i];
        hash *= 16777619;
    }

    string->hash = hash;
}

ObjString *newStringLength(LoxVM *vm, const char *chars, size_t length)
{
    ObjString *string = allocateString(vm, length);

    if (length > 0 && chars != NULL)
        { memcpy(string->value, chars, length); }

    hashString(string);
    return string;
}

ObjString *newString(LoxVM *vm, char *chars)
{
    return newStringLength(vm, chars, strlen(chars));
}

static void printObject(Value value)
{
    switch (OBJ_TYPE(value))
    {
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
            printf("%g", AS_NUMBER(value));
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
        case VAL_OBJ:
        {
            ObjString *aString = AS_STRING(a);
            ObjString *bString = AS_STRING(b);
            return aString->length == bString->length &&
                aString->hash == bString->hash &&
                memcmp(aString->value, bString->value, aString->length) == 0;
        } break;
        default:            return false; // Unreachable
    }
}
