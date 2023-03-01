#include "debug.hh"
#include "values.hh"
#include "bytecodes.hh"

Disassembler::Disassembler(ByteArray* array, const char* name)
{
    bytearray = array;
    this->name = name;
}

void Disassembler::disassembleByteArray()
{
    printf("== %s ==\n", name);

    // incrementing of offset is done by disassembleInstruction because instructions have different sizes
    for (int offset = 0; offset < bytearray->bytes.size();)
    {
        offset = disassembleInstruction(offset);
    }
}

int Disassembler::constantInstruction(const char *name, int offset)
{
    uint8_t constant = bytearray->bytes[offset + 1];
    printf("%-16s %4d '", name, constant);
    printValue(bytearray->constants.values[constant]);
    printf("'\n");
    return offset + 2;
}

int Disassembler::simpleInstruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

int Disassembler::disassembleInstruction(int offset)
{
    printf("%04d ", offset);
    if (offset > 0 && bytearray->lines[offset] == bytearray->lines[offset - 1])
    {
        printf("    | ");
    }
    else
    {
        printf("%4d ", bytearray->lines[offset]);
    }

    uint8_t instruction = bytearray->bytes[offset];
    switch (instruction)
    {
    case OP_CONSTANT:
        return constantInstruction("OP_CONSTANT", offset);
    case OP_NIL:
        return simpleInstruction("OP_NIL", offset);
    case OP_FALSE:
        return simpleInstruction("OP_FALSE", offset);
    case OP_TRUE:
        return simpleInstruction("OP_TRUE", offset);
    case OP_POP:
        return simpleInstruction("OP_POP", offset);
    case OP_DEFINE_GLOBAL:
        return constantInstruction("OP_DEFINE_GLOBAL", offset);
    case OP_GET_GLOBAL:
        return constantInstruction("OP_GET_GLOBAL", offset);
    case OP_SET_GLOBAL:
        return constantInstruction("OP_SET_GLOBAL", offset);
    case OP_EQUAL:
        return simpleInstruction("OP_EQUAL", offset);
    case OP_GREATER:
        return simpleInstruction("OP_GREATER", offset);
    case OP_LESS:
        return simpleInstruction("OP_LESS", offset);
    case OP_ADD:
        return simpleInstruction("OP_ADD", offset);
    case OP_SUBTRACT:
        return simpleInstruction("OP_SUBTRACT", offset);
    case OP_MULTIPLY:
        return simpleInstruction("OP_MULTIPLY", offset);
    case OP_DIVIDE:
        return simpleInstruction("OP_DIVIDE", offset);
    case OP_NOT:
        return simpleInstruction("OP_NOT", offset);
    case OP_NEGATE:
        return simpleInstruction("OP_NEGATE", offset);
    case OP_PRINT:
        return simpleInstruction("OP_PRINT", offset);
    case OP_RETURN:
        return simpleInstruction("OP_RETURN", offset);
    default:
        printf("Unknown opcode %d\n", instruction);
        return offset + 1;
    }
}