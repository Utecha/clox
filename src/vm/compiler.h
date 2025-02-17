#ifndef lox_compiler_h
#define lox_compiler_h

#include "common.h"
#include "vm.h"

typedef struct Compiler Compiler;

bool compile(LoxVM *vm, Chunk *chunk, const char *source);

#endif // lox_compiler_h
