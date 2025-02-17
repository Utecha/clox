#include "chunk.h"

void initChunk(Chunk *chunk)
{
    initByteArray(&chunk->code);
    initLineArray(&chunk->lines);
    initValueArray(&chunk->constants);
}

void freeChunk(Chunk *chunk)
{
    freeByteArray(&chunk->code);
    freeLineArray(&chunk->lines);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

void writeChunk(Chunk *chunk, uint8_t byte, int line)
{
    if (chunk->code.count == 0 || chunk->lines.data[chunk->lines.count - 1].number != line)
    {
        writeLineArray(&chunk->lines, (Line){ .number = line, .offset = chunk->code.count });
    }

    writeByteArray(&chunk->code, byte);
}

int addConstant(Chunk *chunk, Value value)
{
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

int getLine(Chunk *chunk, int offset)
{
    int start = 0;
    int end = chunk->lines.count;

    while (start < end)
    {
        int mid = (start + end) / 2;
        Line line = chunk->lines.data[mid];

        if (offset < line.offset)
        {
            end = mid;
        }
        else if (mid == chunk->lines.count - 1 || offset < chunk->lines.data[mid + 1].offset)
        {
            return line.number;
        }
        else
        {
            start = mid + 1;
        }
    }

    return -1;
}
