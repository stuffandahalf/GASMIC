#ifndef _AS_H
#define _AS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define die(fmt, ...) { \
    fprintf(stderr, fmt, ##__VA_ARGS__); \
    exit(1); \
}
#define streq(__s1, __s2) !strcmp((const char *)__s1, (const char *)__s2)
#define salloc(dest, size) { \
    if ((dest = malloc(size)) == NULL) { \
        die("Failed to allocate memory for variable %s\n", #dest); \
    } \
}

#define MAXMNEMONICSIZE 10
typedef struct {
    char mne[MAXMNEMONICSIZE];
    int arcs;
    int regs;
    int addr_modes;
    uint16_t base_opcode;
    int op_shift;
} Instruction;

typedef struct {
    char *label;
    char *mnemonic;
    char *args[];
} Input;

#endif
