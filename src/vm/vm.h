#ifndef lox_vm_h
#define lox_vm_h

#include "chunk.h"
#include "common.h"
#include "value.h"

#define STACK_MAX 256

typedef enum
{
    RESULT_OK,
    RESULT_COMPILE_ERROR,
    RESULT_RUNTIME_ERROR,
} InterpretResult;

struct LoxVM
{
    Chunk *chunk;
    uint8_t *ip;
    Value stack[STACK_MAX];
    Value *stackTop;
    Obj *objects;
    Compiler *compiler;
};

void initVM(LoxVM *vm);
void freeVM(LoxVM *vm);

void pushVM(LoxVM *vm, Value value);
Value popVM(LoxVM *vm);
Value peekVM(LoxVM *vm, int distance);

InterpretResult interpret(LoxVM *vm, char *source);

#endif // lox_vm_h
