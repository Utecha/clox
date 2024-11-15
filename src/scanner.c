#include <string.h>
#include "include/common.h"
#include "include/scanner.h"

typedef struct {
    const char *start;
    const char *current;
    int line;
} Scanner;

Scanner scanner;

void init_scanner(const char *source)
{
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}

static Token make_token(TokenType type)
{
    Token token;

    token.type = type;
    token.start = scanner.start;
    token.len = (int)(scanner.current - scanner.start);
    token.line = scanner.line;

    return token;
}

static Token error_token(const char *message)
{
    Token token;

    token.type = TK_ERROR;
    token.start = message;
    token.len = (int)strlen(message);
    token.line = scanner.line;

    return token;
}

static bool is_alpha(char c)
{
    return (c >= 'a' && c <= 'z') ||
           (c >= 'A' && c <= 'Z') ||
            c == '_';
}

static bool is_digit(char c)
{
    return c >= '0' && c <= '9';
}

static bool is_alnum(char c)
{
    return is_alpha(c) || is_digit(c);
}

static bool scan_eof()
{
    return *scanner.current == '\0';
}

static bool match(char expected)
{
    if (scan_eof()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

static char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static char peek()
{
    return *scanner.current;
}

static char peek_next()
{
    if (scan_eof()) return '\0';
    return scanner.current[1];
}

void skip_whitespace()
{
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ':
            case '\r':
            case '\t': {
                advance();
            } break;
            case '\n': {
                scanner.line++;
                advance();
            } break;
            case '/': {
                if (match('/')) {
                    while (peek() != '\n' && !scan_eof()) advance();
                } else {
                    return;
                }
            } break;
            default:
                return;
        }
    }
}

TokenType check_keyword(int start, int len, char *rest, TokenType type)
{
    int lens_match = (scanner.current - scanner.start) == (start + len);
    bool mem_equal = memcmp(scanner.start + start, rest, len) == 0;

    if (lens_match && mem_equal) return type;
    return TK_IDENTIFIER;
}

TokenType identifier_type()
{
    switch (scanner.start[0]) {
        case 'a': return check_keyword(1, 2, "nd", TK_AND);
        case 'c': return check_keyword(1, 4, "lass", TK_CLASS);
        case 'e': return check_keyword(1, 3, "lse", TK_ELSE);
        case 'f': {
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'a': return check_keyword(2, 3, "lse", TK_FALSE);
                    case 'o': return check_keyword(2, 1, "r", TK_FOR);
                    case 'u': return check_keyword(2, 1, "n", TK_FUN);
                }
            }
        } break;
        case 'i': return check_keyword(1, 1, "f", TK_IF);
        case 'n': return check_keyword(1, 2, "il", TK_NIL);
        case 'o': return check_keyword(1, 1, "r", TK_OR);
        case 'p': return check_keyword(1, 4, "rint", TK_PRINT);
        case 'r': return check_keyword(1, 5, "eturn", TK_RETURN);
        case 's': return check_keyword(1, 4, "uper", TK_SUPER);
        case 't': {
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return check_keyword(2, 2, "is", TK_THIS);
                    case 'r': return check_keyword(2, 2, "ue", TK_TRUE);
                }
            }
        } break;
        case 'v': return check_keyword(1, 2, "ar", TK_VAR);
        case 'w': return check_keyword(1, 4, "hile", TK_WHILE);
    }
    return TK_IDENTIFIER;
}

Token identifier()
{
    while (is_alnum(peek())) advance();
    return make_token(identifier_type());
}

Token number()
{
    while (is_digit(peek())) advance();

    if (peek() == '.' && is_digit(peek_next())) {
        advance();
        while (is_digit(peek())) advance();
    }

    return make_token(TK_NUMBER);
}

Token string()
{
    while (peek() != '"' && !scan_eof()) {
        if (peek() == '\n') scanner.line++;
        advance();
    }

    if (scan_eof()) return error_token("Unterminated string");

    advance();
    return make_token(TK_STRING);
}

Token scan_token()
{
    skip_whitespace();

    scanner.start = scanner.current;
    if (scan_eof()) return make_token(TK_EOF);

    char c = advance();
    if (is_alpha(c)) return identifier();
    if (is_digit(c)) return number();

    switch (c)
    {
        case '(': return make_token(TK_LPAREN);
        case ')': return make_token(TK_RPAREN);
        case '{': return make_token(TK_LBRACE);
        case '}': return make_token(TK_RBRACE);
        case ',': return make_token(TK_COMMA);
        case '.': return make_token(TK_DOT);
        case ';': return make_token(TK_SEMICOLON);
        case '-': return make_token(TK_MINUS);
        case '+': return make_token(TK_PLUS);
        case '/': return make_token(TK_SLASH);
        case '*': return make_token(TK_STAR);
        case '!': return make_token(match('=') ? TK_NOTEQ : TK_NOT);
        case '=': return make_token(match('=') ? TK_CMPEQ : TK_EQUAL);
        case '>': return make_token(match('=') ? TK_GTEQ : TK_GT);
        case '<': return make_token(match('=') ? TK_LTEQ : TK_LT);
        case '"': return string();
    }

    return error_token("Unexpected character");
}