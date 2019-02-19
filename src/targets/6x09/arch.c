#include <arch.h>

static int test_instruction(Instruction *i, Line *l) {
    switch (i->arg_order) {
    case ARG_ORDER_NONE:
        return l->argc == 0;
    case ARG_ORDER_TO_REG:
    case ARG_ORDER_FROM_REG:
        return l->argc == 2;
    default:
        fail("Instruction %s has invalid argument order.\n", i->mnemonic);
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

static void process_instruction(Line *line, Instruction *instr, Register *reg, Data *data) {
    if (instr->address_modes == ADDR_MODE_INH) {
        if (reg != NULL) {
            fail("Instruction %s does not require a register.\n", instr->mnemonic);
        }
        data->type = DATA_TYPE_BYTES;
        data->contents.bytes.count = instr->base_opcode & 0xFF00 ? 2 : 1;
        data->contents.bytes.array = salloc(sizeof(uint8_t) * data->contents.bytes.count);
        uint16_t opcode = instr->base_opcode;
        int i;
        for (i = data->contents.bytes.count - 1; i >= 0; i--) {
            data->contents.bytes.array[i] = opcode & 0xFF;
            opcode >>= 8;
        }
        address += data->contents.bytes.count;
        add_data(data);
        return;
    }
    
}

static void parse_instruction(Line *l, int arch) {
    char *mnem = NULL;
    char *line = NULL;
    Register *reg = NULL;
    
    Instruction *i = NULL;
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
            fail("Intel syntax is not yet implemented.\n");
            //goto instruction_found;
            break;
        case ATT_SYNTAX:
            fail("AT&T syntax is not yet implemented.\n");
            break;
        }
        
next_instruction:
        continue;
    }
    fail("Invalid instruction.\n");
    
instruction_found:
#ifdef DEBUG
    printf("%s\t%X\n", i->mnemonic, i->base_opcode);
    if (reg != NULL) {
        printf("%s\n", reg->name);
    }
#else
    while(0);   //wtf
#endif
    Data *assembled = salloc(sizeof(Data));
    assembled->next = NULL;
    assembled->address = address;
    
    process_instruction(l, i, reg, assembled);
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
    { "A", 1, MC6809 | HD6309 },
    { "B", 1, MC6809 | HD6309 },
    { "D", 2, MC6809 | HD6309 },
    { "X", 2, MC6809 | HD6309 },
    { "Y", 2, MC6809 | HD6309 },
    { "U", 2, MC6809 | HD6309 },
    { "S", 2, MC6809 | HD6309 },
    { "PC", 2, MC6809 | HD6309 },
    { "E", 1, HD6309 },
    { "F", 1, HD6309 },
    { "W", 2, HD6309 },
    { "Q", 4, HD6309 },
    { "V", 2, HD6309 },
    { "Z", 2, HD6309 },
    { "DP", 1, MC6809 | HD6309 },
    { "CC", 1, MC6809 | HD6309 },
    { "MD", 1, HD6309 },
    { "", 0, 0 }
};
#ifdef DEBUG
int regc = sizeof(registers) / sizeof(Register) - 1;
#endif

Instruction instructions[] = {
    { "ABX", MC6809 | HD6309, 0x3A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ADC", MC6809 | HD6309, 0x89, ARG_ORDER_TO_REG, ADDR_MODE_IMM | ADDR_MODE_DIR | ADDR_MODE_IND | ADDR_MODE_EXT, { REGISTER(REG_A), REGISTER(REG_B), NULL } },
    
    { "ASLA", MC6809 | HD6309, 0x48, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ASLB", MC6809 | HD6309, 0x58, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "ASLD", HD6309, 0x1048, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ASRA", MC6809 | HD6309, 0x47, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ASRB", MC6809 | HD6309, 0x57, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "ASRD", HD6309, 0x1047, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "CLRA", MC6809 | HD6309, 0x4F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "CLRB", MC6809 | HD6309, 0x5F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "CLRD", HD6309, 0x104F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "CLRE", HD6309, 0x114F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "CLRF", HD6309, 0x115F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "CLRW", HD6309, 0x105F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "COMA", MC6809 | HD6309, 0x43, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "COMB", MC6809 | HD6309, 0x53, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "COMD", HD6309, 0x1043, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "COME", HD6309, 0x1143, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "COMF", HD6309, 0x1153, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "COMW", HD6309, 0x1053, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "DAA", MC6809 | HD6309, 0x19, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECA", MC6809 | HD6309, 0x4A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECB", MC6809 | HD6309, 0x5A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECD", HD6309, 0x104A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECE", HD6309, 0x114A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECF", HD6309, 0x115A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "DECW", HD6309, 0x105A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "INCA", MC6809 | HD6309, 0x4C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "INCB", MC6809 | HD6309, 0x5C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "INCD", HD6309, 0x104C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "INCE", HD6309, 0x114C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "INCF", HD6309, 0x115C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "INCW", HD6309, 0x105C, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "LSLA", MC6809 | HD6309, 0x48, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "LSLB", MC6809 | HD6309, 0x58, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "LSLD", HD6309, 0x1048, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "LSRA", MC6809 | HD6309, 0x44, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "LSRB", MC6809 | HD6309, 0x54, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "LSRD", HD6309, 0x1044, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "LSRW", HD6309, 0x1054, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "MUL", MC6809 | HD6309, 0x3D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "NEGA", MC6809 | HD6309, 0x40, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "NEGB", MC6809 | HD6309, 0x50, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "NEGD", HD6309, 0x1040, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "NOP", MC6809 | HD6309, 0x12, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "PSHSW", HD6309, 0x1038, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "PSHUW", HD6309, 0x103A, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "PULSW", HD6309, 0x1039, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "PULUW", HD6309, 0x103B, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ROLA", MC6809 | HD6309, 0x49, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ROLB", MC6809 | HD6309, 0x59, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "ROLD", HD6309, 0x1049, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "ROLW", HD6309, 0x1059, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "RORA", MC6809 | HD6309, 0x46, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "RORB", MC6809 | HD6309, 0x56, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "RORD", HD6309, 0x1046, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "RORW", HD6309, 0x1056, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "RTI", MC6809 | HD6309, 0x3B, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "RTS", MC6809 | HD6309, 0x39, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "SEX", MC6809 | HD6309, 0x1D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "SEXW", HD6309, 0x14, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "SWI", MC6809 | HD6309, 0x3F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "SWI2", MC6809 | HD6309, 0x103F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "SWI3", MC6809 | HD6309, 0x113F, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "TSTA", MC6809 | HD6309, 0x4D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "TSTB", MC6809 | HD6309, 0x5D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "TSTD", HD6309, 0x104D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "TSTE", HD6309, 0x114D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "TSTF", HD6309, 0x115D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    { "TSTW", HD6309, 0x105D, ARG_ORDER_NONE, ADDR_MODE_INH, { NULL } },
    
    { "LD", MC6809 | HD6309, 0x86, ARG_ORDER_TO_REG, ADDR_MODE_IMM | ADDR_MODE_DIR | ADDR_MODE_IND | ADDR_MODE_EXT, { REGISTER(REG_A), REGISTER(REG_B), NULL } },
    { "", 0, 0, 0, 0, {} }
};
