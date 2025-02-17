#include <stdio.h>
#include <stdlib.h>
#include "compiler.h"
#include "lexer.h"
#include "vm.h"

#if DUMP_TOKENS || DUMP_CHUNK
    #include "debug.h"
#endif

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_CONDITIONAL,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} Precedence;

typedef void (*ParseFn)(Compiler *compiler, bool canAssign);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct
{
    LoxVM *vm;
    Lexer *lexer;
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
} Parser;

struct Compiler
{
    Parser *parser;
    Chunk *chunk;
};

static void initParser(Parser *parser, Lexer *lexer, LoxVM *vm)
{
    parser->vm = vm;
    parser->lexer = lexer;
    parser->hadError = false;
    parser->panicMode = false;
}

static void errorAt(Parser *parser, Token *token, const char *message)
{
    if (parser->panicMode) return;
    parser->panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);
    switch (token->type)
    {
        case TK_ERROR:  break;
        case TK_EOF:    fprintf(stderr, " at end"); break;
        default:        fprintf(stderr, " at '%.*s'", token->length, token->lexeme); break;
    }

    fprintf(stderr, ": %s\n", message);
    parser->hadError = true;
}

static void error(Compiler *compiler, const char *message)
{
    errorAt(compiler->parser, &compiler->parser->previous, message);
}

static void errorAtCurrent(Compiler *compiler, const char *message)
{
    errorAt(compiler->parser, &compiler->parser->current, message);
}

static void advance(Compiler *compiler)
{
    compiler->parser->previous = compiler->parser->current;

    for (;;)
    {
        compiler->parser->current = getToken(compiler->parser->lexer);
        if (compiler->parser->current.type != TK_ERROR) break;

        errorAtCurrent(compiler, compiler->parser->current.lexeme);
    }
}

static void consume(Compiler *compiler, TokenType type, const char *message)
{
    if (compiler->parser->current.type == type)
    {
        advance(compiler);
        return;
    }

    errorAtCurrent(compiler, message);
}

static Chunk *currentChunk(Compiler *compiler)
{
    return compiler->chunk;
}

static void initCompiler(Compiler *compiler, Parser *parser)
{
    compiler->parser = parser;
}

static void emitByte(Compiler *compiler, uint8_t byte)
{
    writeChunk(currentChunk(compiler), byte, compiler->parser->previous.line);
}

static void emitBytes(Compiler *compiler, uint8_t byte1, uint8_t byte2)
{
    emitByte(compiler, byte1);
    emitByte(compiler, byte2);
}

