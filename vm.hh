#ifndef VM_H
#define VM_H

#include "bytearray.hh"
#include "common.hh"
#include "object.hh"
#include "table.hh"
#include "compiler.hh"

#define STACK_MAX 256

enum InterpretResult
{
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
};

class Hashing
{
    public:

    size_t operator()(const std::shared_ptr<ObjString> obj) const;
};

class Equality
{
    public:

    bool operator()(const std::shared_ptr<ObjString> obj1, const std::shared_ptr<ObjString> obj2) const
    {
        return obj1->str == obj2->str;
    }
};

class VM
{
public:
    Compiler compiler;
    std::shared_ptr<ByteArray> bytearray = std::make_shared<ByteArray>();
    std::vector<uint8_t>::iterator ip; // Instruction pointer which points to the current chunk being run
    Value stack[STACK_MAX];
    Value *stackTop;
    Table<std::shared_ptr<ObjString>, Value, Hashing, Equality> strings;
    Table<std::shared_ptr<ObjString>, Value, Hashing, Equality> globals;
    Obj* objects;

    VM();

    void resetStack();

    void runtimeError(const char *format, ...);

    // Reads and executes bytes
    InterpretResult run();

    InterpretResult interpret(const char *source);

    void push(Value value);

    Value pop();

    Value peek(int distance);

    bool isFalsey(Value value);

    Value concatenate();
};

extern VM vm;

#endif