#ifndef lox_lexer_h
#define lox_lexer_h

typedef enum
{
    /* Special */
    TK_ERROR,
    TK_EOF,

    /* Delimiter */
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE,
    TK_COMMA,
    TK_DOT,
    TK_SEMICOLON,

    /* Assignment */
    TK_EQUAL,

    /* Conditional */
    TK_COLON,
    TK_QUESTION,

    /* Comparison/Equality */
    TK_BANGEQ,
    TK_EQEQ,
    TK_GT,
    TK_GTEQ,
    TK_LT,
    TK_LTEQ,

    /* Term */
    TK_MINUS,
    TK_PLUS,

    /* Factor */
    TK_SLASH,
    TK_STAR,

    /* Logical NOT */
    TK_BANG,

    /* Literals */
    TK_IDENTIFIER,
    TK_NUMBER,
    TK_STRING,

    /* Keyword */
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

    /* Count */
    TK_COUNT,
} TokenType;

typedef struct
{
    TokenType type;
    const char *lexeme;
    int length;
    int line;
} Token;

typedef struct
{
    const char *start;
    const char *current;
    int line;
} Lexer;

void initLexer(Lexer *lexer, const char *source);
Token getToken(Lexer *lexer);

#endif // lox_lexer_h
