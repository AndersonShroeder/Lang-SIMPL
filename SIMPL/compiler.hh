#ifndef simpl_compiler_h
#define simpl_compiler_h

#include "lexer.hh"
#include "bytearray.hh"
#include "common.hh"

class Compiler;

typedef void (Compiler::*ParseFn)(bool canAssign);

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

class ParseRule
{
public:
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;

    ParseRule(){}

    ParseRule(ParseFn prefix, ParseFn infix, Precedence precedence)
    {
        this->prefix = prefix;
        this->infix = infix;
        this->precedence = precedence;
    };
};

class Parser
{
public:
    Token current;
    Token previous;
    Lexer lexer;
    ParseRule rules[49];

    Parser() {}

    Parser(const char *source)
    {
        lexer = Lexer(source);
        generateRules();
    }

    void generateRules();
    
    // hadError and panicMode get reset after every statement.
    bool hadError;

    // allows us to know if we need to resynch code after an error is spotted
    bool panicMode;

    // Prints where error occured
    void errorAt(Token token, const char *message);

    void error(const char *message);

    void errorAtCurrent(const char *message);

    void advance();

    void consume(TokenType type, const char *message);

    bool check(TokenType type);

    bool match(TokenType type);

    void parsePrecedence(Precedence precedence, Compiler *compiler);
};


class Compiler
{
public:

    Parser parser;
    ByteArray *compilingChunk;

    Compiler(){}

    Compiler(const char *source)
    {
        parser = Parser(source);
    }

    ByteArray *currentChunk();

    void emitByte(uint8_t byte);

    void emitBytes(uint8_t byte1, uint8_t byte2);

    void emitReturn();

    uint8_t makeConstant(Value value);

    void emitConstant(Value value);

    void endCompiler();

    void binary(bool canAssign);

    void literal(bool canAssign);

    void grouping(bool canAssign);

    void number(bool canAssign);

    void string(bool canAssign);

    void namedVariable(Token name, bool canAssign);

    void variable(bool canAssign);

    void unary(bool canAssign);

    uint8_t identifierConstant(Token name);

    uint8_t parseVariable(const char *errorMessage);

    void defineVariable(uint8_t global);

    ParseRule *getRule(TokenType type);

    void expression();

    void varDeclaration();

    void expressionStatement();

    void printStatement();

    void synchronize();

    void declaration();

    void statement();

    bool compile(ByteArray *bytearray);
};

#endif