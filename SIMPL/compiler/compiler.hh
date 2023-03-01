#ifndef simpl_compiler_h
#define simpl_compiler_h

#include "lexer.hh"
#include "../bytearray/bytearray.hh"
#include "../common.hh"

typedef void (*ParseFn)(bool canAssign);

namespace parseTools
{
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
        Token *current;
        Token *previous;
        Lexer lexer;

        Parser() {}

        Parser(const char *source)
        {
            lexer = Lexer(source);
        }

        // hadError and panicMode get reset after every statement.
        bool hadError;

        // allows us to know if we need to resynch code after an error is spotted
        bool panicMode;

        // Prints where error occured
        void errorAt(Token *token, const char *message);

        void error(const char *message);

        void errorAtCurrent(const char *message);

        void advance();

        void consume(TokenType type, const char *message);

        bool check(TokenType type);

        bool match(TokenType type);

        ParseRule *getRule(TokenType type);

        void parsePrecedence(Precedence precedence);
    };

};

namespace compileTools
{

    class Compiler
    {
    public:
        static parseTools::Parser parser;

        static ByteArray *compilingChunk;

        static parseTools::ParseRule rules[49];

        Compiler() {}

        Compiler(const char *source)
        {
            parser = parseTools::Parser(source);
            generateRules();
        }

        void generateRules()
        {
            rules[T_LPAREN] = {grouping, NULL, parseTools::P_NONE};
            rules[T_RPAREN] = {NULL, NULL, parseTools::P_NONE};
            rules[T_LBRACE] = {NULL, NULL, parseTools::P_NONE};
            rules[T_RBRACE] = {NULL, NULL, parseTools::P_NONE};
            rules[T_COMMA] = {NULL, NULL, parseTools::P_NONE};
            rules[T_DOT] = {NULL, NULL, parseTools::P_NONE};
            rules[T_MINUS] = {unary, binary, parseTools::P_TERM};
            rules[T_PLUS] = {NULL, binary, parseTools::P_TERM};
            rules[T_SEMICOLON] = {NULL, NULL, parseTools::P_NONE};
            rules[T_SLASH] = {NULL, binary, parseTools::P_FACTOR};
            rules[T_STAR] = {NULL, binary, parseTools::P_FACTOR};
            rules[T_NOT] = {unary, NULL, parseTools::P_NONE};
            rules[T_DNOTE] = {NULL, binary, parseTools::P_EQUALITY};
            rules[T_EQ] = {NULL, NULL, parseTools::P_NONE};
            rules[T_EQUIV] = {NULL, binary, parseTools::P_EQUALITY};
            rules[T_GRT] = {NULL, binary, parseTools::P_COMPARISON};
            rules[T_GRTEQ] = {NULL, binary, parseTools::P_COMPARISON};
            rules[T_LSS] = {NULL, binary, parseTools::P_COMPARISON};
            rules[T_LSSEQ] = {NULL, binary, parseTools::P_COMPARISON};
            rules[T_ID] = {variable, NULL, parseTools::P_NONE};
            rules[T_STR] = {string, NULL, parseTools::P_NONE};
            rules[T_NUM] = {number, NULL, parseTools::P_NONE};
            rules[T_AND] = {NULL, NULL, parseTools::P_NONE};
            rules[T_CLASS] = {NULL, NULL, parseTools::P_NONE};
            rules[T_ELSE] = {NULL, NULL, parseTools::P_NONE};
            rules[T_FALSE] = {literal, NULL, parseTools::P_NONE};
            rules[T_FOR] = {NULL, NULL, parseTools::P_NONE};
            rules[T_FUN] = {NULL, NULL, parseTools::P_NONE};
            rules[T_IF] = {NULL, NULL, parseTools::P_NONE};
            rules[T_NIL] = {literal, NULL, parseTools::P_NONE};
            rules[T_OR] = {NULL, NULL, parseTools::P_NONE};
            rules[T_PRINT] = {NULL, NULL, parseTools::P_NONE};
            rules[T_RETURN] = {NULL, NULL, parseTools::P_NONE};
            rules[T_SUPER] = {NULL, NULL, parseTools::P_NONE};
            rules[T_THIS] = {NULL, NULL, parseTools::P_NONE};
            rules[T_TRUE] = {literal, NULL, parseTools::P_NONE};
            rules[T_VAR] = {NULL, NULL, parseTools::P_NONE};
            rules[T_WHILE] = {NULL, NULL, parseTools::P_NONE};
            rules[T_ERROR] = {NULL, NULL, parseTools::P_NONE};
            rules[T_EOF] = {NULL, NULL, parseTools::P_NONE};
        }

        static ByteArray *currentChunk();

        static void emitByte(uint8_t byte);

        static void emitBytes(uint8_t byte1, uint8_t byte2);

        static void emitReturn();

        static uint8_t makeConstant(Value value);

        static void emitConstant(Value value);

        static void endCompiler();

        static void binary(bool canAssign);

        static void literal(bool canAssign);

        static void grouping(bool canAssign);

        static void number(bool canAssign);

        static void string(bool canAssign);

        static void namedVariable(Token *name, bool canAssign);

        static void variable(bool canAssign);

        static void unary(bool canAssign);

        static uint8_t identifierConstant(Token *name);

        static uint8_t parseVariable(const char *errorMessage);

        static void defineVariable(uint8_t global);

        static parseTools::ParseRule *getRule(TokenType type);

        static void expression();

        static void varDeclaration();

        static void expressionStatement();

        static void printStatement();

        static void synchronize();

        static void declaration();

        static void statement();

        static bool compile(ByteArray *bytearray);
    };
};

#endif