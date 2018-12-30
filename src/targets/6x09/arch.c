#include <arch.h>

extern Instruction instructions[];

static Instruction *locate_instruction(char *mnemonic, int arch) {
    for (Instruction *i = instructions; i->mnemonic[0] != '\0'; i++) {
        if ((i->architectures & arch) && streq(i->mnemonic, mnemonic)) {
            return i;
        }
    }
    return NULL;
}

static void validate_arg_count(Line *l, Instruction *i) {
    const char *invalid_args_error = "Invalid number of arguments for instruction %s on line %ld\n";
    switch (i->arg_order) {
    case ARG_ORDER_NONE:
        if (l->argc != 0) {
            die(invalid_args_error, i->mnemonic, line_num);
        }
        break;
    case ARG_ORDER_TO_REG:
    case ARG_ORDER_FROM_REG:
    case ARG_ORDER_INTERREG:
        if (l->argc != 2) {
            die(invalid_args_error, i->mnemonic, line_num);
        }
        break;
    }
}

void parse_instruction(Line *l, int arch) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, MC6809)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }
    
    #ifdef DEBUG
    printf("%s, %X\n", i->mnemonic, i->base_opcode);
    #endif
    
    validate_arg_count(l, i);
}

void parse_6809_instruction(Line *l) {    
    parse_instruction(l, MC6809);
}

void parse_6309_instruction(Line *l) {
    parse_instruction(l, HD6309);
}

Architecture architectures[] = {
    { "6809", &parse_6809_instruction, MC6809 },
    { "6309", &parse_6309_instruction, HD6309 },
    { "", NULL, 0 }
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
#ifdef DEBUG
int regc = sizeof(registers) / sizeof(Register) - 1;
#endif

Instruction instructions[] = {
    { "ABX", MC6809 | HD6309, 0x3A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ADC", MC6809 | HD6309, 0x89, ARG_ORDER_TO_REG, ADDR_MODE_IMM | ADDR_MODE_DIR | ADDR_MODE_IND | ADDR_MODE_EXT, { &registers[REG_A], &registers[REG_B], NULL } },
    { "", 0, 0, 0, 0, {} }
};
