#ifndef LOX_OPCODE_H
#define LOX_OPCODE_H

//------------------------------------------------------------------------------

typedef enum
{
    CODE_CONSTANT,
    CODE_RETURN,
} OpCode;

//------------------------------------------------------------------------------

static const char *opcode_names[] = {
    [CODE_CONSTANT]     = "Constant",
    [CODE_RETURN]       = "Return",
};

#endif // LOX_OPCODE_H
