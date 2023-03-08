#include "compiler.hh"
#include "lexer.hh"
#include "vm.hh"
#include "object.hh"
#include "bytecodes.hh"

#ifdef DEBUG_PRINT_CODE
#include "debug.hh"
#endif

std::shared_ptr<ByteArray> Compiler::currentChunk()
{
    return compilingChunk;
}

//////////////////////////////////////////////////
// PARSER
//////////////////////////////////////////////////

/**

    Function to generate rules for different tokens and their corresponding parsing functions, precedence values.
    The rules are stored in a map named 'rules'.
    Each token type is associated with three properties:
        A pointer to the prefix parse function for the token.
        A pointer to the infix parse function for the token.
        A precedence value for the token.
    @return void
    */

void Parser::generateRules()
{
    rules[T_LPAREN] = {Compiler::grouping, NULL, P_NONE};
    rules[T_RPAREN] = {NULL, NULL, P_NONE};
    rules[T_LBRACE] = {NULL, NULL, P_NONE};
    rules[T_RBRACE] = {NULL, NULL, P_NONE};
    rules[T_COMMA] = {NULL, NULL, P_NONE};
    rules[T_DOT] = {NULL, NULL, P_NONE};
    rules[T_MINUS] = {Compiler::unary, Compiler::binary, P_TERM};
    rules[T_PLUS] = {NULL, Compiler::binary, P_TERM};
    rules[T_SEMICOLON] = {NULL, NULL, P_NONE};
    rules[T_SLASH] = {NULL, Compiler::binary, P_FACTOR};
    rules[T_STAR] = {NULL, Compiler::binary, P_FACTOR};
    rules[T_NOT] = {Compiler::unary, NULL, P_NONE};
    rules[T_DNOTE] = {NULL, Compiler::binary, P_EQUALITY};
    rules[T_EQ] = {NULL, NULL, P_NONE};
    rules[T_EQUIV] = {NULL, Compiler::binary, P_EQUALITY};
    rules[T_GRT] = {NULL, Compiler::binary, P_COMPARISON};
    rules[T_GRTEQ] = {NULL, Compiler::binary, P_COMPARISON};
    rules[T_LSS] = {NULL, Compiler::binary, P_COMPARISON};
    rules[T_LSSEQ] = {NULL, Compiler::binary, P_COMPARISON};
    rules[T_ID] = {Compiler::variable, NULL, P_NONE};
    rules[T_STR] = {Compiler::string, NULL, P_NONE};
    rules[T_NUM] = {Compiler::number, NULL, P_NONE};
    rules[T_AND] = {NULL, Compiler::and_, P_NONE};
    rules[T_CLASS] = {NULL, NULL, P_NONE};
    rules[T_ELSE] = {NULL, NULL, P_NONE};
    rules[T_FALSE] = {Compiler::literal, NULL, P_NONE};
    rules[T_FOR] = {NULL, NULL, P_NONE};
    rules[T_FUN] = {NULL, NULL, P_NONE};
    rules[T_IF] = {NULL, NULL, P_NONE};
    rules[T_NIL] = {Compiler::literal, NULL, P_NONE};
    rules[T_OR] = {NULL, Compiler::or_, P_NONE};
    rules[T_PRINT] = {NULL, NULL, P_NONE};
    rules[T_RETURN] = {NULL, NULL, P_NONE};
    rules[T_SUPER] = {NULL, NULL, P_NONE};
    rules[T_THIS] = {NULL, NULL, P_NONE};
    rules[T_TRUE] = {Compiler::literal, NULL, P_NONE};
    rules[T_VAR] = {NULL, NULL, P_NONE};
    rules[T_WHILE] = {NULL, NULL, P_NONE};
    rules[T_ERROR] = {NULL, NULL, P_NONE};
    rules[T_EOF] = {NULL, NULL, P_NONE};
}

/**

    Function to print an error message with the line number and token where the error occured.
    If 'panicMode' is true, it ignores further errors during the compilation process.
    @param token: The token where the error occured.
    @param message: The error message to be printed.
    @return void
    */

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

/**

    Function to print an error message at the previous token with the given message.
    Calls 'errorAt' function with the previous token and the given message.
    @param message: The error message to be printed.
    @return void
    */

void Parser::error(const char *message)
{
    errorAt(previous, message);
}

/**

    Function to print an error message at the current token with the given message.
    Calls 'errorAt' function with the current token and the given message.
    @param message: The error message to be printed.
    @return void
    */

void Parser::errorAtCurrent(const char *message)
{
    errorAt(current, message);
}

/**

    Function to advance the lexer to the next token and update the previous and current tokens.
    If the current token is an error token, calls 'errorAtCurrent' function with the error message.
    @return void
    */

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

/**

    Function to consume the current token if it is of the expected type, else calls 'errorAtCurrent' function
    with the given error message.
    @param type: The expected type of the token to be consumed.
    @param message: The error message to be printed if the token is not of the expected type.
    @return void
    */

void Parser::consume(TokenType type, const char *message)
{
    if (current.type == type)
    {
        advance();
        return;
    }

    errorAtCurrent(message);
}

