#include "bytearray.hh"

/**

    @brief This function writes a byte to the ByteArray along with the line number where the byte was encountered.
    @param byte The byte to be written.
    @param line The line number where the byte was encountered.
    */
void ByteArray::writeByte(uint8_t byte, int line)
{
    bytes.push_back(byte);
    lines.push_back(line);
}

/**

    @brief This function adds a constant value to the ValueArray object within the ByteArray.
    @param value The constant value to be added.
    @return The index where the constant value is located in the ValueArray.
    */
int ByteArray::addConstant(Value value)
{
    constants.writeValue(value);
    return constants.size() - 1;
}