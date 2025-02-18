#include <stdio.h>
#include "debug.h"
#include "chunk.h"
#include "lexer.h"

const char *opcodes[OP_COUNT] = {
    // Simple Instructions (1 byte)
    [OP_NIL]            = "NIL",
    [OP_FALSE]          = "FALSE",
    [OP_TRUE]           = "TRUE",
    [OP_POP]            = "POP",
    [OP_NEGATE]         = "NEGATE",
    [OP_NOT]            = "NOT",
    [OP_ADD]            = "ADD",
    [OP_SUBTRACT]       = "SUBTRACT",
    [OP_MULTIPLY]       = "MULTIPLY",
    [OP_DIVIDE]         = "DIVIDE",
    [OP_EQUAL]          = "EQUAL",
    [OP_GREATER]        = "GREATER",
    [OP_LESS]           = "LESS",
    [OP_RETURN]         = "RETURN",
    [OP_PRINT]          = "PRINT",

    // Constant Instructions (2 bytes)
    [OP_CONSTANT]       = "CONSTANT",
    [OP_DEFINE_GLOBAL]  = "DEFINE GLOBAL",
    [OP_GET_GLOBAL]     = "GET GLOBAL",
    [OP_SET_GLOBAL]     = "SET GLOBAL",

    // Byte Instructions (2 bytes)
    [OP_GET_LOCAL]      = "GET LOCAL",
    [OP_SET_LOCAL]      = "SET LOCAL",
};

const char *tokenTypes[TK_COUNT] = {
    [TK_ERROR]      = "ERROR",
    [TK_EOF]        = "EOF",
    [TK_LPAREN]     = "LPAREN",
    [TK_RPAREN]     = "RPAREN",
    [TK_LBRACE]     = "LBRACE",
    [TK_RBRACE]     = "RBRACE",
    [TK_COMMA]      = "COMMA",
    [TK_DOT]        = "DOT",
    [TK_SEMICOLON]  = "SEMICOLON",
    [TK_EQUAL]      = "EQUAL",
    [TK_COLON]      = "COLON",
    [TK_QUESTION]   = "QUESTION",
    [TK_BANGEQ]     = "BANGEQ",
    [TK_EQEQ]       = "EQEQ",
    [TK_GT]         = "GT",
    [TK_GTEQ]       = "GTEQ",
    [TK_LT]         = "LT",
    [TK_LTEQ]       = "LTEQ",
    [TK_MINUS]      = "MINUS",
    [TK_PLUS]       = "PLUS",
    [TK_SLASH]      = "SLASH",
    [TK_STAR]       = "STAR",
    [TK_BANG]       = "BANG",
    [TK_IDENTIFIER] = "IDENTIFIER",
    [TK_NUMBER]     = "NUMBER",
    [TK_STRING]     = "STRING",
    [TK_AND]        = "AND",
    [TK_CLASS]      = "CLASS",
    [TK_CONST]      = "CONST",
    [TK_ELSE]       = "ELSE",
    [TK_FALSE]      = "FALSE",
    [TK_FOR]        = "FOR",
    [TK_FUN]        = "FUN",
    [TK_IF]         = "IF",
    [TK_NIL]        = "NIL",
    [TK_OR]         = "OR",
    [TK_PRINT]      = "PRINT",
    [TK_RETURN]     = "RETURN",
    [TK_SUPER]      = "SUPER",
    [TK_THIS]       = "THIS",
    [TK_TRUE]       = "TRUE",
    [TK_VAR]        = "VAR",
    [TK_WHILE]      = "WHILE"
};

static int constantInstruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t constant = chunk->code.data[offset + 1];
    printf("%-14s %4d '", name, constant);
    printValue(chunk->constants.data[constant]);
    printf("'\n");
    return offset + 2;
}

static int byteInstruction(const char *name, Chunk *chunk, int offset)
{
    uint8_t slot = chunk->code.data[offset + 1];
    printf("%-14s %4d\n", name, slot);
    return offset + 2;
}

static int simpleInstruction(const char *name, int offset)
{
    puts(name);
    return offset + 1;
}

void disassemble(Chunk *chunk, const char *name)
{
    printf("========= %s =========\n", name);

    for (int offset = 0; offset < chunk->code.count;)
    {
        offset = disassembleInstruction(chunk, offset);
    }
}

int disassembleInstruction(Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    int line = getLine(chunk, offset);
    if (offset > 0 && line == getLine(chunk, offset + 1))
        { printf("   | "); }
    else
        { printf("%4d ", line); }

    uint8_t instruction = chunk->code.data[offset];
    const char *name = opcodes[instruction];
    switch (instruction)
    {
        case OP_NIL:
        case OP_FALSE:
        case OP_TRUE:
        case OP_POP:
        case OP_NEGATE:
        case OP_NOT:
        case OP_ADD:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_EQUAL:
        case OP_GREATER:
        case OP_LESS:
        case OP_RETURN:
        case OP_PRINT:
            return simpleInstruction(name, offset);
        case OP_CONSTANT:
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
            return constantInstruction(name, chunk, offset);
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
            return byteInstruction(name, chunk, offset);
        default:
            printf("Unknown OpCode: %d", instruction);
            return offset + 1;
    }
}

void dumpTokens(const char *source)
{
    Lexer lexer;
    initLexer(&lexer, source);

    for (;;)
    {
        Token token = getToken(&lexer);

        printf("{ %s : %d }\n", tokenTypes[token.type], token.type);
        if (token.type == TK_EOF) break;

        printf("Lexeme: %.*s\n", token.length, token.lexeme);
        printf("Length: %d\n", token.length);
        printf("Line: %d\n", token.line);
    }
}
