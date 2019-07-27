#ifndef Z80_ARCH_H
#define Z80_ARCH_H

#include <as.h>

//#define ENDIANNESS ARCH_LITTLE_ENDIAN

//#define DEFAULT_SYNTAX INTEL_SYNTAX

#define Z80 (1)

#define Z80_REG_A (0)
#define Z80_REG_F (1)
#define Z80_REG_AF (2)
#define Z80_REG_B (3)
#define Z80_REG_C (4)
#define Z80_REG_BC (5)
#define Z80_REG_D (6)
#define Z80_REG_E (7)
#define Z80_REG_DE (8)
#define Z80_REG_H (9)
#define Z80_REG_L (10)
#define Z80_REG_HL (11)
#define Z80_REG_IX (12)
#define Z80_REG_IY (13)
#define Z80_REG_SP (14)
#define Z80_REG_I (15)
#define Z80_REG_R (16)
#define Z80_REG_PC (17)

extern size_t address;
//extern Architecture architectures[];
//extern Register registers[];
//extern Instruction instructions[];

extern Architecture *ARCH_Z80;

void Z80_init(void);

#endif
