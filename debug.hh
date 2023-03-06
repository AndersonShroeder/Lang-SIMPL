#ifndef simpl_debug_h
#define simpl_debug_h

#include "bytearray.hh"

/**

    @brief This class represents a disassembler for a programming language that converts bytecode instructions into human-readable
    assembly code. The Disassembler object contains a ByteArray object that stores the bytecode instructions, a name for the
    program being disassembled, and several functions for decoding and formatting individual bytecode instructions. The
    Disassembler is responsible for iterating through the bytecode instructions and printing out the corresponding assembly
    code for each instruction.
    */

class Disassembler
{
public:
    ByteArray *bytearray;
    const char *name;

    Disassembler(ByteArray* array, const char* name);

    void disassembleByteArray();

    int constantInstruction(const char *name, int offset);

    int simpleInstruction(const char *name, int offset);

    int byteInstruction(const char* name, int offset);

    int jumpInstruction(const char* name, int sign, int offset);

    int disassembleInstruction(int offset);
};

#endif