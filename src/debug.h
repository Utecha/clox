#ifndef LOX_DEBUG_H
#define LOX_DEBUG_H

#include "value.h"

//------------------------------------------------------------------------------

void disassemble(ObjFun *fun, const char *name);
uint32_t disassembleInstruction(ObjFun *fun, uint32_t offset);

#endif // LOX_DEBUG_H
