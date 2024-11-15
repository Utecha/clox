#include <stdio.h>
#include "include/debug.h"
#include "include/object.h"

static int byte_instruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code[offset + 1];
    printf("%-16s %4d\n", name, slot);
    return offset + 2;
}

static int constant_instruction(const char *name, Chunk *chunk, int offset)
{
    int constant = chunk->code[offset + 1];

    printf("%-16s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 2;
}

static int invoke_instruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t constant = chunk->code[offset + 1];
    uint8_t arg_count = chunk->code[offset + 2];

    printf("%-16s (%d args) %4d '", name, arg_count, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");

    return offset + 3;
}

static int jump_instruction(const char *name, int sign, Chunk *chunk, int offset)
{
    uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
    jump |= chunk->code[offset + 2];
    printf("%-16s %4d -> %d\n", name, offset, offset + 3 + sign * jump);
    return offset + 3;
}

static int simple_instruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

void disassemble_chunk(Chunk *chunk, const char *name)
{
    printf("=== %s ===\n", name);

    for (int offset = 0; offset < chunk->count;) {
        offset = disassemble_instruction(chunk, offset);
    }
}

int disassemble_instruction(Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    int line = chunk->lines[offset];
    if (offset > 0 && line == chunk->lines[offset - 1]) {
        printf("   | ");
    } else {
        printf("%4d ", line);
    }

    uint8_t instruction = chunk->code[offset];
    switch (instruction) {
        case OP_CLASS:
            return constant_instruction("CLASS", chunk, offset);
        case OP_INHERIT:
            return simple_instruction("INHERIT", offset);
        case OP_GET_SUPER:
            return constant_instruction("GET SUPER", chunk, offset);
        case OP_CONSTANT:
            return constant_instruction("CONSTANT", chunk, offset);
        case OP_GET_PROPERTY:
            return constant_instruction("GET PROPERTY", chunk, offset);
        case OP_SET_PROPERTY:
            return constant_instruction("SET PROPERTY", chunk, offset);
        case OP_METHOD:
            return constant_instruction("METHOD", chunk, offset);
        case OP_CLOSURE: {
            offset++;

            uint8_t constant = chunk->code[offset++];
            printf("%-16s %4d ", "CLOSURE", constant);
            print_value(chunk->constants.values[constant]);
            printf("\n");

            ObjFunction *function = AS_FUNCTION(chunk->constants.values[constant]);
            for (int j = 0; j < function->upvalue_count; j++) {
                int is_local = chunk->code[offset++];
                int index = chunk->code[offset++];
                printf("%04d    |                    %s %d\n",
                    offset - 2, is_local ? "local" : "upvalue", index);
            }

            return offset;
        } break;
        case OP_DEFINE_GLOBAL:
            return constant_instruction("DEFINE GLOBAL", chunk, offset);
        case OP_GET_GLOBAL:
            return constant_instruction("GET GLOBAL", chunk, offset);
        case OP_SET_GLOBAL:
            return constant_instruction("SET GLOBAL", chunk, offset);
        case OP_GET_UPVALUE:
            return byte_instruction("GET UPVALUE", chunk, offset);
        case OP_SET_UPVALUE:
            return byte_instruction("SET UPVALUE", chunk, offset);
        case OP_CLOSE_UPVALUE:
            return simple_instruction("CLOSE UPVALUE", offset);
        case OP_GET_LOCAL:
            return byte_instruction("GET LOCAL", chunk, offset);
        case OP_SET_LOCAL:
            return byte_instruction("SET LOCAL", chunk, offset);
        case OP_JUMP_IF_FALSE:
            return jump_instruction("JUMP IF FALSE", 1, chunk, offset);
        case OP_JUMP:
            return jump_instruction("JUMP", 1, chunk, offset);
        case OP_LOOP:
            return jump_instruction("LOOP", -1, chunk, offset);
        case OP_CALL:
            return byte_instruction("CALL", chunk, offset);
        case OP_INVOKE:
            return invoke_instruction("INVOKE", chunk, offset);
        case OP_SUPER_INVOKE:
            return invoke_instruction("SUPER INVOKE", chunk, offset);
        case OP_POP:
            return simple_instruction("POP", offset);
        case OP_PRINT:
            return simple_instruction("PRINT", offset);
        case OP_RETURN:
            return simple_instruction("RETURN", offset);
        case OP_NIL:
            return simple_instruction("NIL", offset);
        case OP_TRUE:
            return simple_instruction("TRUE", offset);
        case OP_NOT:
            return simple_instruction("NOT", offset);
        case OP_EQUAL:
            return simple_instruction("EQUAL", offset);
        case OP_GREATER:
            return simple_instruction("GREATER", offset);
        case OP_LESS:
            return simple_instruction("LESS", offset);
        case OP_FALSE:
            return simple_instruction("FALSE", offset);
        case OP_NEGATE:
            return simple_instruction("NEGATE", offset);
        case OP_ADD:
            return simple_instruction("ADD", offset);
        case OP_SUBTRACT:
            return simple_instruction("SUBTRACT", offset);
        case OP_MULTIPLY:
            return simple_instruction("MULTIPLY", offset);
        case OP_DIVIDE:
            return simple_instruction("DIVIDE", offset);
        default:
            printf("Unknown OpCode: %d\n", instruction);
            return offset + 1;
    }
}