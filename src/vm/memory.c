#include <stdlib.h>
#include "memory.h"
#include "value.h"
#include "vm.h"

void *reallocate(void *ptr, size_t oldSize, size_t newSize)
{
    if (newSize == 0)
    {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, newSize);
}

static void freeObject(LoxVM *vm, Obj *object)
{
    switch (object->type)
    {
        case OBJ_STRING:
            FREE(ObjString, object, 1);
            break;
    }
}

void freeObjects(LoxVM *vm)
{
    Obj *object = vm->objects;
    while (object != NULL)
    {
        Obj *next = object->next;
        freeObject(vm, object);
        object = next;
    }
}
