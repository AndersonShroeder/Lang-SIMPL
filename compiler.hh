#ifndef simpl_compiler_h
#define simpl_compiler_h

#include "lexer.hh"
#include "bytearray.hh"
#include "common.hh"

class Compiler;

typedef void (Compiler::*ParseFn)(bool canAssign);

/**

    @brief This class/enum represents a parsing rule for a specific operator or token in a SIMPL's grammar. The ParseRule object
    contains a prefix and infix ParseFn for handling how the token is parsed in different contexts, as well as a Precedence
    enum value representing the operator's precedence relative to other operators in the grammar.
*/

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



/**

    @brief This class encapsulates utility necessary for SIMPL's recursive descent parser. The Parser object contains a Lexer
    object to read in tokens from source code, an array of ParseRule objects that describe how to parse different operators
    and tokens in the grammar, and several functions for parsing different types of statements and expressions. Despite the name, the
    compiler is generally responsible for performing the actually recursive descent.
*/

class Parser
{
public:
    Token current;
    Token previous;
    Lexer lexer;
    ParseRule rules[49];
    bool hadError;
    bool panicMode;

    Parser() {}

    Parser(const char *source)
    {
        lexer = Lexer(source);
        generateRules();
    }

    void generateRules();

    void errorAt(Token token, const char *message);

    void error(const char *message);

    void errorAtCurrent(const char *message);

    void advance();

    void consume(TokenType type, const char *message);

    bool check(TokenType type);

    bool match(TokenType type);

    void parsePrecedence(Precedence precedence, Compiler *compiler);
};



/**

    @brief This class represents the SIMPL compiler which provides functionality for compiling SIMPL source code into bytecode instructions.
    The Compiler class contains a set of methods for generating bytecode instructions from the source code, as well as a set of data structures 
    for keeping track of local and global variables during the compilation process. The Compiler is responsible for generating a ByteArray 
    object that contains the final compiled bytecode instructions, which can then be executed by the SIMPL virtual machine. 
*/

struct Local
{
    int depth;
    Token name;
};

class Compiler
{
public:

    Parser parser;
    ByteArray *compilingChunk;

    // keeps track of information for local/scoped variables during compilation
    Local locals[UINT8_COUNT];
    int localCount = 0;
    int scopeDepth = 0;

    Compiler(){}

    Compiler(const char *source)
    {
        parser = Parser(source);
    }

    ByteArray *currentChunk();

    void emitByte(uint8_t byte);

    int emitJump(uint8_t instruction);

    void emitBytes(uint8_t byte1, uint8_t byte2);

    void emitLoop(int loopStart);

    void emitReturn();

    uint8_t makeConstant(Value value);

    void emitConstant(Value value);

    void patchJump(int offset);

    void endCompiler();

    void binary(bool canAssign);

    void literal(bool canAssign);

    void grouping(bool canAssign);

    void number(bool canAssign);

    void string(bool canAssign);

    int resolveLocal(Token& name);

    void namedVariable(Token name, bool canAssign);

    void variable(bool canAssign);

    void unary(bool canAssign);

    uint8_t identifierConstant(Token name);

    void declareVariable();

    uint8_t parseVariable(const char *errorMessage);

    void defineVariable(uint8_t global);

    void and_(bool canAssign);

    void or_(bool canAssign);

    ParseRule *getRule(TokenType type);

    void expression();

    void block();

    void varDeclaration();

    void expressionStatement();

    void ifStatement();

    void printStatement();

    void whileStatement();

    void synchronize();

    void declaration();

    void statement();

    bool compile(ByteArray *bytearray);
};

#endif