#include "memory.h"
#include "opcode.h"
#include "vm.h"

#ifdef DEBUG
#include "debug.h"
#endif // DEBUG

//------------------------------------------------------------------------------

static void *__reallocate(void *ptr, size_t size)
{
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    return realloc(ptr, size);
}

static void resetStack(LoxVM *vm)
{
    vm->stackTop = vm->stack;
}

LoxVM *newVM()
{
    LoxVM *vm = (LoxVM *)__reallocate(NULL, sizeof(LoxVM));

    vm->fun = NULL;
    vm->ip = NULL;
    resetStack(vm);
    vm->bytes_allocated = 0;
    vm->reallocate = __reallocate;

    return vm;
}

void freeVM(LoxVM *vm)
{
    FREE(LoxVM, vm, vm);
}

InterpretResult interpret(LoxVM *vm, const char *source)
{
    ObjFun fun;
    initFun(&fun);

    uint32_t constant = addConstantFun(vm, &fun, 69);
    writeFun(vm, &fun, CODE_CONSTANT, 1);
    writeFun(vm, &fun, (constant >> 8) & 0xff, 1);
    writeFun(vm, &fun, constant & 0xff, 1);

    writeFun(vm, &fun, CODE_RETURN, 1);

    disassemble(&fun, "test");
    freeFun(vm, &fun);
    return RESULT_SUCCESS;
}
