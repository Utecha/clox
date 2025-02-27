#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "compiler.h"
#include "lexer.h"
#include "vm.h"

#if DUMP_TOKENS || DUMP_CHUNK
    #include "debug.h"
#endif

#define MAX_LOCALS  256
#define MAX_JUMP    1 << 16

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

typedef enum
{
    TYPE_FUNCTION,
    TYPE_MAIN,
} FunctionType;

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

typedef struct
{
    Token name;
    int depth;
    bool isConst;
} Local;

typedef struct Loop
{
    int start;
    int exit;
    int body;
    int scopeDepth;
    struct Loop *enclosing;
} Loop;

struct Compiler
{
    Parser *parser;
    Compiler *enclosing;
    ObjFn *function;
    FunctionType type;
    Loop *loop;
    Local locals[MAX_LOCALS];
    int localCount;
    int scopeDepth;
};

/* Forward Declarations */
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Compiler *compiler, Precedence precedence);
static void expression(Compiler *compiler);
static void statement(Compiler *compiler);
static void declaration(Compiler *compiler);
static void varDeclaration(Compiler *compiler);

// Parser ----------------------------------------------------------------------
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

static bool check(Compiler *compiler, TokenType type)
{
    return compiler->parser->current.type == type;
}

static void consume(Compiler *compiler, TokenType type, const char *message)
{
    if (check(compiler, type))
    {
        advance(compiler);
        return;
    }

    errorAtCurrent(compiler, message);
}

static bool match(Compiler *compiler, TokenType type)
{
    if (!check(compiler, type)) return false;
    advance(compiler);
    return true;
}

// Compiler --------------------------------------------------------------------
static void initCompiler(Compiler *compiler, Parser *parser, Compiler *enclosing, FunctionType type)
{
    compiler->parser = parser;
    compiler->enclosing = enclosing;
    compiler->function = NULL;
    compiler->type = type;
    compiler->loop = NULL;
    compiler->localCount = 0;
    compiler->scopeDepth = 0;
    compiler->function = newFunction(compiler->parser->vm);
    compiler->parser->vm->compiler = compiler;

    if (type != TYPE_MAIN)
    {
        compiler->function->name = newStringLength(
            compiler->parser->vm,
            compiler->parser->previous.lexeme,
            compiler->parser->previous.length
        );
    }

    Local *local = &compiler->locals[compiler->localCount++];
    local->depth = 0;
    local->name.lexeme = "";
    local->name.length = 0;
}

