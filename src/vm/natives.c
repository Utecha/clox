#include <stdio.h>
#include <string.h>
#include <time.h>

#include "natives.h"
#include "vm.h"

static void defineNative(LoxVM *vm, const char *name, NativeFn function, int arity)
{
    pushVM(vm, OBJ_VAL(newStringLength(vm, name, strlen(name))));
    pushVM(vm, OBJ_VAL(newNative(vm, function, arity)));

    tableSet(&vm->globals, AS_STRING(vm->stack[0]), vm->stack[1]);

    popVM(vm);
    popVM(vm);
}

static Value clockNative(int argc, Value *args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

void defineNatives(LoxVM *vm)
{
    defineNative(vm, "clock", clockNative, 0);
}
