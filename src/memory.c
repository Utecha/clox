#include "memory.h"
#include "vm.h"

#include <stdio.h>

//------------------------------------------------------------------------------

uint32_t nextPowerOfTwo(uint32_t n)
{
    if (n == 0) return 2;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

void *reallocate(LoxVM *vm, void *ptr, size_t old_size, size_t new_size)
{
    vm->bytes_allocated += new_size - old_size;
    return vm->reallocate(ptr, new_size);
}
