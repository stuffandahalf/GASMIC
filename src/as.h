#ifndef _AS_H
#define _AS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <smem.h>

#define DEBUG

#define streq(__s1, __s2) !strcmp((const char *)__s1, (const char *)__s2)

#define ARCH_BIG_ENDIAN 1
#define ARCH_LITTLE_ENDIAN 2

typedef struct symtab_entry {
    char *label;
    size_t value;
    struct symtab_entry *next;
} Symbol;

typedef struct {
    Symbol *first;
    Symbol *last;
    Symbol *last_parent;
} SymTab;

#define DATA_TYPE_LABEL 1
#define DATA_TYPE_BYTES 2
typedef struct data_entry {
    uint8_t type;
    union {
        Symbol *sym;
        struct {
            uint8_t byte_count;
            uint8_t *bytes;
        } bytes;
    } contents;
    struct data_entry *next;
} Data;

typedef struct {
    Data *first;
    Data *last;
} DataTab;

typedef struct {
    char name[5];
    int arcs;
} Register;

#define MAXMNEMONICSIZE (10)
#define MAX_REGS (20)
typedef struct {
    char mnemonic[MAXMNEMONICSIZE];
    uint8_t architectures;
    uint16_t base_opcode;
    uint8_t arg_order;
    uint8_t address_modes;
    Register *registers[MAX_REGS];
} Instruction;

#define ARG_TYPE_NONE 0
#define ARG_TYPE_REG  1
#define ARG_TYPE_SYM  2

typedef struct {
    uint8_t type;
    uint8_t addr_mode;
    union {
        char *str;
        Register *reg;
        Symbol *sym;
    } val;
} LineArg;

#define LABEL_STATE (1)
#define MNEMONIC_STATE (2)
#define QUOTE_STATE (4)
#define BRACKET_STATE (8)
typedef struct {
    char *label;
    char *mnemonic;
    LineArg *argv;
    //char **argv;
    size_t argc;
    uint8_t arg_buf_size;
    uint8_t line_state;
} Line;

typedef struct {
    char name[10];
    void (*parse_instruction)(Line *l);
    int value;
} Architecture;

extern size_t line_num;
extern SymTab *symtab;
extern DataTab *datatab;

void assemble(FILE *in, Line *l);
//void parse_arguments(Line *l);
void parse_pseudo_op(Line *line);

#endif
