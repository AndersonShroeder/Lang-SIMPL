#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include "../common.hh"
#include "../values.hh"

class ByteArray
{
    public:
    ValueArray constants;
    std::vector<int> lines;
    std::vector<uint8_t> bytes;

    void writeByte(uint8_t byte, int line);

    // Returns index where constant is located
    int addConstant(Value value);
};

#endif