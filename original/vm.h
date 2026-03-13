#ifndef CLOX_VM_H
#define CLOX_VM_H

#include "chunk.h"
#include "object.h"
#include "table.h"
#include "value.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX * UINT8_COUNT)

typedef struct {
    ObjClosure *closure;
    uint8_t *ip;
    Value *slots;
} CallFrame;

typedef struct {
    CallFrame frames[FRAMES_MAX];
    int frame_count;
    Value stack[STACK_MAX];
    Value *stack_top;
    Table globals;
    Table strings;
    ObjString *init_string;
    ObjUpvalue *open_upvalues;

    size_t bytes_allocated;
    size_t next_GC;
    Obj *objects;
    int gray_capacity;
    int gray_count;
    Obj **gray_stack;
} VM;

typedef enum {
    VM_OK,
    VM_COMPILE_ERROR,
    VM_RUNTIME_ERROR
} VMResult;

extern VM vm;

void init_vm();
void free_vm();
void push(Value value);
Value pop();
VMResult interpret(const char *source);

#endif // CLOX_VM_H