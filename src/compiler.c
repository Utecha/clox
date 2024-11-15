#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include/chunk.h"
#include "include/common.h"
#include "include/compiler.h"
#include "include/object.h"
#include "include/memory.h"
#include "include/scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "include/debug.h"
#endif // DEBUG_PRINT_CODE

typedef struct {
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum {
    PR_NONE,
    PR_ASSIGNMENT,      // =
    PR_OR,              // or
    PR_AND,             // and
    PR_EQUALITY,        // == !=
    PR_COMPARISON,      // > >= < <=
    PR_TERM,            // - +
    PR_FACTOR,          // / *
    PR_UNARY,           // ! -
    PR_CALL,            // . ()
    PR_PRIMARY
} Precedence;

typedef void (*ParseFn)(bool can_assign);

typedef struct {
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct {
    Token name;
    int depth;
    bool is_captured;
} Local;

typedef struct {
    uint8_t index;
    bool is_local;
} Upvalue;

typedef enum {
    TYPE_FUNCTION,
    TYPE_INITIALIZER,
    TYPE_METHOD,
    TYPE_SCRIPT
} FunctionType;

typedef struct Compiler {
    struct Compiler *enclosing;
    ObjFunction *function;
    FunctionType type;

    Local locals[UINT8_COUNT];
    int local_count;
    Upvalue upvalues[UINT8_COUNT];
    int scope_depth;
} Compiler;

typedef struct ClassCompiler {
    struct ClassCompiler *enclosing;
    bool has_superclass;
} ClassCompiler;

Parser parser;
Compiler *current = NULL;
ClassCompiler *current_class = NULL;

Chunk *current_chunk()
{
    return &current->function->chunk;
}

static void error_at(Token *token, const char *message)
{
    if (parser.panic_mode) return;
    parser.panic_mode = true;

    fprintf(stderr, "[COMPILE ERROR] %s ", message);
    fprintf(stderr, "on [line %d]", token->line);

    if (token->type == TK_EOF) {
        fprintf(stderr, " at end\n");
    } else if (token->type == TK_ERROR) {
        fputs("\n", stderr);
    } else {
        fprintf(stderr, " at '%.*s'\n", token->len, token->start);
    }

    parser.had_error = true;
}

static void error(const char *message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char *message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    for (;;) {
        parser.current = scan_token();
        if (parser.current.type != TK_ERROR) break;

        error_at_current(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type) {
        advance();
        return;
    }

    error_at_current(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

static bool match(TokenType type)
{
    if (!check(type)) return false;
    advance();
    return true;
}

static void emit_byte(uint8_t byte)
{
    write_chunk(current_chunk(), byte, parser.previous.line);
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_loop(int loop_start)
{
    emit_byte(OP_LOOP);

    int offset = current_chunk()->count - loop_start + 2;
    if (offset > UINT16_MAX) error("Loop body is too large");

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction)
{
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);

    return current_chunk()->count - 2;
}

static void emit_return()
{
    if (current->type == TYPE_INITIALIZER) {
        emit_bytes(OP_GET_LOCAL, 0);
    } else {
        emit_byte(OP_NIL);
    }

    emit_byte(OP_RETURN);
}

static uint8_t make_constant(Value value)
{
    int constant = add_constant(current_chunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void patch_jump(int offset)
{
    // -2 adjusts for the jump's bytecode offset
    int jump = current_chunk()->count - offset - 2;

    if (jump > UINT16_MAX) {
        error("Too much code to jump over");
    }

    current_chunk()->code[offset] = (jump >> 8) & 0xff;
    current_chunk()->code[offset + 1] = jump & 0xff;
}

static void init_compiler(Compiler *compiler, FunctionType type)
{
    compiler->enclosing = current;
    compiler->function = NULL;
    compiler->type = type;
    compiler->local_count = 0;
    compiler->scope_depth = 0;
    compiler->function = new_function();
    current = compiler;

    if (type != TYPE_SCRIPT) {
        current->function->name = copy_string(
            parser.previous.start, parser.previous.len
        );
    }

    Local *local = &current->locals[current->local_count++];
    local->depth = 0;
    local->is_captured = false;
    if (type != TYPE_FUNCTION) {
        local->name.start = "this";
        local->name.len = 4;
    } else {
        local->name.start = "";
        local->name.len = 0;
    }
}

static ObjFunction *end_compiler()
{
    emit_return();
    ObjFunction *function = current->function;

#ifdef DEBUG_PRINT_CODE
    if (!parser.had_error) {
        disassemble_chunk(current_chunk(), function->name != NULL
            ? function->name->chars : "<script>");
    }
#endif // DEBUG_PRINT_CODE

    current = current->enclosing;
    return function;
}

static void begin_scope()
{
    current->scope_depth++;
}

static void end_scope()
{
    current->scope_depth--;

    while (current->local_count > 0 &&
            current->locals[current->local_count - 1].depth >
            current->scope_depth)
    {
        if (current->locals[current->local_count - 1].is_captured) {
            emit_byte(OP_CLOSE_UPVALUE);
        } else {
            emit_byte(OP_POP);
        }
        current->local_count--;
    }
}

/* Forward Declarations Begin */
static ParseRule *get_rule(TokenType type);
static void parse_precedence(Precedence precedence);
static void expression();
static void declaration();
static void statement();
/* Forward Declarations End */

static uint8_t ident_constant(Token *name)
{
    return make_constant(OBJ_VAL(copy_string(name->start, name->len)));
}

static bool identifiers_equal(Token *a, Token *b) {
    if (a->len != b->len) return false;
    return memcmp(a->start, b->start, a->len) == 0;
}

static void add_local(Token name)
{
    if (current->local_count == UINT8_COUNT) {
        error("Too many local variables in function");
        return;
    }

    Local *local = &current->locals[current->local_count++];
    local->name = name;
    local->depth = -1;
    local->is_captured = false;
}

static int resolve_local(Compiler *compiler, Token *name)
{
    for (int i = compiler->local_count - 1; i >= 0; i--) {
        Local *local = &compiler->locals[i];
        if (identifiers_equal(name, &local->name)) {
            if (local->depth == -1) {
                error("Can't read local variable within its own initializer");
            }

            return i;
        }
    }

    return -1;
}

static int add_upvalue(Compiler *compiler, uint8_t index, bool is_local)
{
    int upvalue_count = compiler->function->upvalue_count;

    for (int i = 0; i < upvalue_count; i++) {
        Upvalue *upvalue = &compiler->upvalues[i];
        if (upvalue->index == index && upvalue->is_local == is_local) {
            return i;
        }
    }

    if (upvalue_count == UINT8_COUNT) {
        error("Too many closure variables in function");
        return 0;
    }

    compiler->upvalues[upvalue_count].is_local = is_local;
    compiler->upvalues[upvalue_count].index = index;
    return compiler->function->upvalue_count++;
}

static int resolve_upvalue(Compiler *compiler, Token *name)
{
    if (compiler->enclosing == NULL) return -1;

    int local = resolve_local(compiler->enclosing, name);
    if (local != -1) {
        compiler->enclosing->locals[local].is_captured = true;
        return add_upvalue(compiler, (uint8_t)local, true);
    }

    int upvalue = resolve_upvalue(compiler->enclosing, name);
    if (upvalue != -1) {
        return add_upvalue(compiler, (uint8_t)upvalue, false);
    }

    return -1;
}

static void declare_variable()
{
    if (current->scope_depth == 0) return;

    Token *name = &parser.previous;
    for (int i = current->local_count - 1; i >= 0; i--) {
        Local *local = &current->locals[i];
        if (local->depth != -1 && local->depth < current->scope_depth) {
            break;
        }

        if (identifiers_equal(name, &local->name)) {
            error("Already a variable with this name in this scope");
        }
    }

    add_local(*name);
}

static uint8_t parse_variable(const char *message)
{
    consume(TK_IDENTIFIER, message);

    declare_variable();
    if (current->scope_depth > 0) return 0;

    return ident_constant(&parser.previous);
}

static void mark_initialized()
{
    if (current->scope_depth == 0) return;
    current->locals[current->local_count - 1].depth = current->scope_depth;
}

static void define_variable(uint8_t global)
{
    if (current->scope_depth > 0) {
        mark_initialized();
        return;
    }

    emit_bytes(OP_DEFINE_GLOBAL, global);
}

static uint8_t argument_list()
{
    uint8_t arg_count = 0;
    if (!check(TK_RPAREN)) {
        do {
            expression();
            if (arg_count == 255) {
                error("Cannot have more than 255 arguments");
            }
            arg_count++;
        } while (match(TK_COMMA));
    }

    consume(TK_RPAREN, "Expected ')' after arguments");
    return arg_count;
}

static void and_(bool can_assign)
{
    int end_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_byte(OP_POP);
    parse_precedence(PR_AND);

    patch_jump(end_jump);
}

static void or_(bool can_assign)
{
    int else_jump = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OP_JUMP);

    patch_jump(else_jump);
    emit_byte(OP_POP);

    parse_precedence(PR_OR);
    patch_jump(end_jump);
}

static void binary(bool can_assign)
{
    TokenType op_type = parser.previous.type;
    ParseRule *rule = get_rule(op_type);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (op_type) {
        case TK_NOTEQ:      emit_bytes(OP_EQUAL, OP_NOT); break;
        case TK_CMPEQ:      emit_byte(OP_EQUAL); break;
        case TK_GT:         emit_byte(OP_GREATER); break;
        case TK_GTEQ:       emit_bytes(OP_LESS, OP_NOT); break;
        case TK_LT:         emit_byte(OP_LESS); break;
        case TK_LTEQ:       emit_bytes(OP_GREATER, OP_NOT); break;
        case TK_MINUS:      emit_byte(OP_SUBTRACT); break;
        case TK_PLUS:       emit_byte(OP_ADD); break;
        case TK_SLASH:      emit_byte(OP_DIVIDE); break;
        case TK_STAR:       emit_byte(OP_MULTIPLY); break;
        default: return; // Unreachable
    }
}

static void call(bool can_assign)
{
    uint8_t arg_count = argument_list();
    emit_bytes(OP_CALL, arg_count);
}

static void dot(bool can_assign)
{
    consume(TK_IDENTIFIER, "Expected property name after '.'");
    uint8_t name = ident_constant(&parser.previous);

    if (can_assign && match(TK_EQUAL)) {
        expression();
        emit_bytes(OP_SET_PROPERTY, name);
    } else if (match(TK_LPAREN)) {
        uint8_t arg_count = argument_list();
        emit_bytes(OP_INVOKE, name);
        emit_byte(arg_count);
    } else {
        emit_bytes(OP_GET_PROPERTY, name);
    }
}

static void literal(bool can_assign)
{
    switch (parser.previous.type) {
        case TK_NIL:        emit_byte(OP_NIL); break;
        case TK_TRUE:       emit_byte(OP_TRUE); break;
        case TK_FALSE:      emit_byte(OP_FALSE); break;
        default: return; // Unreachable
    }
}

static void grouping(bool can_assign)
{
    expression();
    consume(TK_RPAREN, "Expected ')' after expression");
}

static void number(bool can_assign)
{
    double value = strtod(parser.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void string(bool can_assign)
{
    emit_constant(
        OBJ_VAL(copy_string(
            parser.previous.start + 1,
            parser.previous.len - 2
        ))
    );
}

static void named_variable(Token name, bool can_assign)
{
    uint8_t get_op, set_op;
    int arg = resolve_local(current, &name);

    if (arg != -1) {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    } else if ((arg = resolve_upvalue(current, &name)) != -1) {
        get_op = OP_GET_UPVALUE;
        set_op = OP_SET_UPVALUE;
    } else {
        arg = ident_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (match(TK_EQUAL) && can_assign) {
        expression();
        emit_bytes(set_op, (uint8_t)arg);
    } else {
        emit_bytes(get_op, (uint8_t)arg);
    }
}

static void variable(bool can_assign)
{
    named_variable(parser.previous, can_assign);
}

static Token synthetic_token(const char *text)
{
    Token token;
    token.start = text;
    token.len = (int)strlen(text);
    return token;
}

static void super_(bool can_assign)
{
    if (current_class == NULL) {
        error("Cannot use 'super' outside of a class");
    } else if (!current_class->has_superclass) {
        error("Cannot use 'super' in a class with no superclass");
    }

    consume(TK_DOT, "Expected '.' after 'super'");
    consume(TK_IDENTIFIER, "Expected superclass method name");
    uint8_t name = ident_constant(&parser.previous);

    named_variable(synthetic_token("this"), false);

    if (match(TK_LPAREN)) {
        uint8_t arg_count = argument_list();
        named_variable(synthetic_token("super"), false);
        emit_bytes(OP_SUPER_INVOKE, name);
        emit_byte(arg_count);
    } else {
        named_variable(synthetic_token("super"), false);
        emit_bytes(OP_GET_SUPER, name);
    }
}

static void this_(bool can_assign)
{
    if (current_class == NULL) {
        error("Cannot use 'this' outside of a class");
        return;
    }

    variable(false);
}

static void unary(bool can_assign)
{
    TokenType op_type = parser.previous.type;

    parse_precedence(PR_UNARY);

    switch (op_type) {
        case TK_NOT:    emit_byte(OP_NOT); break;
        case TK_MINUS:  emit_byte(OP_NEGATE); break;
        default: return; // Unreachable
    }
}

ParseRule rules[] = {
    [TK_LPAREN]     = { grouping,   call,       PR_CALL },
    [TK_RPAREN]     = { NULL,       NULL,       PR_NONE },
    [TK_LBRACE]     = { NULL,       NULL,       PR_NONE },
    [TK_RBRACE]     = { NULL,       NULL,       PR_NONE },
    [TK_COMMA]      = { NULL,       NULL,       PR_NONE },
    [TK_DOT]        = { NULL,       dot,        PR_CALL },
    [TK_SEMICOLON]  = { NULL,       NULL,       PR_NONE },
    [TK_MINUS]      = { unary,      binary,     PR_TERM },
    [TK_PLUS]       = { NULL,       binary,     PR_TERM },
    [TK_SLASH]      = { NULL,       binary,     PR_FACTOR },
    [TK_STAR]       = { NULL,       binary,     PR_FACTOR },
    [TK_NOT]        = { unary,      NULL,       PR_NONE },
    [TK_NOTEQ]      = { NULL,       binary,     PR_EQUALITY },
    [TK_EQUAL]      = { NULL,       binary,     PR_NONE },
    [TK_CMPEQ]      = { NULL,       binary,     PR_EQUALITY },
    [TK_GT]         = { NULL,       binary,     PR_COMPARISON },
    [TK_GTEQ]       = { NULL,       binary,     PR_COMPARISON },
    [TK_LT]         = { NULL,       binary,     PR_COMPARISON },
    [TK_LTEQ]       = { NULL,       binary,     PR_COMPARISON },
    [TK_IDENTIFIER] = { variable,   NULL,       PR_NONE },
    [TK_NUMBER]     = { number,     NULL,       PR_NONE },
    [TK_STRING]     = { string,     NULL,       PR_NONE },
    [TK_AND]        = { NULL,       and_,       PR_AND },
    [TK_CLASS]      = { NULL,       NULL,       PR_NONE },
    [TK_ELSE]       = { NULL,       NULL,       PR_NONE },
    [TK_FALSE]      = { literal,    NULL,       PR_NONE },
    [TK_FOR]        = { NULL,       NULL,       PR_NONE },
    [TK_FUN]        = { NULL,       NULL,       PR_NONE },
    [TK_IF]         = { NULL,       NULL,       PR_NONE },
    [TK_NIL]        = { literal,    NULL,       PR_NONE },
    [TK_OR]         = { NULL,       or_,        PR_OR },
    [TK_PRINT]      = { NULL,       NULL,       PR_NONE },
    [TK_RETURN]     = { NULL,       NULL,       PR_NONE },
    [TK_SUPER]      = { super_,     NULL,       PR_NONE },
    [TK_THIS]       = { this_,      NULL,       PR_NONE },
    [TK_TRUE]       = { literal,    NULL,       PR_NONE },
    [TK_VAR]        = { NULL,       NULL,       PR_NONE },
    [TK_WHILE]      = { NULL,       NULL,       PR_NONE },
    [TK_ERROR]      = { NULL,       NULL,       PR_NONE },
    [TK_EOF]        = { NULL,       NULL,       PR_NONE },
};

static ParseRule *get_rule(TokenType type)
{
    return &rules[type];
}

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.type)->prefix;
    if (prefix_rule == NULL) {
        error("Expected expression");
        return;
    }

    bool can_assign = precedence <= PR_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.type)->precedence) {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.type)->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(TK_EQUAL)) {
        error("Invalid assignment target.");
    }
}

static void expression()
{
    parse_precedence(PR_ASSIGNMENT);
}

static void block()
{
    while (!check(TK_RBRACE) && !check(TK_EOF)) {
        declaration();
    }

    consume(TK_RBRACE, "Expected '}' after block");
}

static void function(FunctionType type)
{
    Compiler compiler;
    init_compiler(&compiler, type);
    begin_scope();

    consume(TK_LPAREN, "Expected '(' after function name");
    if (!check(TK_RPAREN)) {
        do {
            current->function->arity++;
            if (current->function->arity > 255) {
                error_at_current("Cannot have more than 255 parameters");
            }

            uint8_t constant = parse_variable("Expected parameter name");
            define_variable(constant);
        } while(match(TK_COMMA));
    }

    consume(TK_RPAREN, "Expected ')' after function parameters");
    consume(TK_LBRACE, "Expected '{' before function body");
    block();

    ObjFunction *function = end_compiler();
    emit_bytes(OP_CLOSURE, make_constant(OBJ_VAL(function)));

    for (int i = 0; i < function->upvalue_count; i++) {
        emit_byte(compiler.upvalues[i].is_local ? 1 : 0);
        emit_byte(compiler.upvalues[i].index);
    }
}

static void method()
{
    consume(TK_IDENTIFIER, "Expected method name");
    uint8_t constant = ident_constant(&parser.previous);

    FunctionType type = TYPE_METHOD;
    if (parser.previous.len == 4 && memcmp(parser.previous.start, "init", 4) == 0) {
        type = TYPE_INITIALIZER;
    }

    function(type);

    emit_bytes(OP_METHOD, constant);
}

static void class_declaration()
{
    consume(TK_IDENTIFIER, "Expected class name");
    Token class_name = parser.previous;

    uint8_t name_constant = ident_constant(&parser.previous);
    declare_variable();

    emit_bytes(OP_CLASS, name_constant);
    define_variable(name_constant);

    ClassCompiler class_compiler;
    class_compiler.has_superclass = false;
    class_compiler.enclosing = current_class;
    current_class = &class_compiler;

    if (match(TK_LT)) {
        consume(TK_IDENTIFIER, "Expected superclass name");
        variable(false);

        if (identifiers_equal(&class_name, &parser.previous)) {
            error("A class cannot inherit from itself");
        }

        begin_scope();
        add_local(synthetic_token("super"));
        define_variable(0);

        named_variable(class_name, false);
        emit_byte(OP_INHERIT);
        class_compiler.has_superclass = true;
    }

    named_variable(class_name, false);
    consume(TK_LBRACE, "Expected '{' before class body");
    while (!check(TK_RBRACE) && !check(TK_EOF)) {
        method();
    }

    consume(TK_RBRACE, "Expected '}' after class body");
    emit_byte(OP_POP);

    if (class_compiler.has_superclass) {
        end_scope();
    }

    current_class = current_class->enclosing;
}

static void fun_declaration()
{
    uint8_t global = parse_variable("Expected function name");
    mark_initialized();
    function(TYPE_FUNCTION);
    define_variable(global);
}

static void var_declaration()
{
    uint8_t global = parse_variable("Expected variable name");

    if (match(TK_EQUAL)) {
        expression();
    } else {
        emit_byte(OP_NIL);
    }

    consume(TK_SEMICOLON, "Expected ';' after 'var' declaration.");
    define_variable(global);
}

static void expression_statement()
{
    expression();
    consume(TK_SEMICOLON, "Expected ';' after expression");
    emit_byte(OP_POP);
}

static void if_statement()
{
    consume(TK_LPAREN, "Expected '(' after 'if'");
    expression();
    consume(TK_RPAREN, "Exected ')' after 'if' condition");

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    int else_jump = emit_jump(OP_JUMP);
    patch_jump(then_jump);
    emit_byte(OP_POP);

    if (match(TK_ELSE)) statement();
    patch_jump(else_jump);
}

static void for_statement()
{
    begin_scope();

    consume(TK_LPAREN, "Expected '(' after 'for'");
    if (match(TK_SEMICOLON)) {
        // No initializer
    } else if (match(TK_VAR)) {
        var_declaration();
    } else {
        expression_statement();
    }

    int loop_start = current_chunk()->count;
    int exit_jump = -1;
    if (!match(TK_SEMICOLON)) {
        expression();
        consume(TK_SEMICOLON, "Expected ';' after loop_condition");

        // Jump out of the loop if the condition is false
        exit_jump = emit_jump(OP_JUMP_IF_FALSE);
        emit_byte(OP_POP);
    }

    if (!match(TK_RPAREN)) {
        int body_jump = emit_jump(OP_JUMP);
        int inc_start = current_chunk()->count;

        expression();
        emit_byte(OP_POP);
        consume(TK_RPAREN, "Expected ')' after 'for' clauses");

        emit_loop(loop_start);
        loop_start = inc_start;
        patch_jump(body_jump);
    }

    statement();
    emit_loop(loop_start);

    if (exit_jump != -1) {
        patch_jump(exit_jump);
        emit_byte(OP_POP); // Condition
    }

    end_scope();
}

static void print_statement()
{
    expression();
    consume(TK_SEMICOLON, "Expected ';' after 'print' value");
    emit_byte(OP_PRINT);
}

static void return_statement()
{
    if (current->type == TYPE_SCRIPT) {
        error("Cannot return from top-level code");
    }

    if (match(TK_SEMICOLON)) {
        emit_return();
    } else {
        if (current->type == TYPE_INITIALIZER) {
            error("Cannot return a value from an initializer");
        }

        expression();
        consume(TK_SEMICOLON, "Expected ';' after 'return' value");
        emit_byte(OP_RETURN);
    }
}

static void while_statement()
{
    int loop_start = current_chunk()->count;
    consume(TK_LPAREN, "Expected '(' after 'while'");
    expression();
    consume(TK_RPAREN, "Expected ')' after 'while' condition");

    int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OP_POP);
}

static void synchronize()
{
    parser.panic_mode = false;

    while (parser.current.type != TK_EOF) {
        if (parser.previous.type == TK_SEMICOLON) return;
        switch (parser.current.type) {
            case TK_CLASS:
            case TK_FOR:
            case TK_FUN:
            case TK_IF:
            case TK_PRINT:
            case TK_RETURN:
            case TK_VAR:
            case TK_WHILE:
                return;
            default:
                ; // Do nothing
        }

        advance();
    }
}

static void declaration()
{
    if (match(TK_CLASS)) {
        class_declaration();
    } else if (match(TK_FUN)) {
        fun_declaration();
    } else if (match(TK_VAR)) {
        var_declaration();
    } else {
        statement();
    }

    if (parser.panic_mode) synchronize();
}

static void statement()
{
    if (match(TK_PRINT)) {
        print_statement();
    } else if (match(TK_FOR)) {
        for_statement();
    } else if (match(TK_IF)) {
        if_statement();
    } else if (match(TK_RETURN)) {
        return_statement();
    } else if (match(TK_WHILE)) {
        while_statement();
    } else if (match(TK_LBRACE)) {
        begin_scope();
        block();
        end_scope();
    } else {
        expression_statement();
    }
}

ObjFunction *compile(const char *source)
{
    init_scanner(source);

    Compiler compiler;
    init_compiler(&compiler, TYPE_SCRIPT);

    parser.had_error = false;
    parser.panic_mode = false;

    advance();
    while (!match(TK_EOF)) {
        declaration();
    }

    ObjFunction *function = end_compiler();
    return parser.had_error ? NULL : function;
}

void mark_compiler_roots()
{
    Compiler *compiler = current;
    while (compiler != NULL) {
        mark_object((Obj *)compiler->function);
        compiler = compiler->enclosing;
    }
}