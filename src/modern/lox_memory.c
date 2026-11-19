#include "lox_memory.h"
#include "lox_vm.h"

//--------------------------------------------------------------------------------------------------

size_t lox_next_power_of_two(size_t n)
{
    if (n <= 2) return n;

    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n |= n >> 32;
    n++;

    return n;
}

void *lox_reallocate(LoxVM *vm, void *ptr, ssize_t old_size, ssize_t new_size)
{
    vm->bytes_allocated += new_size - old_size;
    return vm->reallocate(ptr, new_size);
}
