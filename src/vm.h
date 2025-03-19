#ifndef lox_vm_h
#define lox_vm_h

#include "chunk.h"
#include "common.h"
#include "table.h"
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
    Table strings;
    Compiler *compiler;
};

void init_lox_vm(LoxVM *vm);
void free_lox_vm(LoxVM *vm);
void push_stack(LoxVM *vm, Value value);
Value pop_stack(LoxVM *vm);
InterpretResult interpret_lox_vm(LoxVM *vm, const char *source);

#endif // lox_vm_h
