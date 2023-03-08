enum OpCode
{
    OP_CONSTANT,
    OP_NIL,
    OP_TRUE,
    OP_FALSE,
    OP_POP,
    OP_DEFINE_GLOBAL,
    OP_GET_LOCAL,
    OP_SET_LOCAL,
    OP_GET_GLOBAL,
    OP_SET_GLOBAL,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_NOT,
    OP_NEGATE,
    OP_RETURN,
    OP_PRINT,
    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,
}

pub struct ByteArray
{ 
    pub lines: Vec<u32>,
    pub bytes: Vec<u8>,
}

impl ByteArray
{
    fn write_byte(&mut self, byte: u8, line: u32)
    {   
        self.bytes.push(byte);
        self.lines.push(line);
    }

    fn add_constant(&mut self)
    {

    }
}
