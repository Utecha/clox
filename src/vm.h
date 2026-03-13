#ifndef LOX_VM_H
#define LOX_VM_H

#include "value.h"

//------------------------------------------------------------------------------

#define STACK_MAX (1 << 16)

//------------------------------------------------------------------------------

typedef void *(*ReallocateFn)(void *ptr, size_t size);

struct LoxVM
{
    ObjFun *fun;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
    size_t bytes_allocated;
    ReallocateFn reallocate;
};

#endif // LOX_VM_H
