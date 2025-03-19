#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void *reallocate(LoxVM *vm, void *ptr, size_t old_size, size_t new_size)
{
    UNUSED(vm);

    if (new_size == 0)
    {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, new_size);
}

static void free_object(LoxVM *vm, Obj *object)
{
    switch (object->type)
    {
        case OBJ_STRING:
        {
            ObjString *string = (ObjString *)object;
            DEALLOCATE_ARRAY(vm, char, string->data, string->length + 1);
            DEALLOCATE(vm, ObjString, object);
        } break;
    }
}

void free_objects(LoxVM *vm)
{
    Obj *object = vm->objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        free_object(vm, object);
        object = next;
    }
}
