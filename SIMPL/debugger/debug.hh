#ifndef simpl_debug_h
#define simpl_debug_h

#include "../bytearray/bytearray.hh"

class Disassembler
{
public:
    ByteArray *bytearray;
    const char *main_name;

    void disassembleByteArray();

    int constantInstruction(const char *name, int offset);

    int simpleInstruction(const char *name, int offset);

    int disassembleInstruction(int offset);
};

#endif