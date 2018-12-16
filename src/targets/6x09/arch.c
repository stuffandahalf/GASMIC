#include <arch.h>

extern Instruction instructions[];

static Instruction *locate_instruction(char *mnemonic, int arch) {
    for (Instruction *i = instructions; i->mne[0] != '\0'; i++) {
        if ((i->arcs & arch) && /*(i->regs & ) &&*/ streq(i->mne, mnemonic)) {
            return i;
        }
    }
    return NULL;
}

void parse_6809_instruction(Line *l) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, MC6809)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }

    puts("6809 instruction");
    printf("%s, %X, %X\n", i->mne, i->base_opcode, i->regs);
}

void parse_6309_instruction(Line *l) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, HD6309)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }
    
    puts("6309 instruction");
    printf("%s, %X, %X\n", i->mne, i->base_opcode, i->regs);
}

Architecture architectures[] = {
    { "6809", &parse_6809_instruction, MC6809 },
    { "6309", &parse_6309_instruction, HD6309 },
    { "", 0 }
};

Register registers[] = {
    { "A", RA, MC6809 | HD6309 },
    { "B", RB, MC6809 | HD6309 },
    { "D", RD, MC6809 | HD6309 },
    { "X", RX, MC6809 | HD6309 },
    { "Y", RY, MC6809 | HD6309 },
    { "U", RU, MC6809 | HD6309 },
    { "S", RS, MC6809 | HD6309 },
    { "PC", RPC, MC6809 | HD6309 },
    { "E", RE, HD6309 },
    { "F", RF, HD6309 },
    { "W", RW, HD6309 },
    { "Q", RQ, HD6309 },
    { "V", RV, HD6309 },
    { "Z", RZ, HD6309 },
    { "DP", RDP, MC6809 | HD6309 },
    { "CC", RCC, MC6809 | HD6309 },
    { "MD", RMD, HD6309 },
    { "", 0 }
};

Instruction instructions[] = {
    { "ABX", MC6809 | HD6309, INHERENT, NR, 0x3A, MODN },
    { "ADC", MC6809 | HD6309, IMMEDIATE | DIRECT | INDEXED | EXTENDED, RA | RB, 0x89, MODA },
    { "", 0, 0, 0, 0, 0 }
};
