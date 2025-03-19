#ifndef lox_debug_h
#define lox_debug_h

#include "chunk.h"

void dump_tokens(const char *source);
void disassemble_chunk(Chunk *chunk, const char *name);
int disassemble_instruction(Chunk *chunk, int offset);

#endif // lox_debug_h
