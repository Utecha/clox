#ifndef lox_compiler_h
#define lox_compiler_h

#include "common.h"
#include "vm.h"

bool compile(LoxVM *vm, Chunk *chunk, const char *source);

#endif // lox_compiler_h