static uint8_t makeConstant(Compiler *compiler, Value value)
{
    int constant = addConstant(currentChunk(compiler), value);
    if (constant > UINT8_MAX)
    {
        error(compiler, "Too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emitConstant(Compiler *compiler, Value value)
{
    emitBytes(compiler, OP_CONSTANT, makeConstant(compiler, value));
}

static void emitReturn(Compiler *compiler)
{
    emitByte(compiler, OP_RETURN);
}

static void endCompiler(Compiler *compiler)
{
    emitReturn(compiler);

    #if DUMP_CHUNK
        if (!compiler->parser.hadError)
            { disassemble(currentChunk(compiler), "Code"); }
    #endif
}

/* Forward Declarations */
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Compiler *compiler, Precedence precedence);
static void expression(Compiler *compiler);

static void conditional(Compiler *compiler, bool canAssign)
{
    // int ifJump = emitJump(compiler, OP_JUMP_IF);
    parsePrecedence(compiler, PREC_CONDITIONAL);
    consume(compiler, TK_COLON, "Expected ':' after conditional 'then' branch");
    // int elseJump = emitJump(compiler, JUMP);
    // patchJump(compiler, ifJump);
    parsePrecedence(compiler, PREC_ASSIGNMENT);
    // patchJump(compiler, elseJump);
}

static void binary(Compiler *compiler, bool canAssign)
{
    TokenType operator = compiler->parser->previous.type;
    ParseRule *rule = getRule(operator);
    parsePrecedence(compiler, (Precedence)(rule->precedence + 1));

    switch (operator)
    {
        case TK_BANGEQ: emitBytes(compiler, OP_EQUAL, OP_NOT); break;
        case TK_EQEQ:   emitByte(compiler, OP_EQUAL); break;
        case TK_GT:     emitByte(compiler, OP_GREATER); break;
        case TK_GTEQ:   emitBytes(compiler, OP_LESS, OP_NOT); break;
        case TK_LT:     emitByte(compiler, OP_LESS); break;
        case TK_LTEQ:   emitBytes(compiler, OP_GREATER, OP_NOT); break;
        case TK_MINUS:  emitByte(compiler, OP_SUBTRACT); break;
        case TK_PLUS:   emitByte(compiler, OP_ADD); break;
        case TK_SLASH:  emitByte(compiler, OP_DIVIDE); break;
        case TK_STAR:   emitByte(compiler, OP_MULTIPLY); break;
        default:        return; // Unreachable
    }
}

static void unary(Compiler *compiler, bool canAssign)
{
    TokenType operator = compiler->parser->previous.type;
    parsePrecedence(compiler, PREC_UNARY);

    switch (operator)
    {
        case TK_BANG:   emitByte(compiler, OP_NOT); break;
        case TK_MINUS:  emitByte(compiler, OP_NEGATE); break;
        default:        return; // Unreachable
    }
}

static void grouping(Compiler *compiler, bool canAssign)
{
    expression(compiler);
    consume(compiler, TK_RPAREN, "Expected ')' after expression");
}

static void number(Compiler *compiler, bool canAssign)
{
    double value = strtod(compiler->parser->previous.lexeme, NULL);
    emitConstant(compiler, NUMBER_VAL(value));
}

static void string(Compiler *compiler, bool canAssign)
{
    emitConstant(
        compiler,
        OBJ_VAL(
            newStringLength(
                compiler->parser->vm,
                compiler->parser->previous.lexeme + 1,
                compiler->parser->previous.length - 2
            )
        )
    );
}

static void literal(Compiler *compiler, bool canAssign)
{
    switch (compiler->parser->previous.type)
    {
        case TK_NIL:    emitByte(compiler, OP_NIL); break;
        case TK_FALSE:  emitByte(compiler, OP_FALSE); break;
        case TK_TRUE:   emitByte(compiler, OP_TRUE); break;
        default:        return; // Unreachable
    }
}

#define UNUSED                      { NULL, NULL, PREC_NONE }
#define PREFIX(fn)                  { fn, NULL, PREC_NONE }
#define INFIX(fn, prec)             { NULL, fn, prec }
#define MIXFIX(preFn, inFn, prec)   { preFn, inFn, prec }

ParseRule rules[] = {
    [TK_ERROR]          = UNUSED,
    [TK_EOF]            = UNUSED,
    [TK_LPAREN]         = MIXFIX(grouping, NULL, PREC_CALL),
    [TK_RPAREN]         = UNUSED,
    [TK_LBRACE]         = UNUSED,
    [TK_RBRACE]         = UNUSED,
    [TK_COMMA]          = UNUSED,
    [TK_DOT]            = UNUSED,
    [TK_SEMICOLON]      = UNUSED,
    [TK_EQUAL]          = UNUSED,
    [TK_COLON]          = UNUSED,
    [TK_QUESTION]       = INFIX(conditional, PREC_CONDITIONAL),
    [TK_BANGEQ]         = INFIX(binary, PREC_EQUALITY),
    [TK_EQEQ]           = INFIX(binary, PREC_EQUALITY),
    [TK_GT]             = INFIX(binary, PREC_COMPARISON),
    [TK_GTEQ]           = INFIX(binary, PREC_COMPARISON),
    [TK_LT]             = INFIX(binary, PREC_COMPARISON),
    [TK_LTEQ]           = INFIX(binary, PREC_COMPARISON),
    [TK_MINUS]          = MIXFIX(unary, binary, PREC_TERM),
    [TK_PLUS]           = INFIX(binary, PREC_TERM),
    [TK_SLASH]          = INFIX(binary, PREC_FACTOR),
    [TK_STAR]           = INFIX(binary, PREC_FACTOR),
    [TK_BANG]           = PREFIX(unary),
    [TK_IDENTIFIER]     = UNUSED,
    [TK_NUMBER]         = PREFIX(number),
    [TK_STRING]         = PREFIX(string),
    [TK_AND]            = UNUSED,
    [TK_CLASS]          = UNUSED,
    [TK_ELSE]           = UNUSED,
    [TK_FALSE]          = PREFIX(literal),
    [TK_FOR]            = UNUSED,
    [TK_FUN]            = UNUSED,
    [TK_IF]             = UNUSED,
    [TK_NIL]            = PREFIX(literal),
    [TK_OR]             = UNUSED,
    [TK_PRINT]          = UNUSED,
    [TK_RETURN]         = UNUSED,
    [TK_SUPER]          = UNUSED,
    [TK_THIS]           = UNUSED,
    [TK_TRUE]           = PREFIX(literal),
    [TK_VAR]            = UNUSED,
    [TK_WHILE]          = UNUSED,
};

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

static void parsePrecedence(Compiler *compiler, Precedence precedence)
{
    advance(compiler);
    ParseFn prefix = getRule(compiler->parser->previous.type)->prefix;
    if (prefix == NULL)
    {
        error(compiler, "Expected expression");
        return;
    }

    bool canAssign = precedence <= PREC_CONDITIONAL;
    prefix(compiler, canAssign);

    while (precedence <= getRule(compiler->parser->current.type)->precedence)
    {
        advance(compiler);
        ParseFn infix = getRule(compiler->parser->previous.type)->infix;
        infix(compiler, canAssign);
    }
}

static void expression(Compiler *compiler)
{
    parsePrecedence(compiler, PREC_ASSIGNMENT);
}

bool compile(LoxVM *vm, Chunk *chunk, const char *source)
{
    #if DUMP_TOKENS
        dumpTokens(source);
    #endif

    Lexer lexer;
    initLexer(&lexer, source);

    Parser parser;
    initParser(&parser, &lexer, vm);

    Compiler compiler;
    initCompiler(&compiler, &parser);
    compiler.chunk = chunk;

    advance(&compiler);
    expression(&compiler);
    consume(&compiler, TK_EOF, "Expected end of expression");
    endCompiler(&compiler);

    return !compiler.parser->hadError;
}
