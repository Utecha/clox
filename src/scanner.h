#ifndef lox_scanner_h
#define lox_scanner_h

typedef enum
{
    // Special
    TK_ERROR,
    TK_EOF,

    // Delimiter
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE,
    TK_COMMA,
    TK_DOT,
    TK_SEMICOLON,

    // Assignment
    TK_EQUAL,

    // Logical
    TK_NOT,

    // Equality
    TK_NOTEQ,
    TK_EQEQ,

    // Comparison
    TK_GT,
    TK_GTEQ,
    TK_LT,
    TK_LTEQ,

    // Arithmetic
    TK_MINUS,
    TK_PLUS,
    TK_SLASH,
    TK_STAR,

    // Literals
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,

    // Keywords
    TK_AND,
    TK_CLASS,
    TK_ELSE,
    TK_FALSE,
    TK_FOR,
    TK_FUN,
    TK_IF,
    TK_NIL,
    TK_OR,
    TK_PRINT,
    TK_RETURN,
    TK_SUPER,
    TK_THIS,
    TK_TRUE,
    TK_VAR,
    TK_WHILE,
} TokenType;

typedef struct
{
    TokenType type;
    const char *start;
    int length;
    int line;
} Token;

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Scanner;

void init_scanner(Scanner *scanner, const char *source);
Token scan_token(Scanner *scanner);

#endif // lox_scanner_h
