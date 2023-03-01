#include "compiler.hh"
#include "lexer.hh"
#include "vm.hh"
#include "object.hh"
#include "bytecodes.hh"

#ifdef DEBUG_PRINT_CODE
#include "debug.hh"
#endif

using namespace compileTools;
using namespace parseTools;

ByteArray *Compiler::currentChunk()
{
    return compilingChunk;
}

//////////////////////////////////////////////////
// PARSER
//////////////////////////////////////////////////

// Prints where error occured
void Parser::errorAt(Token token, const char *message)
{
    // If we have seen a previous error we ignore future errors when compile
    if (panicMode)
        return;

    panicMode = true;

    fprintf(stderr, "[line %d] Error", token.line);

    if (token.type == T_EOF)
    {
        fprintf(stderr, " at end");
    }

    else if (token.type == T_ERROR)
    {
    }

    else
    {
        fprintf(stderr, " at '%.*s'", token.length, token.start);
    }

    fprintf(stderr, ": %s\n", message);
    hadError = true;
}

void Parser::error(const char *message)
{
    errorAt(previous, message);
}

void Parser::errorAtCurrent(const char *message)
{
    errorAt(current, message);
}

void Parser::advance()
{
    previous = current;

    for (;;)
    {
        current = lexer.scanToken();
        if (current.type != T_ERROR)
            break;
        errorAtCurrent(current.start);
    }
}

void Parser::consume(TokenType type, const char *message)
{
    if (current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

bool Parser::check(TokenType type)
{
    return current.type == type;
}

// If a token has type consume the token and move on, otherwise leave it
bool Parser::match(TokenType type)
{
    if (!check(type))
        return false;

    advance();
    return true;
}


ParseRule *Compiler::Compiler::getRule(TokenType type)
{
    return &rules[type];
}

void Parser::parsePrecedence(Precedence precedence)
{
    advance();
    ParseFn prefixRule = Compiler::Compiler::getRule(previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= P_ASSIGNMENT;
    prefixRule(canAssign);

    while (precedence <= Compiler::getRule(current.type)->precedence)
    {
        advance();
        ParseFn infixRule = Compiler::getRule(previous.type)->infix;
        infixRule(canAssign);
    }

    if (canAssign && match(T_EQ))
    {
        error("Invalid assignment target.");
    }
}

//////////////////////////////////////////////////
// Compiler
//////////////////////////////////////////////////

void Compiler::emitByte(uint8_t byte)
{
    currentChunk()->writeByte(byte, parser.previous.line);
}

void Compiler::emitBytes(uint8_t byte1, uint8_t byte2)
{
    emitByte(byte1);
    emitByte(byte2);
}

void Compiler::emitReturn()
{
    emitByte(OP_RETURN);
}

uint8_t Compiler::makeConstant(Value value)
{
    int constant = currentChunk()->addConstant(value);
    if (constant > UINT8_MAX)
    {
        parser.error("Too may consants in one chunk.");
        return 0;
    }

    return uint8_t(constant);
}

void Compiler::emitConstant(Value value)
{
    emitBytes(OP_CONSTANT, makeConstant(value));
}

void Compiler::endCompiler()
{
    emitReturn();
#ifdef DEBUG_PRINT_CODE
    if (!parser.hadError)
    {
        Disassembler debug = Disassembler(currentChunk(), "Code");
        debug.disassembleByteArray();
    }
#endif
}

void Compiler::binary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;
    ParseRule *rule = Compiler::getRule(operatorType);
    parser.parsePrecedence((Precedence)(rule->precedence + 1));

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

void Compiler::literal(bool canAssign)
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
void Compiler::grouping(bool canAssign)
{
    expression();
    parser.consume(T_RPAREN, "Expect ')' after expression.");
}

void Compiler::number(bool canAssign)
{
    double value = strtod(parser.previous.start, NULL);
    emitConstant(NUMBER_VAL(value));
}

// // Creates a string object and wraps it in Value then adds to constant table
void Compiler::string(bool canAssign)
{
    // // +1 and -2 trim the leading and ending qoutation marks
    // emitConstant(OBJ_VAL(copyString(parser.previous.start + 1, parser.previous.length - 2)));
}

void Compiler::namedVariable(Token name, bool canAssign)
{
    // uint8_t arg = identifierConstant(name);

    // if (canAssign && parser.match(T_EQ))
    // {
    //     expression();
    //     emitBytes(OP_SET_GLOBAL, arg);
    // }
    // else
    //     emitBytes(OP_GET_GLOBAL, arg);
}

void Compiler::variable(bool canAssign)
{
    // namedVariable(parser.previous, canAssign);
}

// Unary Negation
void Compiler::unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    // compile operand
    parser.parsePrecedence(P_UNARY);

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

// returns index of variable name as a string in chunk -> a string name is too big for
// byte code stream which is why we access using the index of type uint8_t
// uint8_t Compiler::identifierConstant(Token name)
// {
//     return makeConstant(OBJ_VAL(copyString(name.start, name.length)));
// }

// uint8_t Compiler::parseVariable(const char *errorMessage)
// {
//     parser.consume(T_ID, errorMessage);
//     return identifierConstant(parser.previous);
// }

// void Compiler::defineVariable(uint8_t global)
// {
//     emitBytes(OP_DEFINE_GLOBAL, global);
// }


void Compiler::expression()
{
    parser.parsePrecedence(P_ASSIGNMENT);
}

// void Compiler::varDeclaration()
// {
//     uint8_t global = parseVariable("Expect variable name.");

//     // if there is assignment var gets that expression result, else the var value is init to nil
//     if (parser.match(T_EQ))
//         expression();
//     else
//         emitByte(OP_NIL);

//     parser.consume(T_SEMICOLON, "Expect ';' after variable declaration.");

//     defineVariable(global);
// }

void Compiler::expressionStatement()
{
    expression();
    parser.consume(T_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::printStatement()
{
    expression();
    parser.consume(T_SEMICOLON, "Expect ';' after value.");
    emitByte(OP_PRINT);
}

void Compiler::synchronize()
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

        parser.advance();
    }
}

void Compiler::declaration()
{
    if (parser.match(T_VAR))
    {
        // varDeclaration();
    }
    else
    {
        statement();
    }

    // if we hit an error in previous statement we resynch code
    if (parser.panicMode)
        synchronize();
}

void Compiler::statement()
{
    if (parser.match(T_PRINT))
    {
        printStatement();
    }
    else
    {
        expressionStatement();
    }
}

bool Compiler::compile(ByteArray *bytearray)
{

    compilingChunk = bytearray;

    parser.hadError = false;
    parser.panicMode = false;

    parser.advance();
    while (!parser.match(T_EOF))
    {
        declaration();
    }
    endCompiler();
    return !parser.hadError;
}