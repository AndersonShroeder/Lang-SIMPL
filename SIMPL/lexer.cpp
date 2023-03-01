#include "token.hh"
#include "lexer.hh"

Lexer::Lexer(const char *source)
{
    this->line = 1;
    this->start = source;
    this->current = source;
}

char Lexer::peekNext()
{
    if (isEnd())
        return '\0';
    return current[1];
}

void Lexer::skipWhitespace()
{
    while (true)
    {
        char c = peek();
        switch (c)
        {
        case ' ':
        case '\r':
        case '\t':
            advance();
            break;
        case '\n':
            line++;
            advance();
            break;
        case '/':
            // Continues skipping until reaching next line
            if (peekNext() == '/')
                while (peek() != '\n' && !isEnd())
                    advance();
            else
                return;
            break;
        default:
            return;
        }
    }
}

// Checks if reached end of input seq of characters by checking for null termination
bool Lexer::isEnd()
{
    return *current == '\0';
}

// Conditionally consumes a character if it matches an expected character
bool Lexer::match(char expected)
{
    if (isEnd())
        return false;
    if (*current != expected)
        return false;
    current++;
    return true;
}

// Looks one char ahead without consuming it
char Lexer::peek()
{
    return *current;
}

char Lexer::advance()
{
    current++;
    return current[-1];
}

TokenType Lexer::checkKeyword(int start, int length, const char *remain, TokenType type)
{
    if (current - this->start == start + length && memcmp(this->start + start, remain, length) == 0)
    {
        return type;
    }

    return T_ID;
}

TokenType Lexer::identifierType()
{
    switch (start[0])
    {
    case 'a':
        return checkKeyword(1, 2, "nd", T_AND);
    case 'c':
        return checkKeyword(1, 4, "lass", T_CLASS);
    case 'e':
        return checkKeyword(1, 3, "lse", T_ELSE);
    case 'f':
        if (current - start > 1)
        {
            switch (start[1])
            {
            case 'a':
                return checkKeyword(2, 3, "lse", T_FALSE);
            case 'o':
                return checkKeyword(2, 1, "r", T_FOR);
            case 'u':
                return checkKeyword(2, 1, "n", T_FUN);
            }
        }
        break;
    case 'i':
        return checkKeyword(1, 1, "f", T_IF);
    case 'n':
        return checkKeyword(1, 2, "il", T_NIL);
    case 'o':
        return checkKeyword(1, 1, "r", T_OR);
    case 'p':
        return checkKeyword(1, 4, "rint", T_PRINT);
    case 'r':
        return checkKeyword(1, 5, "eturn", T_RETURN);
    case 's':
        return checkKeyword(1, 4, "uper", T_SUPER);
    case 't':
        if (current - start > 1)
        {
            switch (start[1])
            {
            case 'h':
                return checkKeyword(2, 2, "is", T_THIS);
            case 'r':
                return checkKeyword(2, 2, "ue", T_TRUE);
            }
        }
        break;
    case 'v':
        return checkKeyword(1, 2, "ar", T_VAR);
    case 'w':
        return checkKeyword(1, 4, "hile", T_WHILE);
    }

    return T_ID;
}

Token Lexer::string()
{
    while (peek() != '"' && !isEnd())
    {
        // tracks line within literal
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isEnd())
        return Token(T_ERROR, this, "Unterminated string", true);

    // accounts for closing qoute
    advance();
    return Token(T_STR, this, NULL);
}

Token Lexer::number()
{
    while (isdigit(peek()))
        advance();

    // check for decimal
    if (peek() == '.' && isdigit(peekNext()))
    {
        advance();

        while (isdigit(peek()))
            advance();
    }

    return Token(T_NUM, this, NULL);
}

Token Lexer::identifier()
{
    while (isalpha(peek()) || isdigit(peek()))
        advance();
    return Token(identifierType(), this, NULL);
}

Token Lexer::scanToken()
{
    skipWhitespace();
    this->start = this->current;

    if (isEnd())
        return Token(T_EOF, this, NULL);

    char c = advance();
    if (isalpha(c))
    {
        return identifier();
    }
    if (isdigit(c))
    {
        return number();
    }

    switch (c)
    {
    // One character lexemes
    case '(':
        return Token(T_LPAREN, this, NULL);
    case ')':
        return Token(T_RPAREN, this, NULL);
    case '{':
        return Token(T_LBRACE, this, NULL);
    case '}':
        return Token(T_RBRACE, this, NULL);
    case ';':
        return Token(T_SEMICOLON, this, NULL);
    case ',':
        return Token(T_COMMA, this, NULL);
    case '.':
        return Token(T_DOT, this, NULL);
    case '-':
        return Token(T_MINUS, this, NULL);
    case '+':
        return Token(T_PLUS, this, NULL);
    case '/':
        return Token(T_SLASH, this, NULL);
    case '*':
        return Token(T_STAR, this, NULL);

    // One or two character lexemes
    case '!':
        return Token(match('=') ? T_DNOTE : T_NOT, this, NULL);
    case '=':
        return Token(match('=') ? T_EQUIV : T_EQ, this, NULL);
    case '>':
        return Token(match('=') ? T_GRTEQ : T_GRT, this, NULL);
    case '<':
        return Token(match('=') ? T_LSSEQ : T_LSS, this, NULL);

    // Literals
    case '"':
        return string();
    }

    return Token(T_ERROR, this, "Unexpected character", true);
}