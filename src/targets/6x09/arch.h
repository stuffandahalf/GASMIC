#ifndef _ARCH_H
#define _ARCH_H

#include <as.h>

#define MC6809 1
#define HD6309 2

typedef struct {
    char name[10];
    void (*parse_instruction)(Line *l);
    int value;
} Architecture;

enum register_values {
    NR = 0,
    RA = 1,
    RB = 2,
    RD = 4,
    RX = 8,
    RY = 16,
    RU = 32,
    RS = 64,
    RPC = 128,
    RE = 256,
    RF = 512,
    RW = 1024,
    RQ = 2048,
    RV = 4096,
    RZ = 8192,
    RDP = 16384,
    RCC = 32768,
    RMD = 65536
};

typedef struct {
    char name[5];
    uint32_t value;
    int arcs;
} Register;

typedef enum {
    INHERENT = 1,
    IMMEDIATE = 2,
    DIRECT = 3,
    INDEXED = 4,
    EXTENDED = 5
} address_modes;

typedef enum {
    MODN = 0,
    MODA = 0x10,
    MODB = 0x1000,
    MODC = 0x1100
} opcode_modifiers;

extern Architecture architectures[];
extern Instruction instructions[];

#endif
