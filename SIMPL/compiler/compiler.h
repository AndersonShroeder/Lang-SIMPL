#include "lexer.h"
#include "chunk.h"
#ifndef simpl_compiler_h
#define simpl_compiler_h

class Parser
{
public:
    Token current;
    Token previous;

    // hadError and panicMode get reset after every statement.
    bool hadError;

    // allows us to know if we need to resynch code after an error is spotted
    bool panicMode;

    Parser(){};
};

// Numerically larger precedence in enum are of higher prio
enum Precedence
{
    P_NONE,
    P_ASSIGNMENT, // =
    P_OR,         // or
    P_AND,        // and
    P_EQUALITY,   // == !=
    P_COMPARISON, // < > <= >=
    P_TERM,       // + -
    P_FACTOR,     // * /
    P_UNARY,      // ! -
    P_CALL,       // . ()
    P_PRIMARY
};

typedef void (*ParseFn)(bool canAssign);

class ParseRule
{
public:
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;

    ParseRule(ParseFn prefix, ParseFn infix, Precedence precedence)
    {
        this->prefix = prefix;
        this->infix = infix;
        this->precedence = precedence;
    };
};

bool compile(const char *source, Chunk *chunk);

#endif