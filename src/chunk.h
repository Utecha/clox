#ifndef lox_chunk_h
#define lox_chunk_h

#include "common.h"
#include "value.h"

typedef enum
{
    // Simple Instructions (1 byte)
    OP_NIL,
    OP_FALSE,
    OP_TRUE,
    OP_NOT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_RETURN,

    // Constant Instructions (2 bytes)
    OP_CONSTANT,
} OpCode;

typedef struct
{
    int count;
    int capacity;
    uint8_t *code;
    int *lines;
    ValueArray constants;
} Chunk;

void init_chunk(Chunk *chunk);
void free_chunk(LoxVM *vm, Chunk *chunk);
void write_chunk(LoxVM *vm, Chunk *chunk, uint8_t byte, int line);
int add_constant(LoxVM *vm, Chunk *chunk, Value value);

#endif // lox_chunk_h
