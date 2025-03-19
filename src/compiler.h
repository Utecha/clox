#ifndef lox_compiler_h
#define lox_compiler_h

#include "chunk.h"
#include "vm.h"

bool compile(LoxVM *vm, const char *source, Chunk *chunk);

#endif // lox_compiler_h
