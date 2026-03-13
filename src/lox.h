#ifndef LOX_H
#define LOX_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

//------------------------------------------------------------------------------

typedef struct LoxVM LoxVM;

typedef enum
{
    RESULT_SUCCESS,
    RESULT_COMPILE_ERROR,
    RESULT_RUNTIME_ERROR
} InterpretResult;

//------------------------------------------------------------------------------

LoxVM *newVM();
void freeVM(LoxVM *vm);
InterpretResult interpret(LoxVM *vm, const char *source);

#endif // LOX_H
