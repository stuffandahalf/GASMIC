#include <targets/z80/arch.h>

static void parse_z80_instruction(Line *l) {
    
}

Architecture z80_architectures[] = {
    { "Z80", &parse_z80_instruction, Z80 },
    { "", NULL, 0 }
};

Register z80_registers[] = {
    { "A", 1, Z80 },
    { "F", 1, Z80 },
    { "AF", 2, Z80 },
    { "B", 1, Z80 },
    { "C", 1, Z80 },
    { "BC", 2, Z80 },
    { "D", 1, Z80 },
    { "E", 1, Z80 },
    { "DE", 2, Z80 },
    { "H", 1, Z80 },
    { "L", 1, Z80 },
    { "HL", 2, Z80 },
    { "IX", 2, Z80 },
    { "IY", 2, Z80 },
    { "SP", 2, Z80 },
    { "I", 1, Z80 },
    { "R", 1, Z80 },
    { "PC", 2, Z80 },
    { "", 0, 0 }
};
#ifndef NDEBUG
//int regc = sizeof(registers) / sizeof(Register) - 1;
#endif

Instruction z80_instructions[] = {
    { "", 0, 0, 0, 0, 0 }
};
