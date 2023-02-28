#include <stdio.h>
#include <stdlib.h>
#include ".\SIMPL\common.h"
#include "compiler.h"
#include "lexer.h"
#include ".\SIMPL\vm\vm.h"
#include ".\SIMPL\object.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

Parser parser;
Lexer lex;
Chunk *compilingChunk;

static Chunk *currentChunk()
{
    return compilingChunk;
}

// Prints where error occured
static void errorAt(Token *token, const char *message)
{
    // If we have seen a previous error we ignore future errors when compile
    if (parser.panicMode)
        return;

    parser.panicMode = true;

    fprintf(stderr, "[line %d] Error", token->line);

    if (token->type == T_EOF)
    {
        fprintf(stderr, " at end");
    }

    else if (token->type == T_ERROR)
    {
    }

    else
    {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

static void error(const char *message)
{
    errorAt(&parser.previous, message);
}

static void errorAtCurrent(const char *message)
{
    errorAt(&parser.current, message);
}

static void advance(Lexer &lexer)
{
    parser.previous = parser.current;

    for (;;)
    {
        parser.current = lexer.scanToken();
        if (parser.current.type != T_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

static void consume(TokenType type, const char *message)
{
    if (parser.current.type == type)
    {
        advance(lex);
        return;
    }

    errorAtCurrent(message);
}

static bool check(TokenType type)
{
    return parser.current.type == type;
}

// If a token has type consume the token and move on, otherwise leave it
static bool match(TokenType type, Lexer &lex)
{
    if (!check(type))
        return false;
    advance(lex);
    return true;
}

static void emitByte(uint8_t byte)
{
    currentChunk()->write_chunk(byte, parser.previous.line);
}

static void emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn()
{
    emitByte(OP_RETURN);
}

static uint8_t makeConstant(Value value)
{
    int constant = currentChunk()->add_constant(value);
    if (constant > UINT8_MAX)
    {
        error("Too may consants in one chunk.");
        return 0;
    }

    return uint8_t(constant);
}

static void emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        disassembleChunk(currentChunk(), "code");
    }
#endif
}

static void expression();
static void statement(Lexer &lex);
static void declaration(Lexer &lex);
static uint8_t identifierConstant(Token* name);
static ParseRule *getRule(TokenType type);
static void parsePrecedence(Precedence precedence);

static void binary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = getRule(operatorType);
    parsePrecedence((Precedence)(rule->precedence + 1));

    switch (operatorType)
    {
    // !=, >=, <= output two bytes -> one that is the opposite of the operation (=, <, >) and a negation to flip it
    case T_DNOTE:
        emitBytes(OP_EQUAL, OP_NOT);
        break;
    case T_EQUIV:
        emitByte(OP_EQUAL);
        break;
    case T_GRT:
        emitByte(OP_GREATER);
        break;
    case T_GRTEQ:
        emitBytes(OP_LESS, OP_NOT);
        break;
    case T_LSS:
        emitByte(OP_LESS);
        break;
    case T_LSSEQ:
        emitBytes(OP_GREATER, OP_NOT);
        break;
    case T_PLUS:
        emitByte(OP_ADD);
        break;
    case T_MINUS:
        emitByte(OP_SUBTRACT);
        break;
    case T_STAR:
        emitByte(OP_MULTIPLY);
        break;
    case T_SLASH:
        emitByte(OP_DIVIDE);
        break;
    default:
        return;
    }
}

static void literal(bool canAssign)
{
    switch (parser.previous.type)
    {
    case T_FALSE:
        emitByte(OP_FALSE);
        break;
    case T_NIL:
        emitByte(OP_NIL);
        break;
    case T_TRUE:
        emitByte(OP_TRUE);
        break;
    default:
        return;
    }
}

// Parens
static void grouping(bool canAssign)
{
    expression();
    consume(T_RPAREN, "Expect ')' after expression.");
}

static void number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// Creates a string object and wraps it in Value then adds to constant table
static void string(bool canAssign)
{
    // +1 and -2 trim the leading and ending qoutation marks
    emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

static void namedVariable(Token name, bool canAssign)
{
    uint8_t arg = identifierConstant(&name);
    
    if (canAssign && match(T_EQ, lex))
    {
        expression();
        emitBytes(OP_SET_GLOBAL, arg);
    }
    else emitBytes(OP_GET_GLOBAL, arg);
}

static void variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

// Unary Negation
static void unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    // compile operand
    parsePrecedence(P_UNARY);

    switch (operatorType)
    {
    case T_NOT:
        emitByte(OP_NOT);
        break;
    case T_MINUS:
        emitByte(OP_NEGATE);
        break;
    default:
        return;
    }
}

ParseRule rules[] = {
    rules[T_LPAREN] = {grouping, NULL, P_NONE},
    rules[T_RPAREN] = {NULL, NULL, P_NONE},
    rules[T_LBRACE] = {NULL, NULL, P_NONE},
    rules[T_RBRACE] = {NULL, NULL, P_NONE},
    rules[T_COMMA] = {NULL, NULL, P_NONE},
    rules[T_DOT] = {NULL, NULL, P_NONE},
    rules[T_MINUS] = {unary, binary, P_TERM},
    rules[T_PLUS] = {NULL, binary, P_TERM},
    rules[T_SEMICOLON] = {NULL, NULL, P_NONE},
    rules[T_SLASH] = {NULL, binary, P_FACTOR},
    rules[T_STAR] = {NULL, binary, P_FACTOR},
    rules[T_NOT] = {unary, NULL, P_NONE},
    rules[T_DNOTE] = {NULL, binary, P_EQUALITY},
    rules[T_EQ] = {NULL, NULL, P_NONE},
    rules[T_EQUIV] = {NULL, binary, P_EQUALITY},
    rules[T_GRT] = {NULL, binary, P_COMPARISON},
    rules[T_GRTEQ] = {NULL, binary, P_COMPARISON},
    rules[T_LSS] = {NULL, binary, P_COMPARISON},
    rules[T_LSSEQ] = {NULL, binary, P_COMPARISON},
    rules[T_ID] = {variable, NULL, P_NONE},
    rules[T_STR] = {string, NULL, P_NONE},
    rules[T_NUM] = {number, NULL, P_NONE},
    rules[T_AND] = {NULL, NULL, P_NONE},
    rules[T_CLASS] = {NULL, NULL, P_NONE},
    rules[T_ELSE] = {NULL, NULL, P_NONE},
    rules[T_FALSE] = {literal, NULL, P_NONE},
    rules[T_FOR] = {NULL, NULL, P_NONE},
    rules[T_FUN] = {NULL, NULL, P_NONE},
    rules[T_IF] = {NULL, NULL, P_NONE},
    rules[T_NIL] = {literal, NULL, P_NONE},
    rules[T_OR] = {NULL, NULL, P_NONE},
    rules[T_PRINT] = {NULL, NULL, P_NONE},
    rules[T_RETURN] = {NULL, NULL, P_NONE},
    rules[T_SUPER] = {NULL, NULL, P_NONE},
    rules[T_THIS] = {NULL, NULL, P_NONE},
    rules[T_TRUE] = {literal, NULL, P_NONE},
    rules[T_VAR] = {NULL, NULL, P_NONE},
    rules[T_WHILE] = {NULL, NULL, P_NONE},
    rules[T_ERROR] = {NULL, NULL, P_NONE},
    rules[T_EOF] = {NULL, NULL, P_NONE},
};

static void parsePrecedence(Precedence precedence)
{
    advance(lex);
    ParseFn prefixRule = getRule(parser.previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= P_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= getRule(parser.current.type)->precedence)
    {
        advance(lex);
        ParseFn infixRule = getRule(parser.previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(T_EQ, lex))
    {
        error("Invalid assignment target.");
    }
}

// returns index of variable name as a string in chunk -> a string name is too big for
// byte code stream which is why we access using the index of type uint8_t
static uint8_t identifierConstant(Token* name)
{
    return makeConstant(OBJ_VAL(copyString(name->start, name->length)));
}

static uint8_t parseVariable(const char* errorMessage)
{
    consume(T_ID, errorMessage);
    return identifierConstant(&parser.previous);
}

static void defineVariable(uint8_t global)
{
    emitBytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule *getRule(TokenType type)
{
    return &rules[type];
}

static void expression()
{
    parsePrecedence(P_ASSIGNMENT);
}

static void varDeclaration(Lexer &lex)
{
    uint8_t global = parseVariable("Expect variable name.");

    // if there is assignment var gets that expression result, else the var value is init to nil
    if (match(T_EQ, lex)) expression();
    else emitByte(OP_NIL);

    consume(T_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

static void expressionStatement()
{
    expression();
    consume(T_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

static void printStatement()
{
    expression();
    consume(T_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

static void synchronize(Lexer &lex)
{
    parser.panicMode = false;

    // skips everything until we reach something that indicates the end of a statement
    while (parser.current.type != T_EOF)
    {
        if (parser.previous.type == T_SEMICOLON)
            return;
        switch (parser.current.type)
        {
        case T_CLASS:
        case T_FUN:
        case T_VAR:
        case T_FOR:
        case T_IF:
        case T_WHILE:
        case T_PRINT:
        case T_RETURN:
            return;

        default:; // Do nothing.
        }

        advance(lex);
    }
}

static void declaration(Lexer &lex)
{
    if (match(T_VAR, lex))
    {
        varDeclaration(lex);
    }
    else
    {
        statement(lex);
    }

    // if we hit an error in previous statement we resynch code
    if (parser.panicMode)
        synchronize(lex);
}

static void statement(Lexer &lex)
{
    if (match(T_PRINT, lex))
    {
        printStatement();
    }
    else
    {
        expressionStatement();
    }
}

bool compile(const char *source, Chunk *chunk)
{

    compilingChunk = chunk;
    lex = Lexer(source);

    parser.hadError = false;
    parser.panicMode = false;

    advance(lex);
    while (!match(T_EOF, lex))
    {
        declaration(lex);
    }
    endCompiler();
    return !parser.hadError;
}