static Chunk *currentChunk(Compiler *compiler)
{
    return &compiler->function->chunk;
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

static void emitShort(Compiler *compiler, int arg)
{
    emitBytes(compiler, (arg >> 8) & 0xff, arg & 0xff);
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

static int emitJump(Compiler *compiler, uint8_t instruction)
{
    emitByte(compiler, instruction);
    emitBytes(compiler, 0xff, 0xff);
    return currentChunk(compiler)->code.count - 2;
}

static void patchJump(Compiler *compiler, int offset)
{
    int jump = currentChunk(compiler)->code.count - offset - 2;
    if (jump > MAX_JUMP)
    {
        error(compiler, "Too much code to jump over");
    }

    currentChunk(compiler)->code.data[offset] = (jump >> 8) & 0xff;
    currentChunk(compiler)->code.data[offset + 1] = jump & 0xff;
}

static void emitLoop(Compiler *compiler, int loopStart)
{
    emitByte(compiler, OP_LOOP);

    int offset = currentChunk(compiler)->code.count - loopStart + 2;
    if (offset > MAX_JUMP) error(compiler, "Loop body is too large");

    emitShort(compiler, offset);
    // emitByte(compiler, (offset >> 8) & 0xff);
    // emitByte(compiler, offset & 0xff);
}

static void emitReturn(Compiler *compiler)
{
    emitByte(compiler, OP_NIL);
    emitByte(compiler, OP_RETURN);
}

static ObjFn *endCompiler(Compiler *compiler)
{
    emitReturn(compiler);
    ObjFn *function = compiler->function;

    #if DUMP_CHUNK
        if (!compiler->parser->hadError)
        {
            disassemble(
                currentChunk(compiler),
                function->name != NULL ? function->name->value : "main"
            );
        }
    #endif

    compiler->parser->vm->compiler = compiler->enclosing;
    return function;
}

static uint8_t identifierConstant(Compiler *compiler, Token *name)
{
    return makeConstant(
        compiler,
        OBJ_VAL(newStringLength(compiler->parser->vm, name->lexeme, name->length))
    );
}

static bool identifiersEqual(Token *a, Token *b)
{
    if (a->length != b->length) return false;
    return memcmp(a->lexeme, b->lexeme, a->length) == 0;
}

static void addLocal(Compiler *compiler, Token name)
{
    if (compiler->localCount == MAX_LOCALS)
    {
        error(compiler, "Too many local variables in function");
        return;
    }

    Local *local = &compiler->locals[compiler->localCount++];
    local->name = name;
    local->depth = -1;
}

static int resolveLocal(Compiler *compiler, Token *name)
{
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &compiler->locals[i];
        if (identifiersEqual(name, &local->name))
        {
            if (local->depth == -1)
            {
                error(compiler, "Cannot read a local variable within its own initializer");
            }

            return i;
        }
    }

    return -1;
}

static int discardLocals(Compiler *compiler, int depth)
{
    int local = compiler->localCount - 1;
    while (local >= 0 && compiler->locals[local].depth >= depth)
    {
        emitByte(compiler, OP_POP);
        local--;
    }

    return compiler->localCount - local - 1;
}

static void beginScope(Compiler *compiler)
{
    compiler->scopeDepth++;
}

static void endScope(Compiler *compiler)
{
    int popped = discardLocals(compiler, compiler->scopeDepth);
    compiler->localCount -= popped;
    compiler->scopeDepth--;
}

static void conditional(Compiler *compiler, bool canAssign)
{
    int ifJump = emitJump(compiler, OP_JUMP_IF);
    parsePrecedence(compiler, PREC_CONDITIONAL);
    consume(compiler, TK_COLON, "Expected ':' after conditional 'then' branch");

    int elseJump = emitJump(compiler, OP_JUMP);
    patchJump(compiler, ifJump);
    parsePrecedence(compiler, PREC_ASSIGNMENT);
    patchJump(compiler, elseJump);
}

static void or(Compiler *compiler, bool canAssign)
{
    int jump = emitJump(compiler, OP_OR);
    parsePrecedence(compiler, PREC_OR);
    patchJump(compiler, jump);
}

static void and(Compiler *compiler, bool canAssign)
{
    int jump = emitJump(compiler, OP_AND);
    parsePrecedence(compiler, PREC_AND);
    patchJump(compiler, jump);
}

static void binary(Compiler *compiler, bool canAssign)
{
    TokenType operator = compiler->parser->previous.type;
    ParseRule *rule = getRule(operator);
    parsePrecedence(compiler, (Precedence)(rule->precedence + 1));

    switch (operator)
    {
        case TK_BANGEQ:     emitBytes(compiler, OP_EQUAL, OP_NOT); break;
        case TK_EQEQ:       emitByte(compiler, OP_EQUAL); break;
        case TK_GT:         emitByte(compiler, OP_GREATER); break;
        case TK_GTEQ:       emitBytes(compiler, OP_LESS, OP_NOT); break;
        case TK_LT:         emitByte(compiler, OP_LESS); break;
        case TK_LTEQ:       emitBytes(compiler, OP_GREATER, OP_NOT); break;
        case TK_MINUS:      emitByte(compiler, OP_SUBTRACT); break;
        case TK_PLUS:       emitByte(compiler, OP_ADD); break;
        case TK_SLASH:      emitByte(compiler, OP_DIVIDE); break;
        case TK_STAR:       emitByte(compiler, OP_MULTIPLY); break;
        case TK_MODULUS:    emitByte(compiler, OP_REMAINDER); break;
        case TK_POWER:      emitByte(compiler, OP_POWER); break;
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

static uint8_t argumentList(Compiler *compiler)
{
    uint8_t argc = 0;
    if (!check(compiler, TK_RPAREN))
    {
        do
        {
            expression(compiler);
            if (argc == 255)
            {
                error(compiler, "Cannot have more than 255 arguments to a call expression");
            }

            argc++;
        } while (match(compiler, TK_COMMA));
    }

    consume(compiler, TK_RPAREN, "Expected ')' after arguments");
    return argc;
}

static void call(Compiler *compiler, bool canAssign)
{
    uint8_t argc = argumentList(compiler);
    emitBytes(compiler, OP_CALL, argc);
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

static void namedVariable(Compiler *compiler, Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(compiler, &name);

    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else
    {
        arg = identifierConstant(compiler, &name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && match(compiler, TK_EQUAL))
    {
        expression(compiler);
        emitBytes(compiler, setOp, (uint8_t)arg);
    }
    else
    {
        emitBytes(compiler, getOp, (uint8_t)arg);
    }
}

static void variable(Compiler *compiler, bool canAssign)
{
    namedVariable(compiler, compiler->parser->previous, canAssign);
}

#define UNUSED                      { NULL, NULL, PREC_NONE }
#define PREFIX(fn)                  { fn, NULL, PREC_NONE }
#define INFIX(fn, prec)             { NULL, fn, prec }
#define MIXFIX(preFn, inFn, prec)   { preFn, inFn, prec }

ParseRule rules[] = {
    [TK_ERROR]          = UNUSED,
    [TK_EOF]            = UNUSED,
    [TK_LPAREN]         = MIXFIX(grouping, call, PREC_CALL),
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
    [TK_MODULUS]        = INFIX(binary, PREC_FACTOR),
    [TK_POWER]          = INFIX(binary, PREC_FACTOR),
    [TK_BANG]           = PREFIX(unary),
    [TK_IDENTIFIER]     = PREFIX(variable),
    [TK_NUMBER]         = PREFIX(number),
    [TK_STRING]         = PREFIX(string),
    [TK_AND]            = MIXFIX(NULL, and, PREC_AND),
    [TK_BREAK]          = UNUSED,
    [TK_CLASS]          = UNUSED,
    [TK_CONST]          = UNUSED,
    [TK_CONTINUE]       = UNUSED,
    [TK_ELSE]           = UNUSED,
    [TK_FALSE]          = PREFIX(literal),
    [TK_FOR]            = UNUSED,
    [TK_FUN]            = UNUSED,
    [TK_IF]             = UNUSED,
    [TK_NIL]            = PREFIX(literal),
    [TK_OR]             = MIXFIX(NULL, or, PREC_OR),
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

    if (canAssign && match(compiler, TK_EQUAL))
    {
        error(compiler, "Invalid assignment target");
    }
}

static void expression(Compiler *compiler)
{
    parsePrecedence(compiler, PREC_ASSIGNMENT);
}

static void startLoop(Compiler *compiler, Loop *loop)
{
    loop->enclosing = compiler->loop;
    loop->start = currentChunk(compiler)->code.count;
    loop->scopeDepth = compiler->scopeDepth;
    compiler->loop = loop;
}

static void loopBody(Compiler *compiler)
{
    compiler->loop->body = currentChunk(compiler)->code.count;
    statement(compiler);
}

static void testLoopExit(Compiler *compiler)
{
    compiler->loop->exit = emitJump(compiler, OP_JUMP_IF);
}

static int getByteCount(const uint8_t *code, const Value *constants, int ip)
{
    switch (code[ip])
    {
        // Simple Instructions (1 byte)
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
        case OP_END:
            return 0;

        case OP_CONSTANT:
        case OP_DEFINE_GLOBAL:
        case OP_GET_GLOBAL:
        case OP_SET_GLOBAL:
        case OP_GET_LOCAL:
        case OP_SET_LOCAL:
            return 1;

        case OP_JUMP:
        case OP_JUMP_IF:
        case OP_LOOP:
        case OP_AND:
        case OP_OR:
            return 2;
    }

    return -1;
}

static void endLoop(Compiler *compiler)
{
    if (compiler->loop->exit != -1)
    {
        patchJump(compiler, compiler->loop->exit);
        emitByte(compiler, OP_POP);
    }

    int i = compiler->loop->body;
    while (i < currentChunk(compiler)->code.count)
    {
        if (currentChunk(compiler)->code.data[i] == OP_END)
        {
            currentChunk(compiler)->code.data[i] = OP_JUMP;
            patchJump(compiler, i + 1);
            i += 3;
        }
        else
        {
            i += 1 + getByteCount(currentChunk(compiler)->code.data, currentChunk(compiler)->constants.data, i);
        }
    }

    compiler->loop = compiler->loop->enclosing;
}

static void block(Compiler *compiler)
{
    while (!check(compiler, TK_RBRACE) && !check(compiler, TK_EOF))
        { declaration(compiler); }

    consume(compiler, TK_RBRACE, "Expected '}' after block");
}

static void breakStatement(Compiler *compiler)
{
    if (compiler->loop == NULL)
    {
        error(compiler, "Cannot use 'break' outside of a loop");
        return;
    }

    consume(compiler, TK_SEMICOLON, "Expected ';' after 'break'");
    discardLocals(compiler, compiler->loop->scopeDepth + 1);
    emitJump(compiler, OP_END);
}

static void continueStatement(Compiler *compiler)
{
    if (compiler->loop == NULL)
    {
        error(compiler, "Cannot use 'continue' outside of a loop");
        return;
    }

    consume(compiler, TK_SEMICOLON, "Expected ';' after 'continue'");
    discardLocals(compiler, compiler->loop->scopeDepth + 1);
    emitLoop(compiler, compiler->loop->start);
}

static void expressionStatement(Compiler *compiler)
{
    if (match(compiler, TK_SEMICOLON)) return;
    expression(compiler);
    consume(compiler, TK_SEMICOLON, "Expected ';' after expression.");
    emitByte(compiler, OP_POP);
}

static void forStatement(Compiler *compiler)
{
    beginScope(compiler);

    bool hasLeftParen = false;
    if (match(compiler, TK_LPAREN)) hasLeftParen = true;

    /* Initializer */
    if (match(compiler, TK_SEMICOLON))
        { /* No initializer */ }
    else if (match(compiler, TK_VAR))
        { varDeclaration(compiler); }
    else
        { expressionStatement(compiler); }

    Loop loop;
    startLoop(compiler, &loop);

    /* Condition */
    compiler->loop->exit = -1;
    if (!match(compiler, TK_SEMICOLON))
    {
        expression(compiler);
        consume(compiler, TK_SEMICOLON, "Expected ';' after 'for' condition");

        // Exit the loop if the condition is false
        testLoopExit(compiler);
        emitByte(compiler, OP_POP);
    }

    /* Increment */
    if (!match(compiler, TK_RPAREN))
    {
        int bodyJump = emitJump(compiler, OP_JUMP);
        int incrementStart = currentChunk(compiler)->code.count;

        expression(compiler);
        emitByte(compiler, OP_POP);
        if (match(compiler, TK_RPAREN))
        {
            if (!hasLeftParen) error(compiler, "Right parenthesis found with no matching left");
        }
        else
        {
            if (hasLeftParen) error(compiler, "Left parenthesis found with no matching right");
        }

        emitLoop(compiler, compiler->loop->start);
        compiler->loop->start = incrementStart;
        patchJump(compiler, bodyJump);
    }

    if (!(compiler->parser->previous.type == TK_RPAREN) && !hasLeftParen && !check(compiler, TK_LBRACE))
    {
        error(compiler, "Parenthesis surrounding 'for' clauses are required for single line 'for' loops");
    }

    loopBody(compiler);
    emitLoop(compiler, compiler->loop->start);
    endLoop(compiler);
    endScope(compiler);
}

static void ifStatement(Compiler *compiler)
{
    // consume(compiler, TK_LPAREN, "Expected '(' after 'if'");
    expression(compiler);
    // consume(compiler, TK_RPAREN, "Expected ')' after 'if' condition");

    int thenJump = emitJump(compiler, OP_JUMP_IF);
    emitByte(compiler, OP_POP);
    statement(compiler);

    int elseJump = emitJump(compiler, OP_JUMP);
    patchJump(compiler, thenJump);
    emitByte(compiler, OP_POP);

    if (match(compiler, TK_ELSE)) statement(compiler);
    patchJump(compiler, elseJump);
}

static void printStatement(Compiler *compiler)
{
    expression(compiler);
    consume(compiler, TK_SEMICOLON, "Expected ';' after 'value'.");
    emitByte(compiler, OP_PRINT);
}

static void returnStatement(Compiler *compiler)
{
    if (compiler->type == TYPE_MAIN)
    {
        error(compiler, "Cannot return from top-level code");
    }

    if (match(compiler, TK_SEMICOLON))
    {
        emitReturn(compiler);
    }
    else
    {
        expression(compiler);
        consume(compiler, TK_SEMICOLON, "Expected ';' after return value");
        emitByte(compiler, OP_RETURN);
    }
}

static void whileStatement(Compiler *compiler)
{
    Loop loop;
    startLoop(compiler, &loop);

    // consume(compiler, TK_LPAREN, "Expected '(' after 'while'");
    expression(compiler);
    // consume(compiler, TK_RPAREN, "Expected ')' after 'while' condition");

    testLoopExit(compiler);
    emitByte(compiler, OP_POP);

    loopBody(compiler);
    emitLoop(compiler, loop.start);
    endLoop(compiler);
}

static void statement(Compiler *compiler)
{
    if (match(compiler, TK_BREAK))
    {
        breakStatement(compiler);
    }
    else if (match(compiler, TK_CONTINUE))
    {
        continueStatement(compiler);
    }
    else if (match(compiler, TK_FOR))
    {
        forStatement(compiler);
    }
    else if (match(compiler, TK_IF))
    {
        ifStatement(compiler);
    }
    else if (match(compiler, TK_PRINT))
    {
        printStatement(compiler);
    }
    else if (match(compiler, TK_RETURN))
    {
        returnStatement(compiler);
    }
    else if (match(compiler, TK_WHILE))
    {
        whileStatement(compiler);
    }
    else if (match(compiler, TK_LBRACE))
    {
        beginScope(compiler);
        block(compiler);
        endScope(compiler);
    }
    else
    {
        expressionStatement(compiler);
    }
}

static void synchronize(Compiler *compiler)
{
    Parser *parser = compiler->parser;
    parser->panicMode = false;

    while (parser->current.type != TK_EOF)
    {
        if (parser->previous.type == TK_SEMICOLON) return;
        switch (parser->current.type)
        {
            case TK_CLASS:
            case TK_FOR:
            case TK_FUN:
            case TK_IF:
            case TK_PRINT:
            case TK_RETURN:
            case TK_WHILE:
                return;
            default:
                ; // Do nothing
        }

        advance(compiler);
    }
}

static void markInitialized(Compiler *compiler)
{
    if (compiler->scopeDepth == 0) return;
    compiler->locals[compiler->localCount - 1].depth = compiler->scopeDepth;
}

static void declareVariable(Compiler *compiler)
{
    if (compiler->scopeDepth == 0) return;

    Token *name = &compiler->parser->previous;
    for (int i = compiler->localCount - 1; i >= 0; i--)
    {
        Local *local = &compiler->locals[i];
        if (local->depth != -1 && local->depth < compiler->scopeDepth)
        {
            break;
        }

        if (identifiersEqual(name, &local->name))
        {
            error(compiler, "Already a variable with this name in this scope");
        }
    }

    addLocal(compiler, *name);
}

static uint8_t parseVariable(Compiler *compiler, const char *message)
{
    consume(compiler, TK_IDENTIFIER, message);
    declareVariable(compiler);
    if (compiler->scopeDepth > 0) return 0;
    return identifierConstant(compiler, &compiler->parser->previous);
}

static void defineVariable(Compiler *compiler, uint8_t global)
{
    if (compiler->scopeDepth > 0)
    {
        markInitialized(compiler);
        return;
    }

    emitBytes(compiler, OP_DEFINE_GLOBAL, global);
}

static void funDefinition(Compiler *parent, FunctionType type)
{
    Compiler compiler;
    initCompiler(&compiler, parent->parser, parent, type);
    beginScope(&compiler);

    consume(&compiler, TK_LPAREN, "Expected '(' after function name");
    if (!check(&compiler, TK_RPAREN))
    {
        do
        {
            compiler.function->arity++;
            if (compiler.function->arity > 255)
            {
                errorAtCurrent(&compiler, "Cannot have more than 255 parameters to a function");
            }

            uint8_t constant = parseVariable(&compiler, "Expected parameter name");
            defineVariable(&compiler, constant);
        } while (match(&compiler, TK_COMMA));
    }

    consume(&compiler, TK_RPAREN, "Expected ')' after function parameters");
    consume(&compiler, TK_LBRACE, "Expected '{' before function body");
    block(&compiler);

    ObjFn *function = endCompiler(&compiler);
    emitBytes(compiler.enclosing, OP_CONSTANT, makeConstant(compiler.enclosing, OBJ_VAL(function)));
}

static void funDeclaration(Compiler *compiler)
{
    uint8_t global = parseVariable(compiler, "Expected function name");
    markInitialized(compiler);
    funDefinition(compiler, TYPE_FUNCTION);
    defineVariable(compiler, global);
}

static void varDeclaration(Compiler *compiler)
{
    uint8_t global = parseVariable(compiler, "Expected variable name");

    if (match(compiler, TK_EQUAL))
    {
        expression(compiler);
    }
    else
    {
        emitByte(compiler, OP_NIL);
    }

    consume(compiler, TK_SEMICOLON, "Expected ';' after variable declaration");
    defineVariable(compiler, global);
}

static void declaration(Compiler *compiler)
{
    if (match(compiler, TK_FUN))
    {
        funDeclaration(compiler);
    }
    else if (match(compiler, TK_VAR))
        { varDeclaration(compiler); }
    else
        { statement(compiler); }

    if (compiler->parser->panicMode) synchronize(compiler);
}

ObjFn *compile(LoxVM *vm, const char *source)
{
    #if DUMP_TOKENS
        dumpTokens(source);
    #endif

    Lexer lexer;
    initLexer(&lexer, source);

    Parser parser;
    initParser(&parser, &lexer, vm);

    Compiler compiler;
    initCompiler(&compiler, &parser, NULL, TYPE_MAIN);

    advance(&compiler);
    while (!match(&compiler, TK_EOF))
    {
        declaration(&compiler);
    }

    ObjFn *function = endCompiler(&compiler);
    return compiler.parser->hadError ? NULL : function;
}
