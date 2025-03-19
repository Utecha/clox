#include "chunk.h"
#include "memory.h"
#include "vm.h"

void init_chunk(Chunk *chunk)
{
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    init_value_array(&chunk->constants);
}

void free_chunk(LoxVM *vm, Chunk *chunk)
{
    free_value_array(vm, &chunk->constants);
    DEALLOCATE_ARRAY(vm, int, chunk->lines, chunk->capacity);
    DEALLOCATE_ARRAY(vm, uint8_t, chunk->code, chunk->capacity);
}

void write_chunk(LoxVM *vm, Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->capacity < chunk->count + 1)
    {
        int capacity = GROW_CAPACITY(chunk->capacity);
        chunk->code = REALLOCATE(vm, uint8_t, chunk->code, chunk->capacity, capacity);
        chunk->lines = REALLOCATE(vm, int, chunk->lines, chunk->capacity, capacity);
    }

    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int add_constant(LoxVM *vm, Chunk *chunk, Value value)
{
    write_value_array(vm, &chunk->constants, value);
    return chunk->constants.count - 1;
}
