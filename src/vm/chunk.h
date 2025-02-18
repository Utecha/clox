#ifndef lox_chunk_h
#define lox_chunk_h

#include "utils.h"
#include "value.h"

typedef enum
{
    // Simple Instructions (1 byte)
    OP_NIL,
    OP_FALSE,
    OP_TRUE,
    OP_POP,
    OP_NEGATE,
    OP_NOT,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,
    OP_PRINT,

    // Constant Instructions (2 bytes)
    OP_CONSTANT,
    OP_DEFINE_GLOBAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,

    // Byte Instructions (2 bytes)
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_CALL,

    // Jump Instructions (3 bytes)
    OP_JUMP,
    OP_JUMP_IF,
    OP_LOOP,
    OP_AND,
    OP_OR,
    OP_END,

    // Count
    OP_COUNT,
} OpCode;

#endif // lox_chunk_h
