#ifndef LOX_VALUE_H
#define LOX_VALUE_H

#include "array.h"
#include "lox.h"

//------------------------------------------------------------------------------

// Forward Declared Types
typedef struct ObjFun ObjFun;
typedef struct Line Line;

typedef double Value;

DECLARE_ARRAY(uint8_t, Byte);
DECLARE_ARRAY(Line, Line);
DECLARE_ARRAY(Value, Value);

struct ObjFun
{
    ByteArray code;
    LineArray lines;
    ValueArray constants;
};

struct Line
{
    int32_t value;
    uint32_t offset;
};

//------------------------------------------------------------------------------

// Value functions
void printValue(Value value);

// Object functions
void initFun(ObjFun *fun);
void freeFun(LoxVM *vm, ObjFun *fun);
void writeFun(LoxVM *vm, ObjFun *fun, uint8_t byte, int32_t line);
uint32_t addConstantFun(LoxVM *vm, ObjFun *fun, Value value);
int32_t funGetLine(ObjFun *fun, uint32_t offset);

#endif // LOX_VALUE_H
