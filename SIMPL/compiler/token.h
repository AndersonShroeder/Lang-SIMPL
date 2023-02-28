#ifndef TOKEN_H
#define TOKEN_H

class Lexer;

enum TokenType
{
  // Single-character tokens.
  T_LPAREN,
  T_RPAREN,
  T_LBRACE,
  T_RBRACE,
  T_COMMA,
  T_DOT,
  T_MINUS,
  T_PLUS,
  T_SEMICOLON,
  T_SLASH,
  T_STAR,
  // One or two character tokens.
  T_NOT,
  T_DNOTE,
  T_EQ,
  T_EQUIV,
  T_GRT,
  T_GRTEQ,
  T_LSS,
  T_LSSEQ,
  // Literals.
  T_ID,
  T_STR,
  T_NUM,
  // Keywords.
  T_AND,
  T_CLASS,
  T_ELSE,
  T_FALSE,
  T_FOR,
  T_FUN,
  T_IF,
  T_NIL,
  T_OR,
  T_PRINT,
  T_RETURN,
  T_SUPER,
  T_THIS,
  T_TRUE,
  T_VAR,
  T_WHILE,

  T_ERROR,
  T_EOF
};

class Token
{
public:
  TokenType type;
  const char *start;
  int length;
  int line;

  Token(){};
  Token(TokenType type, Lexer *lex, const char *message, bool error = false);
};

#endif