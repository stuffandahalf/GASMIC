#include "arch.h"

Architecture architectures[] = {
    { "6809", MC6809 },
    { "6309", HD6309 },
    { "", 0 }
};

Instruction instructions[] = {
    { "ABX", MC6809 | HD6309, INHERENT, NR, 0x3A, MODN },
    { "ADC", MC6809 | HD6309, IMMEDIATE | DIRECT | INDEXED | EXTENDED, RA | RB, 0x89, MODA },
    { "", 0, 0, 0, 0, 0 }
};
