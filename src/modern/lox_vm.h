#ifndef LOX_VM_H
#define LOX_VM_H

#include "lox.h"

//--------------------------------------------------------------------------------------------------

struct LoxVM
{
    ssize_t bytes_allocated;
    LoxReallocateFn reallocate;
};

#endif // LOX_VM_H
