#include <string.h>

#include "common.h"
#include "scanner.h"

struct Keyword
{
    const char *lexeme;
    int length;
    TokenType type;
};

static const struct Keyword keywords[] = {
    { "and", 3, TK_AND },
    { "class", 5, TK_CLASS },
    { "else", 4, TK_ELSE },
    { "false", 5, TK_FALSE },
    { "for", 3, TK_FOR },
    { "fun", 3, TK_FUN },
    { "if", 2, TK_IF },
    { "nil", 3, TK_NIL },
    { "or", 2, TK_OR },
    { "return", 6, TK_RETURN },
    { "super", 5, TK_SUPER },
    { "this", 4, TK_THIS },
    { "true", 4, TK_TRUE },
    { "var", 3, TK_VAR },
    { "while", 5, TK_WHILE },
    { NULL, 0, TK_EOF },  // Sentinel value to mark end
};

void init_scanner(Scanner *scanner, const char *source)
{
    scanner->start = source;
    scanner->current = source;
    scanner->line = 1;
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

static bool at_end(Scanner *scanner)
{
    return *scanner->current == '\0';
}

static char peek(Scanner *scanner)
{
    return *scanner->current;
}

static char peek_next(Scanner *scanner)
{
    if (at_end(scanner)) return '\0';
    return *(scanner->current + 1);
}

static char advance(Scanner *scanner)
{
    return *scanner->current++;
}

static bool match(Scanner *scanner, char c)
{
    if (peek(scanner) != c) return false;
    scanner->current++;
    return true;
}

static Token make_token(Scanner *scanner, TokenType type)
{
    Token token;
    token.type = type;
    token.start = scanner->start;
    token.length = (int)(scanner->current - scanner->start);
    token.line = scanner->line;
    return token;
}

static Token two_char_token(Scanner *scanner, char c, TokenType two, TokenType one)
{
    return make_token(scanner, match(scanner, c) ? two : one);
}

static Token error_token(Scanner *scanner, const char *msg)
{
    Token token;
    token.type = TK_ERROR;
    token.start = msg;
    token.length = (int)strlen(msg);
    token.line = scanner->line;
    return token;
}

static void skip_whitespace(Scanner *scanner)
{
    for (;;)
    {
        char c = peek(scanner);
        switch (c)
        {
            case ' ':
            case '\r':
            case '\t':
            {
                advance(scanner);
            }   break;
            case '\n':
            {
                scanner->line++;
                advance(scanner);
            } break;
            case '/':
            {
                if (peek_next(scanner) == '/')
                {
                    while (peek(scanner) != '\n' && !at_end(scanner))
                    {
                        advance(scanner);
                    }
                }
                else
                {
                    return;
                }
            } break;
            default:
                return;
        }
    }
}

static Token make_identifier_token(Scanner *scanner)
{
    while (is_alnum(peek(scanner))) advance(scanner);

    TokenType type = TK_IDENTIFIER;
    int length = (int)(scanner->current - scanner->start);

    for (int i = 0; keywords[i].lexeme != NULL; i++)
    {
        if (length == keywords[i].length &&
            memcmp(scanner->start, keywords[i].lexeme, length) == 0)
        {
            type = keywords[i].type;
            break;
        }
    }

    return make_token(scanner, type);
}

static Token make_number_token(Scanner *scanner)
{
    while (is_digit(peek(scanner))) advance(scanner);

    if (peek(scanner) == '.' && is_digit(peek_next(scanner)))
    {
        advance(scanner);
        while (is_digit(peek(scanner))) advance(scanner);
    }

    return make_token(scanner, TK_NUMBER);
}

static Token make_string_token(Scanner *scanner)
{
    while (peek(scanner) != '"' && !at_end(scanner))
    {
        if (peek(scanner) == '\n') scanner->line++;
        advance(scanner);
    }

    if (at_end(scanner)) return error_token(scanner, "Unterminated string");

    advance(scanner); // Eat the closing '"'
    return make_token(scanner, TK_STRING);
}

Token scan_token(Scanner *scanner)
{
    skip_whitespace(scanner);
    scanner->start = scanner->current;

    if (at_end(scanner)) return make_token(scanner, TK_EOF);

    char c = advance(scanner);
    if (is_alpha(c)) return make_identifier_token(scanner);
    if (is_digit(c)) return make_number_token(scanner);

    switch (c)
    {
        case '(':   return make_token(scanner, TK_LPAREN);
        case ')':   return make_token(scanner, TK_RPAREN);
        case '{':   return make_token(scanner, TK_LBRACE);
        case '}':   return make_token(scanner, TK_RBRACE);
        case ',':   return make_token(scanner, TK_COMMA);
        case '.':   return make_token(scanner, TK_DOT);
        case ';':   return make_token(scanner, TK_SEMICOLON);
        case '-':   return make_token(scanner, TK_MINUS);
        case '+':   return make_token(scanner, TK_PLUS);
        case '/':   return make_token(scanner, TK_SLASH);
        case '*':   return make_token(scanner, TK_STAR);
        case '!':   return two_char_token(scanner, '=', TK_NOTEQ, TK_NOT);
        case '=':   return two_char_token(scanner, '=', TK_EQEQ, TK_EQUAL);
        case '>':   return two_char_token(scanner, '=', TK_GTEQ, TK_GT);
        case '<':   return two_char_token(scanner, '=', TK_LTEQ, TK_LT);
        case '"':   return make_string_token(scanner);
    }

    return error_token(scanner, "Unexpected character");
}
