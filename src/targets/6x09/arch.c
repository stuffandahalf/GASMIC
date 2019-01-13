#include <arch.h>

extern Instruction instructions[];
extern char *str_to_upper(char str[]);

static int test_instruction(Instruction *i, Line *l) {
    switch (i->arg_order) {
    case ARG_ORDER_NONE:
        return l->argc == 0;
    case ARG_ORDER_TO_REG:
    case ARG_ORDER_FROM_REG:
        return l->argc == 2;
    default:
        die("Instruction %s has invalid argument order\n", i->mnemonic);
        return 0;
    }
}

static Instruction *locate_instruction(Line *l, int arch) {
    Instruction *i;
    for (i = instructions; i->mnemonic[0] != '\0'; i++) {
        if ((i->architectures & arch) && streq(i->mnemonic, l->mnemonic) && test_instruction(i, l)) {
            return i;
        }
    }
    return NULL;
}

static void parse_instruction(Line *l, int arch) {
    //parse_arguments(l);
    Instruction *i;
    if ((i = locate_instruction(l, arch)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld.\nCheck argument order and types\n", l->mnemonic, line_num);
    }
    
    #ifdef DEBUG
    printf("%s, %X\n", i->mnemonic, i->base_opcode);
    #endif
    
    /*Argument *source = salloc(sizeof(Argument));
    Argument *dest = salloc(sizeof(Argument));*/
    /*Argument *args[l->argc];
    int j;
    for (j = 0; j < l->argc; j++) {
        args[j] = salloc(sizeof(Argument));
        parse_argument(l->argv[j], args[j]);
    }
    
    validate_arg_count(l, i);*/
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
