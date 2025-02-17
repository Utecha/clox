#ifndef CLOX_SCANNER_H
#define CLOX_SCANNER_H

typedef enum {
    /* Delimiter */
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE,
    TK_COMMA,
    TK_DOT,
    TK_SEMICOLON,

    /* Arithmetic */
    TK_MINUS,
    TK_PLUS,
    TK_SLASH,
    TK_STAR,

    /* Equality/Comparison */
    TK_NOT,
    TK_NOTEQ,
    TK_EQUAL,
    TK_CMPEQ,
    TK_GT,
    TK_GTEQ,
    TK_LT,
    TK_LTEQ,

    /* Literals */
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,

    /* Keywords */
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

    /* Special */
    TK_ERROR,
    TK_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    int len;
    int line;
} Token;

void init_scanner(const char *source);
Token scan_token();

#endif // CLOX_SCANNER_H