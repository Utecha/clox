#include <stdio.h>
#include <stdlib.h>

#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG
#include "debug.h"
#endif // DEBUG

//------------------------------------------------------------------------------
// Enums, Structs and Types
//------------------------------------------------------------------------------

typedef enum
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
} Precedence;

typedef struct
{
    LoxVM *vm;
    Scanner *scanner;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef void (*ParseFn)(Compiler *compiler, bool can_assign);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

struct Compiler
{
    Parser *parser;
    Chunk *chunk;
};

//------------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------------

static void error_at(Parser *parser, Token *token, const char *msg)
{
    if (parser->panic_mode) return;
    parser->panic_mode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == TK_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->type == TK_ERROR)
    {
        // Do nothing
    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", msg);
    parser->had_error = true;
}

static void error(Parser *parser, const char *msg)
{
    error_at(parser, &parser->previous, msg);
}

static void error_at_current(Parser *parser, const char *msg)
{
    error_at(parser, &parser->current, msg);
}

static void advance(Parser *parser)
{
    parser->previous = parser->current;

    for (;;)
    {
        parser->current = scan_token(parser->scanner);
        if (parser->current.type != TK_ERROR) break;

        error_at_current(parser, parser->current.start);
    }
}

static void consume(Parser *parser, TokenType type, const char *msg)
{
    if (parser->current.type == type)
    {
        advance(parser);
        return;
    }

    error_at_current(parser, msg);
}

//------------------------------------------------------------------------------
// Compiler | Emitter Functions
//------------------------------------------------------------------------------

static Chunk *current_chunk(Compiler *compiler)
{
    return compiler->chunk;
}

static void emit_byte(Compiler *compiler, uint8_t byte)
{
    write_chunk(compiler->parser->vm, current_chunk(compiler), byte, compiler->parser->previous.line);
}

static void emit_bytes(Compiler *compiler, uint8_t byte1, uint8_t byte2)
{
    emit_byte(compiler, byte1);
    emit_byte(compiler, byte2);
}

static void emit_return(Compiler *compiler)
{
    emit_byte(compiler, OP_RETURN);
}

static uint8_t make_constant(Compiler *compiler, Value value)
{
    int constant = add_constant(compiler->parser->vm, current_chunk(compiler), value);
    if (constant > UINT8_MAX)
    {
        error(compiler->parser, "Too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Compiler *compiler, Value value)
{
    emit_bytes(compiler, OP_CONSTANT, make_constant(compiler, value));
}

static void end_compiler(Compiler *compiler)
{
    emit_return(compiler);

    #if DEBUG_DUMP_CODE
        if (!compiler->parser->had_error)
        {
            disassemble_chunk(current_chunk(compiler), "Code");
        }
    #endif
}

//------------------------------------------------------------------------------
// Compiler | Forward Declarations
//------------------------------------------------------------------------------

static void parse_precedence(Compiler *compiler, Precedence precedence);
static void expression(Compiler *compiler);
static ParseRule *get_rule(TokenType type);

//------------------------------------------------------------------------------
// Compiler | Expressions
//------------------------------------------------------------------------------

static void binary(Compiler *compiler, bool can_assign)
{
    TokenType op_type = compiler->parser->previous.type;
    ParseRule *rule = get_rule(op_type);
    parse_precedence(compiler, (Precedence)(rule->precedence + 1));

    switch (op_type)
    {
        case TK_NOTEQ:  emit_bytes(compiler, OP_EQUAL, OP_NOT); break;
        case TK_EQEQ:   emit_byte(compiler, OP_EQUAL); break;
        case TK_GT:     emit_byte(compiler, OP_GREATER); break;
        case TK_GTEQ:   emit_bytes(compiler, OP_LESS, OP_NOT); break;
        case TK_LT:     emit_byte(compiler, OP_LESS); break;
        case TK_LTEQ:   emit_bytes(compiler, OP_GREATER, OP_NOT); break;
        case TK_MINUS:  emit_byte(compiler, OP_SUBTRACT); break;
        case TK_PLUS:   emit_byte(compiler, OP_ADD); break;
        case TK_SLASH:  emit_byte(compiler, OP_DIVIDE); break;
        case TK_STAR:   emit_byte(compiler, OP_MULTIPLY); break;
        default:        UNREACHABLE();
    }
}

static void literal(Compiler *compiler, bool can_assign)
{
    switch (compiler->parser->previous.type)
    {
        case TK_FALSE:  emit_byte(compiler, OP_FALSE); break;
        case TK_NIL:    emit_byte(compiler, OP_NIL); break;
        case TK_TRUE:   emit_byte(compiler, OP_TRUE); break;
        default:        UNREACHABLE();
    }
}

static void grouping(Compiler *compiler, bool can_assign)
{
    expression(compiler);
    consume(compiler->parser, TK_RPAREN, "Expected ')' after expression");
}

static void number(Compiler *compiler, bool can_assign)
{
    double value = strtod(compiler->parser->previous.start, NULL);
    emit_constant(compiler, NUMBER_VAL(value));
}

static void string(Compiler *compiler, bool can_assign)
{
    const char *value = compiler->parser->previous.start + 1;
    int length = compiler->parser->previous.length - 2;
    emit_constant(compiler, OBJ_VAL(copy_string(compiler->parser->vm, value, length)));
}

static void unary(Compiler *compiler, bool can_assign)
{
    TokenType op_type = compiler->parser->previous.type;
    parse_precedence(compiler, PREC_UNARY);

    switch (op_type)
    {
        case TK_NOT:    emit_byte(compiler, OP_NOT); break;
        case TK_MINUS:  emit_byte(compiler, OP_NEGATE); break;
        default:        UNREACHABLE();
    }
}

static void parse_precedence(Compiler *compiler, Precedence precedence)
{
    advance(compiler->parser);
    ParseFn prefix_rule = get_rule(compiler->parser->previous.type)->prefix;
    if (!prefix_rule)
    {
        error(compiler->parser, "Expected expression");
        return;
    }

    bool can_assign = precedence <= PREC_ASSIGNMENT;
    prefix_rule(compiler, can_assign);

    while (precedence <= get_rule(compiler->parser->current.type)->precedence)
    {
        advance(compiler->parser);
        ParseFn infix_rule = get_rule(compiler->parser->previous.type)->infix;
        infix_rule(compiler, can_assign);
    }

    // if (match(compiler->parser, TK_EQUAL) && can_assign)
    // {
    //     error(compiler->parser, "Invalid assignment target");
    // }
}

static void expression(Compiler *compiler)
{
    parse_precedence(compiler, PREC_ASSIGNMENT);
}

//------------------------------------------------------------------------------
// Compiler | Parse Rules
//------------------------------------------------------------------------------

static ParseRule rules[] = {
    [TK_ERROR]          = { NULL,       NULL,        PREC_NONE },
    [TK_EOF]            = { NULL,       NULL,        PREC_NONE },
    [TK_LPAREN]         = { grouping,   NULL,        PREC_CALL },
    [TK_RPAREN]         = { NULL,       NULL,        PREC_NONE },
    [TK_LBRACE]         = { NULL,       NULL,        PREC_NONE },
    [TK_RBRACE]         = { NULL,       NULL,        PREC_NONE },
    [TK_COMMA]          = { NULL,       NULL,        PREC_NONE },
    [TK_DOT]            = { NULL,       NULL,        PREC_NONE },
    [TK_SEMICOLON]      = { NULL,       NULL,        PREC_NONE },
    [TK_EQUAL]          = { NULL,       NULL,        PREC_NONE },
    [TK_NOT]            = { unary,      NULL,        PREC_NONE },
    [TK_NOTEQ]          = { NULL,       binary,      PREC_EQUALITY },
    [TK_EQEQ]           = { NULL,       binary,      PREC_EQUALITY },
    [TK_GT]             = { NULL,       binary,      PREC_COMPARISON },
    [TK_GTEQ]           = { NULL,       binary,      PREC_COMPARISON },
    [TK_LT]             = { NULL,       binary,      PREC_COMPARISON },
    [TK_LTEQ]           = { NULL,       binary,      PREC_COMPARISON },
    [TK_MINUS]          = { unary,      binary,      PREC_TERM },
    [TK_PLUS]           = { NULL,       binary,      PREC_TERM },
    [TK_SLASH]          = { NULL,       binary,      PREC_FACTOR },
    [TK_STAR]           = { NULL,       binary,      PREC_FACTOR },
    [TK_IDENTIFIER]     = { NULL,       NULL,        PREC_NONE },
    [TK_NUMBER]         = { number,     NULL,        PREC_NONE },
    [TK_STRING]         = { string,     NULL,        PREC_NONE },
    [TK_AND]            = { NULL,       NULL,        PREC_NONE },
    [TK_CLASS]          = { NULL,       NULL,        PREC_NONE },
    [TK_ELSE]           = { NULL,       NULL,        PREC_NONE },
    [TK_FALSE]          = { literal,    NULL,        PREC_NONE },
    [TK_FOR]            = { NULL,       NULL,        PREC_NONE },
    [TK_FUN]            = { NULL,       NULL,        PREC_NONE },
    [TK_IF]             = { NULL,       NULL,        PREC_NONE },
    [TK_NIL]            = { literal,    NULL,        PREC_NONE },
    [TK_OR]             = { NULL,       NULL,        PREC_NONE },
    [TK_PRINT]          = { NULL,       NULL,        PREC_NONE },
    [TK_RETURN]         = { NULL,       NULL,        PREC_NONE },
    [TK_SUPER]          = { NULL,       NULL,        PREC_NONE },
    [TK_THIS]           = { NULL,       NULL,        PREC_NONE },
    [TK_TRUE]           = { literal,    NULL,        PREC_NONE },
    [TK_VAR]            = { NULL,       NULL,        PREC_NONE },
    [TK_WHILE]          = { NULL,       NULL,        PREC_NONE },
};

static ParseRule *get_rule(TokenType type)
{
    return &rules[type];
}

//------------------------------------------------------------------------------
// Compiler | Initialize
//------------------------------------------------------------------------------

static void init_compiler(Compiler *compiler, Parser *parser, Chunk *chunk)
{
    compiler->parser = parser;
    compiler->chunk = chunk;
    compiler->parser->vm->compiler = compiler;
}

//------------------------------------------------------------------------------
// Compiler | Statements
//------------------------------------------------------------------------------

bool compile(LoxVM *vm, const char *source, Chunk *chunk)
{
    Scanner scanner;
    init_scanner(&scanner, source);

    Parser parser;
    parser.vm = vm;
    parser.scanner = &scanner;
    parser.had_error = false;
    parser.panic_mode = false;

    Compiler compiler;
    init_compiler(&compiler, &parser, chunk);

    advance(compiler.parser);
    expression(&compiler);
    consume(compiler.parser, TK_EOF, "Expected end of expression");
    end_compiler(&compiler);

    return !compiler.parser->had_error;
}
