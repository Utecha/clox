#ifndef lox_compiler_h
#define lox_compiler_h

#include "common.h"
#include "vm.h"

ObjFn *compile(LoxVM *vm, const char *source);

#endif // lox_compiler_h
