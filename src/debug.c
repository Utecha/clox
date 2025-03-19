#include <stdio.h>

#include "debug.h"
#include "scanner.h"
#include "value.h"

static const char *opcodes[] = {
    [OP_NIL]            = "NIL",
    [OP_FALSE]          = "FALSE",
    [OP_TRUE]           = "TRUE",
    [OP_NOT]            = "NOT",
    [OP_NEGATE]         = "NEGATE",
    [OP_ADD]            = "ADD",
    [OP_SUBTRACT]       = "SUBTRACT",
    [OP_MULTIPLY]       = "MULTIPLY",
    [OP_DIVIDE]         = "DIVIDE",
    [OP_EQUAL]          = "EQUAL",
    [OP_GREATER]        = "GREATER",
    [OP_LESS]           = "LESS",
    [OP_RETURN]         = "RETURN",
    [OP_CONSTANT]       = "CONSTANT",
};

static const char *tokenTypes[] = {
    [TK_ERROR]          = "ERROR",
    [TK_EOF]            = "EOF",
    [TK_LPAREN]         = "LPAREN",
    [TK_RPAREN]         = "RPAREN",
    [TK_LBRACE]         = "LBRACE",
    [TK_RBRACE]         = "RBRACE",
    [TK_COMMA]          = "COMMA",
    [TK_DOT]            = "DOT",
    [TK_SEMICOLON]      = "SEMICOLON",
    [TK_EQUAL]          = "EQUAL",
    [TK_NOT]            = "NOT",
    [TK_NOTEQ]          = "NOTEQ",
    [TK_EQEQ]           = "EQEQ",
    [TK_GT]             = "GT",
    [TK_GTEQ]           = "GTEQ",
    [TK_LT]             = "LT",
    [TK_LTEQ]           = "LTEQ",
    [TK_MINUS]          = "MINUS",
    [TK_PLUS]           = "PLUS",
    [TK_SLASH]          = "SLASH",
    [TK_STAR]           = "STAR",
    [TK_IDENTIFIER]     = "IDENTIFIER",
    [TK_NUMBER]         = "NUMBER",
    [TK_STRING]         = "STRING",
    [TK_AND]            = "AND",
    [TK_CLASS]          = "CLASS",
    [TK_ELSE]           = "ELSE",
    [TK_FALSE]          = "FALSE",
    [TK_FOR]            = "FOR",
    [TK_FUN]            = "FUN",
    [TK_IF]             = "IF",
    [TK_NIL]            = "NIL",
    [TK_OR]             = "OR",
    [TK_PRINT]          = "PRINT",
    [TK_RETURN]         = "RETURN",
    [TK_SUPER]          = "SUPER",
    [TK_THIS]           = "THIS",
    [TK_TRUE]           = "TRUE",
    [TK_VAR]            = "VAR",
    [TK_WHILE]          = "WHILE",
};

static int constant_instruction(const char *name, Chunk *chunk, int offset)
{
    int constant = chunk->code[offset + 1];
    printf("%-14s %4d '", name, constant);
    print_value(chunk->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

static int simple_instruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

void dump_tokens(const char *source)
{
    Scanner scanner;
    init_scanner(&scanner, source);

    for (;;)
    {
        Token token = scan_token(&scanner);

        printf("{ Type: %s | ID: %d }\n", tokenTypes[token.type], token.type);
        if (token.type == TK_EOF)
        {
            printf("\n");
            break;
        }

        printf("Lexeme: '%.*s'\n", token.length, token.start);
        printf("Length: %d\n", token.length);
        printf("Line: %d\n\n", token.line);
    }
}

void disassemble_chunk(Chunk *chunk, const char *name)
{
    printf("========= %s =========\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = disassemble_instruction(chunk, offset);
    }
}

int disassemble_instruction(Chunk *chunk, int offset)
{
    printf("%04d ", offset);

    int line = chunk->lines[offset];
    if (offset > 0 && line == chunk->lines[offset - 1])
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", line);
    }

    uint8_t instruction = chunk->code[offset];
    const char *instruction_name = opcodes[instruction];

    switch (instruction)
    {
        case OP_NIL:
        case OP_FALSE:
        case OP_TRUE:
        case OP_NOT:
        case OP_NEGATE:
        case OP_ADD:
        case OP_SUBTRACT:
        case OP_MULTIPLY:
        case OP_DIVIDE:
        case OP_EQUAL:
        case OP_GREATER:
        case OP_LESS:
        case OP_RETURN:
            return simple_instruction(instruction_name, offset);
        case OP_CONSTANT:
            return constant_instruction(instruction_name, chunk, offset);
        default:
            printf("Unknown OpCode: %d", instruction);
            return offset + 1;
    }
}
