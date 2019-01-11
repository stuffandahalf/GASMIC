#include <arch.h>

extern Instruction instructions[];
extern char *str_to_upper(char str[]);

static Instruction *locate_instruction(char *mnemonic, int arch) {
    for (Instruction *i = instructions; i->mnemonic[0] != '\0'; i++) {
        if ((i->architectures & arch) && streq(i->mnemonic, mnemonic)) {
            return i;
        }
    }
    return NULL;
}

static void parse_argument(const char input[], Argument *arg) {
    extern Register registers[];
    
    char *reg_name;
    Register *r;
    for (r = registers; r->name[0] != '\0'; r++) {
        reg_name = strdup(input);
        str_to_upper(reg_name);
        if (streq(reg_name, r->name)) {
            arg->type = ARG_TYPE_REG;
            arg->arg.r = r;
            #ifdef DEBUG
            printf("Argument is register %s\n", r->name);
            #endif
            free(reg_name);
            return;
        }
        free(reg_name);
    }
    
    /*char *end;
    size_t immediate = strtol(input, &end, 0);
    if (*/
    
    Symbol *last_full;
    Symbol *s;
    for (s = symtab->first; s != NULL; s = s->next) {
        char *c;
        for (c = s->label; *c != '.' && *c != '\0'; c++);
        switch (*c) {
        case '.':
            break;
            
        }
    }
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

static void parse_instruction(Line *l, int arch) {
    Instruction *i;
    if ((i = locate_instruction(l->mnemonic, MC6809)) == NULL) {
        die("Failed to locate instruction with mnemonic of %s on line %ld\n", l->mnemonic, line_num);
    }
    
    #ifdef DEBUG
    printf("%s, %X\n", i->mnemonic, i->base_opcode);
    #endif
    
    /*Argument *source = salloc(sizeof(Argument));
    Argument *dest = salloc(sizeof(Argument));*/
    Argument *args[l->argc];
    int j;
    for (j = 0; j < l->argc; j++) {
        args[j] = salloc(sizeof(Argument));
        parse_argument(l->argv[j], args[j]);
    }
    
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
