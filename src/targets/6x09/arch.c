#include <arch.h>

//extern Instruction instructions[];
//extern char *str_to_upper(char str[]);

static int test_instruction(Instruction *i, Line *l) {
    switch (i->arg_order) {
    case ARG_ORDER_NONE:
        return l->argc == 0;
    case ARG_ORDER_TO_REG:
    case ARG_ORDER_FROM_REG:
        return l->argc == 2;
    default:
        die("Error on line %ld. Instruction %s has invalid argument order\n", line_num, i->mnemonic);
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

/*static void parse_arguments(Line *l) {
    size_t i;
    for (i = 0; i < l->argc; i++) {
        LineArg *la = &(l->argv[i]);
        char *c;
        for (c = la->val.str; *c != '\0'; c++) {
            switch (*c) {
            case '[':
                
                break;
            }
        }
    }
}*/

static void parse_instruction(Line *l, int arch) {
    char *mnem;
    char *line;
    char *reg_name;
    Register *reg;
    
    Instruction *i;
    for (i = instructions; *i->mnemonic != '\0'; i++) {
        if (!(i->architectures & arch)) {
            goto next_instruction;
        }
        
        switch (configuration.syntax) {
        case MOTOROLA_SYNTAX:
            mnem = i->mnemonic;
            line = l->mnemonic;
            while (*mnem != '\0' && *line != '\0') {
                if (*mnem++ != *line++) {
                    goto next_instruction;
                }
            }
            
            switch (i->arg_order) {
            case ARG_ORDER_NONE:
            case ARG_ORDER_INTERREG:
                if (*line != '\0') {
                    goto next_instruction;
                }
                goto instruction_found;
            case ARG_ORDER_FROM_REG:
            case ARG_ORDER_TO_REG:
                for (reg = registers; *reg->name != '\0'; reg++) {
                    if (streq(line, reg->name)) {
                        goto instruction_found;
                    }
                }
            }
            
            //printf("not fail\n");
            //goto instruction_found;
            //if (argc - 1 == 
            //Register *get_register(line->arg
            break;
        case INTEL_SYNTAX:
            die("Intel syntax is not yet implemented\n");
            //goto instruction_found;
            break;
        case ATT_SYNTAX:
            die("AT&T syntax is not yet implemented");
            break;
        }
        
next_instruction:
        continue;
    }
    die("Invalid instruction on line %ld\n", line_num);
    
instruction_found:
    #ifdef DEBUG
    printf("%s\t%X\n", i->mnemonic, i->base_opcode);
    printf("%s\n", reg->name);
    #endif

}

static void parse_6809_instruction(Line *l) {
    parse_instruction(l, MC6809);
}

static void parse_6309_instruction(Line *l) {
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
    { "LD", MC6809 | HD6309, 0x86, ARG_ORDER_TO_REG, ADDR_MODE_IMM | ADDR_MODE_DIR | ADDR_MODE_IND | ADDR_MODE_EXT, { &registers[REG_A], &registers[REG_B], NULL } },
    { "", 0, 0, 0, 0, {} }
};
