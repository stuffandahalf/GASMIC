#include <arch.h>

static void parse_z80_instruction(Line *l) {
    
}

Architecture architectures[] = {
    { "Z80", &parse_z80_instruction, Z80 },
    { "", NULL, 0 }
};

Register registers[] = {
    { "A", Z80 },
    { "F", Z80 },
    { "AF", Z80 },
    { "B", Z80 },
    { "C", Z80 },
    { "BC", Z80 },
    { "D", Z80 },
    { "E", Z80 },
    { "DE", Z80 },
    { "H", Z80 },
    { "L", Z80 },
    { "HL", Z80 },
    { "IX", Z80 },
    { "IY", Z80 },
    { "SP", Z80 },
    { "I", Z80 },
    { "R", Z80 },
    { "PC", Z80 },
    { "", 0 }
};
#ifndef DEBUG
int regc = sizeof(registers) / sizeof(Register) - 1
#endif

Instruction instructions[] = {
    { "", 0, 0, 0, 0, {} }
};
