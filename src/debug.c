#include <stdio.h>

#include "debug.h"
#include "opcode.h"

//------------------------------------------------------------------------------

void disassemble(ObjFun *fun, const char *name)
{
    printf("[ %s ]:\n", name);

    for (uint32_t offset = 0; offset < fun->code.length;)
    {
        offset = disassembleInstruction(fun, offset);
    }

    printf("\n");
}

uint32_t disassembleInstruction(ObjFun *fun, uint32_t offset)
{
    uint8_t *code = fun->code.data;
    OpCode instruction = (OpCode)code[offset];
    int32_t line = funGetLine(fun, offset);

    if (offset > 0 && line == funGetLine(fun, offset - 1))
    {
        printf("     ");
    }
    else
    {
        printf("%4d:", line);
    }

    printf(" %04d ", offset);
    switch (instruction)
    {
        case CODE_CONSTANT:
        {
            offset += 3; // 1 byte for the instruction, 2 bytes for the argument
            uint16_t constant = (code[offset - 2] << 8) | code[offset - 1];
            printf("%-16s %4d '", opcode_names[instruction], constant);
            printValue(fun->constants.data[constant]);
            printf("'\n");
            return offset;
        }

        case CODE_RETURN:
            printf("%s\n", opcode_names[instruction]);
            return offset + 1;

        default:
            printf("Unknown OpCode: %d\n", code[offset]);
            return offset + 1;
    }
}
