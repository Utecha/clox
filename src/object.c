#include <stdio.h>
#include <string.h>
#include "include/memory.h"
#include "include/object.h"
#include "include/table.h"
#include "include/value.h"
#include "include/vm.h"

#define ALLOCATE_OBJ(type, object_type)     \
    (type *)allocate_object(sizeof(type), object_type)

static Obj *allocate_object(size_t size, ObjType type)
{
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    object->is_marked = false;
    object->next = vm.objects;
    vm.objects = object;

#ifdef DEBUG_LOG_GC
    printf("Addr: %p -- Allocate %zu bytes | Type %d\n", (void *)object, size, type);
#endif

    return object;
}

ObjString *allocate_string(char *chars, int len, uint32_t hash)
{
    ObjString *string = ALLOCATE_OBJ(ObjString, OBJ_STRING);
    string->len = len;
    string->chars = chars;
    string->hash = hash;

    push(OBJ_VAL(string));
    table_set(&vm.strings, string, NIL_VAL);

    pop();
    return string;
}

static uint32_t hash_string(const char *key, int len)
{
    uint32_t hash = 2166136261u;
    for (int i = 0; i < len; i++) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }

    return hash;
}

ObjBoundMethod *new_bound_method(Value receiver, ObjClosure *method)
{
    ObjBoundMethod *bound = ALLOCATE_OBJ(ObjBoundMethod, OBJ_BOUND_METHOD);
    bound->receiver = receiver;
    bound->method = method;
    return bound;
}

ObjClass *new_class(ObjString *name)
{
    ObjClass *klass = ALLOCATE_OBJ(ObjClass, OBJ_CLASS);
    klass->name = name;
    init_table(&klass->methods);
    return klass;
}

ObjClosure *new_closure(ObjFunction *function)
{
    ObjUpvalue **upvalues = ALLOCATE(ObjUpvalue *, function->upvalue_count);
    for (int i = 0; i < function->upvalue_count; i++) {
        upvalues[i] = NULL;
    }

    ObjClosure *closure = ALLOCATE_OBJ(ObjClosure, OBJ_CLOSURE);
    closure->function = function;
    closure->upvalues = upvalues;
    closure->upvalue_count = function->upvalue_count;
    return closure;
}

ObjFunction *new_function()
{
    ObjFunction *function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);
    function->arity = 0;
    function->upvalue_count = 0;
    function->name = NULL;
    init_chunk(&function->chunk);
    return function;
}

ObjInstance *new_instance(ObjClass *klass)
{
    ObjInstance *instance = ALLOCATE_OBJ(ObjInstance, OBJ_INSTANCE);
    instance->klass = klass;
    init_table(&instance->fields);
    return instance;
}

ObjNative *new_native(ObjString *name, NativeFn function)
{
    ObjNative *native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    return native;
}

ObjUpvalue *new_upvalue(Value *slot)
{
    ObjUpvalue *upvalue = ALLOCATE_OBJ(ObjUpvalue, OBJ_UPVALUE);
    upvalue->closed = NIL_VAL;
    upvalue->location = slot;
    upvalue->next = NULL;
    return upvalue;
}

ObjString *take_string(char *chars, int len)
{
    uint32_t hash = hash_string(chars, len);
    ObjString *interned = table_find_string(&vm.strings, chars, len, hash);
    if (interned != NULL) {
        FREE_ARRAY(char, chars, len + 1);
        return interned;
    }

    return allocate_string(chars, len, hash);
}

ObjString *copy_string(const char *chars, int len)
{
    uint32_t hash = hash_string(chars, len);
    ObjString *interned = table_find_string(&vm.strings, chars, len, hash);
    if (interned != NULL) return interned;

    char *heap_chars = ALLOCATE(char, len + 1);
    memcpy(heap_chars, chars, len);
    heap_chars[len] = '\0';
    return allocate_string(heap_chars, len, hash);
}

static void print_function(ObjFunction *function)
{
    if (function->name == NULL) {
        printf("<script>");
        return;
    }

    printf("<user func %s>", function->name->chars);
}

void print_object(Value value)
{
    switch (OBJ_TYPE(value)) {
        case OBJ_BOUND_METHOD:
            print_function(AS_BOUND_METHOD(value)->method->function);
            break;
        case OBJ_CLASS:
            printf("%s Class", AS_CLASS(value)->name->chars);
            break;
        case OBJ_CLOSURE:
            print_function(AS_CLOSURE(value)->function);
            break;
        case OBJ_FUNCTION:
            print_function(AS_FUNCTION(value));
            break;
        case OBJ_INSTANCE:
            printf("%s Instance", AS_INSTANCE(value)->klass->name->chars);
            break;
        case OBJ_NATIVE:
            printf("<native func>");
            break;
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
        case OBJ_UPVALUE:
            printf("upvalue");
            break;
    }
}