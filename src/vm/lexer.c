#include <string.h>
#include "common.h"
#include "lexer.h"

typedef struct
{
    const char *identifier;
    size_t length;
    TokenType type;
} Keyword;

static Keyword keywords[] = {
    { "and", 3, TK_AND },
    { "class", 5, TK_CLASS },
    { "const", 5, TK_CONST },
    { "else", 4, TK_ELSE },
    { "false", 5, TK_FALSE },
    { "for", 3, TK_FOR },
    { "fun", 3, TK_FUN },
    { "if", 2, TK_IF },
    { "nil", 3, TK_NIL },
    { "or", 2, TK_OR },
    { "print", 5, TK_PRINT },
    { "return", 6, TK_RETURN },
    { "super", 5, TK_SUPER },
    { "this", 4, TK_THIS },
    { "true", 4, TK_TRUE },
    { "var", 3, TK_VAR },
    { "while", 5, TK_WHILE },
};

static inline bool isLower(char c)
{
    return (c >= 'a' && c <= 'z');
}

static inline bool isUpper(char c)
{
    return (c >= 'A' && c <= 'Z');
}

static inline bool isAlpha(char c)
{
    return isLower(c) || isUpper(c);
}

static inline bool isDigit(char c)
{
    return (c >= '0' && c <= '9');
}

static inline bool isAlnum(char c)
{
    return isAlpha(c) || isDigit(c);
}

static inline bool isIdent(char c)
{
    return isAlnum(c) || c == '_';
}

void initLexer(Lexer *lexer, const char *source)
{
    lexer->start = source;
    lexer->current = source;
    lexer->line = 1;
}

static bool isAtEnd(Lexer *lexer)
{
    return *lexer->current == '\0';
}

static char peek(Lexer *lexer)
{
    return *lexer->current;
}

static char peekNext(Lexer *lexer)
{
    if (isAtEnd(lexer)) return '\0';
    return *(lexer->current + 1);
}

static char advance(Lexer *lexer)
{
    return *lexer->current++;
}

static bool match(Lexer *lexer, char c)
{
    if (peek(lexer) != c) return false;
    lexer->current++;
    return true;
}

static void skipLnComment(Lexer *lexer)
{
    while (peek(lexer) != '\n' && !isAtEnd(lexer))
        { advance(lexer); }
}

static void skipWhitespace(Lexer *lexer)
{
    for (;;)
    {
        char c = peek(lexer);
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
                advance(lexer);
                break;
            case '\n':
            {
                lexer->line++;
                advance(lexer);
            } break;
            case '/':
            {
                if (peekNext(lexer) == '/')
                    { skipLnComment(lexer); }
                else
                    { return; }
            } break;
            default:
                return;
        }
    }
}

static Token makeToken(Lexer *lexer, TokenType type)
{
    Token token;
    token.type = type;
    token.lexeme = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    return token;
}

static Token twoCharToken(Lexer *lexer, char c, TokenType two, TokenType one)
{
    return makeToken(lexer, match(lexer, c) ? two : one);
}

static Token errorToken(Lexer *lexer, const char *message)
{
    Token token;
    token.type = TK_ERROR;
    token.lexeme = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    return token;
}

static Token identifier(Lexer *lexer)
{
    while (isIdent(peek(lexer))) advance(lexer);

    TokenType type = TK_IDENTIFIER;
    size_t length = lexer->current - lexer->start;

    for (int i = 0; keywords[i].identifier != NULL; i++)
    {
        if (length == keywords[i].length &&
            memcmp(lexer->start, keywords[i].identifier, length) == 0)
        {
            type = keywords[i].type;
            break;
        }
    }

    return makeToken(lexer, type);
}

static Token number(Lexer *lexer)
{
    while (isDigit(peek(lexer))) advance(lexer);

    if (peek(lexer) == '.' && isDigit(peekNext(lexer)))
    {
        advance(lexer);
        while (isDigit(peek(lexer))) advance(lexer);
    }

    return makeToken(lexer, TK_NUMBER);
}

static Token string(Lexer *lexer)
{
    while (peek(lexer) != '"' && !isAtEnd(lexer))
    {
        if (peek(lexer) == '\n') lexer->line++;
        advance(lexer);
    }

    if (isAtEnd(lexer))
        { return errorToken(lexer, "Unterminated string"); }

    advance(lexer); // The closing quote
    return makeToken(lexer, TK_STRING);
}

Token getToken(Lexer *lexer)
{
    skipWhitespace(lexer);
    lexer->start = lexer->current;

    if (isAtEnd(lexer)) return makeToken(lexer, TK_EOF);

    char c = advance(lexer);
    if (isAlpha(c)) return identifier(lexer);
    if (isDigit(c)) return number(lexer);

    switch (c)
    {
        case '(':   return makeToken(lexer, TK_LPAREN);
        case ')':   return makeToken(lexer, TK_RPAREN);
        case '{':   return makeToken(lexer, TK_LBRACE);
        case '}':   return makeToken(lexer, TK_RBRACE);
        case ',':   return makeToken(lexer, TK_COMMA);
        case '.':   return makeToken(lexer, TK_DOT);
        case ';':   return makeToken(lexer, TK_SEMICOLON);
        case ':':   return makeToken(lexer, TK_COLON);
        case '?':   return makeToken(lexer, TK_QUESTION);
        case '-':   return makeToken(lexer, TK_MINUS);
        case '+':   return makeToken(lexer, TK_PLUS);
        case '/':   return makeToken(lexer, TK_SLASH);
        case '*':   return makeToken(lexer, TK_STAR);
        case '!':   return twoCharToken(lexer, '=', TK_BANGEQ, TK_BANG);
        case '=':   return twoCharToken(lexer, '=', TK_EQEQ, TK_EQUAL);
        case '>':   return twoCharToken(lexer, '=', TK_GTEQ, TK_GT);
        case '<':   return twoCharToken(lexer, '=', TK_LTEQ, TK_LT);
        case '"':   return string(lexer);
    }

    return errorToken(lexer, "Unexpected character");
}
