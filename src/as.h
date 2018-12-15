#ifndef _AS_H
#define _AS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <smem.h>

#define streq(__s1, __s2) !strcmp((const char *)__s1, (const char *)__s2)
// Safe alloc
/*#define salloc(dest, size) { \
    if ((dest = malloc(size)) == NULL) { \
        die("Failed to allocate memory for variable %s\n", #dest); \
    } \
}
#define srealloc(dest, size) { \
    if ((dest = realloc(dest, size)) == NULL) { \
        die("Failed to reallocate memory for variable %s\n", #dest); \
    } \
}*/

#define MAXMNEMONICSIZE 10
typedef struct {
    char mne[MAXMNEMONICSIZE];
    int arcs;
    int regs;
    int addr_modes;
    uint16_t base_opcode;
    int op_shift;
} Instruction;


#define LABEL_STATE (1)
#define MNEMONIC_STATE (2)
typedef struct {
    char *label;
    char *mnemonic;
    char **argv;
    size_t argc;
    unsigned char arg_buf_size;
    unsigned char line_state;
} Line;

typedef struct symtab_entry {
    char *label;
    size_t address;
    struct symtab_entry *next;
} Symbol;

typedef struct {
    Symbol *first;
    Symbol *last;
} SymTab;

#endif
