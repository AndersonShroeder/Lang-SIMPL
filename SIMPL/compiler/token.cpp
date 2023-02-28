#include "token.h"
#include "lexer.h"

Token::Token(TokenType type, Lexer *lex, const char *message, bool error)
{
    this->type = !error ? type : T_ERROR;
    this->start = !message ? lex->start : message;
    this->line = lex->line;
    this->length = !message ? (int)(lex->current - lex->start) : (int)strlen(message);
}
