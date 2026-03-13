#include <stdio.h>

#include "memory.h"
#include "value.h"

//------------------------------------------------------------------------------

DEFINE_ARRAY(uint8_t, Byte);
DEFINE_ARRAY(Line, Line);
DEFINE_ARRAY(Value, Value);

void printValue(Value value)
{
    printf("%.16g", value);
}

void initFun(ObjFun *fun)
{
    initByteArray(&fun->code);
    initLineArray(&fun->lines);
    initValueArray(&fun->constants);
}

void freeFun(LoxVM *vm, ObjFun *fun)
{
    clearValueArray(vm, &fun->constants);
    clearLineArray(vm, &fun->lines);
    clearByteArray(vm, &fun->code);
}

void writeFun(LoxVM *vm, ObjFun *fun, uint8_t byte, int32_t line)
{
    if (fun->code.length == 0 || fun->lines.data[fun->lines.length - 1].value != line)
    {
        writeLineArray(
            vm,
            &fun->lines,
            (Line) { .value = line, .offset = fun->code.length }
        );
    }

    writeByteArray(vm, &fun->code, byte);
}

uint32_t addConstantFun(LoxVM *vm, ObjFun *fun, Value value)
{
    writeValueArray(vm, &fun->constants, value);
    return fun->constants.length - 1;
}

int32_t funGetLine(ObjFun *fun, uint32_t offset)
{
    uint32_t start = 0;
    uint32_t end = fun->lines.length - 1;

    while (start <= end)
    {
        uint32_t mid = (start + end) / 2;
        Line line = fun->lines.data[mid];

        if (offset < line.offset)
        {
            end = mid - 1;
        }
        else if (mid == fun->lines.length - 1 ||
            offset < fun->lines.data[mid + 1].offset)
        {
            return line.value;
        }
        else
        {
            start = mid + 1;
        }
    }

    return -1;
}
