#include <stdio.h>

#include "lox_memory.h"
#include "lox_vm.h"

//------------------------------------------------------------------------------

int main(void)
{
    LoxVM vm = { 0 };
    ssize_t capacity = 2;
    int *ints = LOX_ALLOCA(int, &vm, capacity);

    ints[0] = 34;
    ints[1] = 35;
    printf("%d + %d = %d\n", ints[0], ints[1], ints[0] + ints[1]);

    LOX_FREEA(int, &vm, ints, capacity);
    return 0;
}
