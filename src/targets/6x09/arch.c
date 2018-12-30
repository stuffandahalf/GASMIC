#include <arch.h>

extern Instruction instructions[];

static Instruction *locate_instruction(char *mnemonic, int arch) {
    for (Instruction *i = instructions; i->mnemonic[0] != '\0'; i++) {
        //if ((i->arcs & arch) && /*(i->regs & ) &&*/ streq(i->mne, mnemonic)) {
            return i;
        //}
    }
    return NULL;
}

void parse_6809_instruction(Line *l) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, MC6809)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }

    puts("6809 instruction");
    printf("%s, %X\n", i->mnemonic, i->base_opcode);
}

void parse_6309_instruction(Line *l) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, HD6309)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }
    
    puts("6309 instruction");
    printf("%s, %X\n", i->mnemonic, i->base_opcode);
}

Architecture architectures[] = {
    { "6809", &parse_6809_instruction, MC6809 },
    { "6309", &parse_6309_instruction, HD6309 },
    { "", 0, 0 }
};

Register registers[] = {
    { "A", MC6809 | HD6309 },
    { "B", MC6809 | HD6309 },
    { "D", MC6809 | HD6309 },
    { "X", MC6809 | HD6309 },
    { "Y", MC6809 | HD6309 },
    { "U", MC6809 | HD6309 },
    { "S", MC6809 | HD6309 },
    { "PC", MC6809 | HD6309 },
    { "E", HD6309 },
    { "F", HD6309 },
    { "W", HD6309 },
    { "Q", HD6309 },
    { "V", HD6309 },
    { "Z", HD6309 },
    { "DP", MC6809 | HD6309 },
    { "CC", MC6809 | HD6309 },
    { "MD", HD6309 },
    { "", 0 }
};
int regc = sizeof(registers) / sizeof(Register) - 1;

Instruction instructions[] = {
    { "ABX", MC6809 | HD6309, 0x3A, ADDR_MODE_IMM | ADDR_MODE_DIR | ADDR_MODE_IND | ADDR_MODE_EXT, { &registers[REG_A], &registers[REG_B], NULL } },
    //{ "ADC", MC6809 | HD6309, IMMEDIATE | DIRECT | INDEXED | EXTENDED,  },
    { "", 0, 0, 0, {} }
};
