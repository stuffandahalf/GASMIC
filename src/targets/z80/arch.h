#ifndef _ARCH_H
#define _ARCH_H

#include <as.h>

#define ENDIANNESS LITTLE_ENDIAN

#define DEFAULT_SYNTAX INTEL_SYNTAX

#define Z80 (1)

#define REG_A (0)
#define REG_F (1)
#define REG_AF (2)
#define REG_B (3)
#define REG_C (4)
#define REG_BC (5)
#define REG_D (6)
#define REG_E (7)
#define REG_DE (8)
#define REG_H (9)
#define REG_L (10)
#define REG_HL (11)
#define REG_IX (12)
#define REG_IY (13)
#define REG_SP (14)
#define REG_I (15)
#define REG_R (16)
#define REG_PC (17)

typedef uint16_t addr_t;

extern addr_t address;
extern Architecture architectures[];
extern Register registers[];
extern Instruction instructions[];

#endif
