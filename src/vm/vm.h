#ifndef lox_vm_h
#define lox_vm_h

#include "chunk.h"
#include "common.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * (1 << 8))

typedef enum
{
    RESULT_OK,
    RESULT_COMPILE_ERROR,
    RESULT_RUNTIME_ERROR,
} InterpretResult;

struct LoxVM
{
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    Value stack[STACK_MAX];
    Value *stackTop;
    Obj *objects;
    Table strings;
    Table globals;
    Compiler *compiler;
};

void initVM(LoxVM *vm);
void freeVM(LoxVM *vm);

void pushVM(LoxVM *vm, Value value);
Value popVM(LoxVM *vm);
Value peekVM(LoxVM *vm, int distance);

InterpretResult interpret(LoxVM *vm, char *source);

#endif // lox_vm_h
