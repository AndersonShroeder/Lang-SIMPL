#include "bytearray.h"

void ByteArray::writeByte(uint8_t byte, int line)
{
    bytes.push_back(byte);
    lines.push_back(line);
}

int ByteArray::addConstant(Value value)
{
    constants.writeValue(value);
    return constants.size() - 1;
}