#ifndef BYTE_ARRAY_H
#define BYTE_ARRAY_H

#include "common.hh"
#include "values.hh"

/**

    @brief This class represents an array of bytes along with associated line numbers and constant values.
    The ByteArray class provides functionality to write bytes to the array, add constant values and retrieve their index.
    */
class ByteArray
{
    public:
    ValueArray constants;
    std::vector<int> lines;
    std::vector<uint8_t> bytes;

    ByteArray(){}

    void writeByte(uint8_t byte, int line);

    int addConstant(Value value);
};

#endif