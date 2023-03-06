#ifndef simpl_debug_h
#define simpl_debug_h

#include "bytearray.hh"

class Disassembler
{
public:
    ByteArray *bytearray;
    const char *name;

    Disassembler(ByteArray* array, const char* name);

    void disassembleByteArray();

    int constantInstruction(const char *name, int offset);

    int simpleInstruction(const char *name, int offset);

    int byteInstruction(const char* name, ByteArray* array, int offset);

    int disassembleInstruction(int offset);
};

#endif