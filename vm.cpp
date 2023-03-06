#include <stdarg.h>
#include <string.h>
#include "vm.hh"
#include "debug.hh"
#include "bytecodes.hh"
#include "table.cpp"


/* 
Every operation has a number associated with it in terms of how it modifies the stack i.e.:
- OP_ADD = -1 because it pops two elements from the stack and pushes one: x - 2 + 1 = x - 1

Adding a sequence of instructions gives the total effect on the stack:
- Expression: one because every expression results in only one value on the stack
- Statement: zero because a statement does not push any values onto the stack after execution
*/

size_t Hashing::operator()(const std::shared_ptr<ObjString> obj) const
{
        return std::hash<std::string>{}(obj->str);
}

VM::VM()
{
    resetStack();
}

void VM::resetStack()
{
    this->stackTop = this->stack;
    this->objects = NULL;
}

void VM::runtimeError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    // Get line/chunk that caused error for debugging
    size_t instruction = *(this->ip - this->bytearray->bytes.size() - 1);
    int line = this->bytearray->lines[instruction];
    fprintf(stderr, "[line %d] in script\n", line);
    resetStack();
}

// Reads and executes bytes
InterpretResult VM::run()
{
#define READ_BYTE() ((this->ip)++)
#define READ_CONSTANT() (this->bytearray->constants.values[*READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define BINARY_OP(valueType, op)                        \
    do                                                  \
    {                                                   \
        if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) \
        {                                               \
            runtimeError("Operands must be numbers.");  \
            return INTERPRET_RUNTIME_ERROR;             \
        }                                               \
        double b = AS_NUMBER(pop());                    \
        double a = AS_NUMBER(pop());                    \
        push(valueType(a op b));                        \
    } while (false)

#ifdef DEBUG_TRACE_EXECUTION
    printf("          ");
    for (Value *slot = this->stack; slot < this->stackTop; slot++)
    {
        printf("[ ");
        printValue(*slot);
        printf(" ]");
    }
    printf("\n");
    // disassembleInstruction(this->chunk, int(this->ip - this->bytearray->bytes));
#endif

    for (;;)
    {
        uint8_t instruction;
        switch (instruction = *READ_BYTE())
        {
        case OP_ADD:
        {
            if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
            {
                Value val = concatenate();
                push(val);
            }
            else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
            {
                double b = AS_NUMBER(pop());
                double a = AS_NUMBER(pop());
                push(NUMBER_VAL(a + b));
            }
            else
            {
                runtimeError("Operands must be two numbers or two strings.");
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }
        case OP_SUBTRACT:
        {
            BINARY_OP(NUMBER_VAL, -);
            break;
        }

        case OP_MULTIPLY:
        {
            BINARY_OP(NUMBER_VAL, *);
            break;
        }

        case OP_DIVIDE:
        {
            BINARY_OP(NUMBER_VAL, /);
            break;
        }

        case OP_NOT:
            push(BOOL_VAL(isFalsey(pop())));
            break;

        case OP_NEGATE:
        {
            // If value on top of stack is not a number - cant negate therefore runtime error
            // Check that value by using a peek showing the next item of the stack
            if (!IS_NUMBER(peek(0)))
            {
                runtimeError("Operand must be a number.");
                return INTERPRET_RUNTIME_ERROR;
            }
            push(NUMBER_VAL(-AS_NUMBER(pop())));
            break;
        }

        case OP_PRINT:
        {
            printValue(pop());
            std::cout << '\n';
            break;
        }

        case OP_RETURN:
        {
            // Exit interpreter
            return INTERPRET_OK;
        }

        case OP_CONSTANT:
        {
            Value constant = READ_CONSTANT();
            this->push(constant);
            break;
        }

        case OP_NIL:
            push(NIL_VAL);
            break;

        case OP_FALSE:
            push(BOOL_VAL(true));
            break;

        case OP_TRUE:
            push(BOOL_VAL(false));
            break;

        case OP_POP:
            pop();
            break;

        case OP_GET_LOCAL:
        {
            uint8_t slot = *READ_BYTE();
            push(stack[slot]);
            break;
        }

        case OP_SET_LOCAL:
        {
            uint8_t slot = *READ_BYTE();
            stack[slot] = peek(0);
            break;
        }

        case OP_DEFINE_GLOBAL:
        {
            std::shared_ptr<ObjString> name = READ_STRING();
            globals.tableSet(name, peek(0));
            pop();
            break;
        }

        case OP_GET_GLOBAL:
        {
            std::shared_ptr<ObjString> name = READ_STRING();
            Value value;
            if (!(globals.tableGet(name, value)))
            {
                runtimeError("Undefined variable '%s'.", name->str);
                return INTERPRET_RUNTIME_ERROR;
            }
            push(value);
            break;
        }

        case OP_SET_GLOBAL:
        {
            std::shared_ptr<ObjString> name = READ_STRING();
            if (globals.tableSet(name, peek(0)))
            {
                globals.tableDelete(name);
                runtimeError("Undefined variable '%s'", name->str);
                return INTERPRET_RUNTIME_ERROR;
            }
            break;
        }

        case OP_GREATER:
            BINARY_OP(BOOL_VAL, >);
            break;

        case OP_LESS:
            BINARY_OP(BOOL_VAL, <);
            break;

        case OP_EQUAL:
        {
            Value b = pop();
            Value a = pop();
            push(BOOL_VAL(valuesEqual(a, b)));
            break;
        }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult VM::interpret(const char *source)
{
    // Chunk to be filled from user input
    ByteArray fill;
    compiler = Compiler(source);

    // If compilation fails, return result
    if (!compiler.compile(&fill))
    {
        return INTERPRET_COMPILE_ERROR;
    }

    // otherwise the chunk is run on the virtual machine
    this->bytearray = &fill;
    this->ip = this->bytearray->bytes.begin();
    InterpretResult result = run();

    return result;
}

/*
Assigns the next available space in the stack to the input value. Then moves stackTop to point to the
next available space in the stack.
 */
void VM::push(Value value)
{
    *(this->stackTop) = value;
    (this->stackTop)++;
}

Value VM::pop()
{
    this->stackTop--;
    return *(this->stackTop);
}

Value VM::peek(int distance)
{
    return this->stackTop[-1 - distance];
}

bool VM::isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

Value VM::concatenate()
{
    std::shared_ptr<ObjString> b = AS_STRING(pop());
    std::shared_ptr<ObjString> a = AS_STRING(pop());

    const int aLen = a->str.size();
    const int bLen = b->str.size();

    const int len = aLen + bLen;

    char characters[len];

    for (int i = 0; i < aLen; i++)
    {
        characters[i] = a->str.at(i);
    }

    for (int i = 0 ; i < bLen; i++)
    {
        characters[i + aLen] = b->str.at(i);
    }

    return OBJ_VAL(makeString(characters, len));
}