/**

    Function to check if the current token is of the given type.
    @param type: The token type to be checked.
    @return bool: True if the current token is of the given type, false otherwise.
    */

bool Parser::check(TokenType type)
{
    return current.type == type;
}

/**

    Function to match the current token with the expected token type and advance the lexer if they match.
    If they don't match, returns false.
    @param type: The expected token type to be matched with the current token.
    @return bool: True if the current token is of the given type and the lexer is advanced, false otherwise.
    */

bool Parser::match(TokenType type)
{
    if (!check(type))
        return false;

    advance();
    return true;
}

/**

    Function to get the parse rule for a given token type from the 'rules' map of the parser.
    @param type: The token type for which the parse rule is to be obtained.
    @return ParseRule*: A pointer to the parse rule for the given token type.
    */

ParseRule *Compiler::getRule(TokenType type)
{
    return &parser.rules[type];
}

/**

    Function to parse an expression with the given precedence level using Pratt parsing algorithm.
    Advances the lexer to the next token and parses the expression recursively.
    @param precedence: The precedence level of the current expression to be parsed.
    @param compiler: A pointer to the compiler object.
    @return void
    */

void Parser::parsePrecedence(Precedence precedence, Compiler *compiler)
{
    advance();
    ParseFn prefixRule = compiler->getRule(previous.type)->prefix;
    if (prefixRule == NULL)
    {
        error("Expect expression.");
        return;
    }

    bool canAssign = precedence <= P_ASSIGNMENT;
    (compiler->*prefixRule)(canAssign);

    while (precedence <= compiler->getRule(current.type)->precedence)
    {
        advance();
        ParseFn infixRule = compiler->getRule(previous.type)->infix;
        (compiler->*infixRule)(canAssign);
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

void Compiler::emitLoop(int loopStart)
{
    emitByte(OP_LOOP);

    int offset = currentChunk()->bytes.size() - loopStart + 2;
    if (offset > UINT16_MAX)
        parser.error("Loop body too large.");

    emitByte((offset >> 8) & 0xff);
    emitByte(offset & 0xff);
}

int Compiler::emitJump(uint8_t instruction)
{
    emitByte(instruction);
    emitByte(0xff);
    emitByte(0xff);
    return currentChunk()->bytes.size() - 2;
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

void Compiler::patchJump(int offset)
{
    int jump = currentChunk()->bytes.size() - offset - 2;

    if (jump > UINT16_MAX)
    {
        parser.error("Too much to jump ober.");
    }

    currentChunk()->bytes[offset] = (jump >> 8) & 0xff;
    currentChunk()->bytes[offset + 1] = jump & 0xff;
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
    parser.parsePrecedence((Precedence)(rule->precedence + 1), this);

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
    // +1 and -2 trim the leading and ending qoutation marks
    emitConstant(OBJ_VAL(makeString(parser.previous.start + 1, parser.previous.length - 2)));
}

static bool identifiersEqual(Token* a, Token* b) {
  if (a->length != b->length) return false;
  return memcmp(a->start, b->start, a->length) == 0;
}

int Compiler::resolveLocal(Token &name)
{
    for (int i = localCount - 1; i >= 0; i--)
    {
        Local* local = &locals[i];
        if (identifiersEqual(&local->name, &name))
        {
            if (local->depth == -1)
            {
                parser.error("Can't read local variable in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

void Compiler::namedVariable(Token name, bool canAssign)
{
    uint8_t getOp, setOp;
    int arg = resolveLocal(name);
    if (arg != -1)
    {
        getOp = OP_GET_LOCAL;
        setOp = OP_SET_LOCAL;
    }
    else
    {
        arg = identifierConstant(name);
        getOp = OP_GET_GLOBAL;
        setOp = OP_SET_GLOBAL;
    }

    if (canAssign && parser.match(T_EQ))
    {
        expression();
        emitBytes(setOp, (uint8_t)arg);
    }
    else
        emitBytes(getOp, (uint8_t)arg);
}

void Compiler::variable(bool canAssign)
{
    namedVariable(parser.previous, canAssign);
}

// Unary Negation
void Compiler::unary(bool canAssign)
{
    TokenType operatorType = parser.previous.type;

    // compile operand
    parser.parsePrecedence(P_UNARY, this);

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
uint8_t Compiler::identifierConstant(Token name)
{
    return makeConstant(OBJ_VAL(makeString(name.start, name.length)));
}

void Compiler::declareVariable()
{
    // because globals are late bound simply return if not in scope
    if (scopeDepth == 0)
        return;

    Token &name = parser.previous;

    for (int i = localCount - 1; i >= 0; i--)
    {
        Local *local = &locals[i];
        if (local->depth != -1 && local->depth < scopeDepth)
        {
            break;
        }

        if (std::string(name.start) == std::string(local->name.start))
        {
            parser.error("Already a variable with this name in this scope.");
        }
    }

    if (localCount == UINT8_COUNT)
    {
        parser.error("Too many local variables in function.");
        return;
    }

    Local *local = &locals[localCount++];
    local->depth = -1;
    local->name = name;
}

uint8_t Compiler::parseVariable(const char *errorMessage)
{
    parser.consume(T_ID, errorMessage);

    declareVariable();
    if (scopeDepth > 0)
        return 0;

    return identifierConstant(parser.previous);
}

void Compiler::defineVariable(uint8_t global)
{
    if (scopeDepth > 0)
    {
        locals[localCount - 1].depth = scopeDepth;
        return;
    }

    emitBytes(OP_DEFINE_GLOBAL, global);
}

// when this is called for expr A and B, A is on top of the stack, so if A is false we skip the rest of
// B and just return A, else B also gets evaluated.
void Compiler::and_(bool canAssign)
{
    int endJump = emitJump(OP_JUMP_IF_FALSE);

    emitByte(OP_POP);
    parser.parsePrecedence(P_AND, this);

    patchJump(endJump);
}

void Compiler::or_(bool canAssign)
{
    int elseJump = emitJump(OP_JUMP_IF_FALSE);
    int endJump = emitJump(OP_JUMP);

    patchJump(elseJump);
    emitByte(OP_POP);

    parser.parsePrecedence(P_OR, this);
    patchJump(endJump);
}

void Compiler::exitScope()
{
    scopeDepth--;

    // remove locals declared in scope
    while (localCount > 0 && locals[localCount - 1].depth > scopeDepth)
    {
        emitByte(OP_POP);
        localCount--;
    }
}

void Compiler::expression()
{
    parser.parsePrecedence(P_ASSIGNMENT, this);
}

// creates a block as long as we havent reached the end of block/file
void Compiler::block()
{
    while (!parser.check(T_RBRACE) && !parser.check(T_EOF))
    {
        declaration();
    }

    parser.consume(T_RBRACE, "Expected '}' after block.");
}

void Compiler::varDeclaration()
{
    uint8_t global = parseVariable("Expect variable name.");

    // if there is assignment var gets that expression result, else the var value is init to nil
    if (parser.match(T_EQ))
        expression();
    else
        emitByte(OP_NIL);

    parser.consume(T_SEMICOLON, "Expect ';' after variable declaration.");

    defineVariable(global);
}

void Compiler::expressionStatement()
{
    expression();
    parser.consume(T_SEMICOLON, "Expect ';' after expression.");
    emitByte(OP_POP);
}

void Compiler::forStatement()
{
    scopeDepth++;

    parser.consume(T_LPAREN, "Expected '(' after 'for'.");

    // Initializer clause
    if (parser.match(T_SEMICOLON))
    {
        // Nothing
    }
    else if (parser.match(T_VAR))
    {
        varDeclaration();
    }
    else
    {
        expressionStatement();
    }

    // condition clause
    int loopStart = currentChunk()->bytes.size();
    int exitJump = -1;
    if (!parser.match(T_SEMICOLON))
    {
        expression();
        parser.consume(T_SEMICOLON, "Expect ';'.");

        exitJump = emitJump(OP_JUMP_IF_FALSE);
        emitByte(OP_POP);
    }

    if (!parser.match(T_RPAREN))
    {
        int bodyJump = emitJump(OP_JUMP);
        int incrementStart = currentChunk()->bytes.size();
        expression();
        emitByte(OP_POP);
        parser.consume(T_RPAREN, "Expect ')' after for clauses.");

        emitLoop(loopStart);
        loopStart = incrementStart;
        patchJump(bodyJump);
    }

    statement();
    emitLoop(loopStart);

    if (exitJump != -1)
    {
        patchJump(exitJump);
        emitByte(OP_POP);
    }

    exitScope();
}

void Compiler::ifStatement()
{
    parser.consume(T_LPAREN, "Expect '(' after 'if'");
    expression();
    parser.consume(T_RPAREN, "Expect ')' after condition");

    // tells how much to offset the VM stack by if the condition is false
    int thenJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();

    int elseJump = emitJump(OP_JUMP);

    // back patch
    patchJump(thenJump);
    emitByte(OP_POP);

    // check for else statement
    if (parser.match(T_ELSE))
        statement();
    patchJump(elseJump);
}

void Compiler::whileStatement()
{
    int loopStart = currentChunk()->bytes.size();
    parser.consume(T_LPAREN, "Expect '(' after 'while'.");
    expression();
    parser.consume(T_RPAREN, "Expect ')' after condition.");

    int exitJump = emitJump(OP_JUMP_IF_FALSE);
    emitByte(OP_POP);
    statement();
    emitLoop(loopStart);

    patchJump(exitJump);
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
        varDeclaration();
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

    else if (parser.match(T_FOR))
    {
        forStatement();
    }

    else if (parser.match(T_IF))
    {
        ifStatement();
    }

    else if (parser.match(T_WHILE))
    {
        whileStatement();
    }

    else if (parser.match(T_LBRACE))
    {
        scopeDepth++;
        block();
        exitScope();
    }
    else
    {
        expressionStatement();
    }
}

bool Compiler::compile(std::shared_ptr<ByteArray> bytearray)
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