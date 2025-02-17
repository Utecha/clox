#ifndef lox_debug_h
#define lox_debug_h

#include "chunk.h"

void disassemble(Chunk *chunk, const char *name);
int disassembleInstruction(Chunk *chunk, int offset);
void dumpTokens(const char *source);

#endif // lox_debug_h
