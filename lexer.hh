#ifndef LEXER_H
#define LEXER_H

#include "common.hh"
#include "token.hh"

/**

    @brief The Lexer class is responsible for turning a source code string into a list of tokens,
    which can then be used by the Compiler to generate bytecode. It scans through the source code
    character by character, identifying keywords, literals, and identifiers and creating tokens for them.
    The Lexer maintains a current position within the source code and a line number. It skips whitespace
    characters and keeps track of the start and end of each token. Tokens are stored in a vector for later
    use by the Compiler. This class plays a critical role in the compilation process and is used 
    extensively by the Compiler.
    */

class Lexer
{
public:
  int line;
  const char *start;
  const char *current;
  std::vector<Token> tokens = {};

  Lexer(){}

  Lexer(const char *source);

  void skipWhitespace();

  bool isEnd();

  bool match(char expected);

  Token scanToken();

  Token string();

  Token number();

  Token identifier();

  TokenType identifierType();

  TokenType checkKeyword(int start, int length, const char *remain, TokenType type);

  char advance();

  char peek();

  char peekNext();
};

#endif