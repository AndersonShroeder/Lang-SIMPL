#ifndef VM_H
#define VM_H

#include "../bytearray/bytearray.h"
#include "../common.h"
#include "../object.h"
#include "../table.h"

#define STACK_MAX 256

enum InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class VM
{
public:
    ByteArray *bytearray;
    std::vector<uint8_t>::iterator ip; // Instruction pointer which points to the current chunk being run
    Value stack[STACK_MAX];
    Value *stackTop;
    Table<ObjString, Value> strings;
    Table<ObjString, Value> globals;
    Obj* objects;

    VM();

    ~VM();

    void resetStack();

    void runtimeError(const char *format, ...);

    // Reads and executes bytes
    InterpretResult run();

    InterpretResult interpret(const char *source);

    void push(Value value);

    Value pop();

    Value peek(int distance);

    bool isFalsey(Value value);

    void concatenate();
};

extern VM vm;

#endif