#ifndef Z80_ARCH_H
#define Z80_ARCH_H

#include <as.h>

//#define ENDIANNESS ARCH_LITTLE_ENDIAN

//#define DEFAULT_SYNTAX INTEL_SYNTAX

#define Z80 (1)

#define Z80_REG_INVALID (-1)
#define Z80_REG_NONE    (0)
#define Z80_REG_A       (1)
#define Z80_REG_F       (2)
#define Z80_REG_AF      (3)
#define Z80_REG_B       (4)
#define Z80_REG_C       (5)
#define Z80_REG_BC      (6)
#define Z80_REG_D       (7)
#define Z80_REG_E       (8)
#define Z80_REG_DE      (9)
#define Z80_REG_H       (10)
#define Z80_REG_L       (11)
#define Z80_REG_HL      (12)
#define Z80_REG_IX      (13)
#define Z80_REG_IY      (14)
#define Z80_REG_SP      (15)
#define Z80_REG_I       (16)
#define Z80_REG_R       (17)
#define Z80_REG_PC      (18)

#define Z80_ADDR_INVALID    (0)

typedef ARCH_INSTRUCTION(Z80, uint8_t, 18, 6) Z80_Instruction;

extern size_t address;
//extern Architecture architectures[];
//extern Register registers[];
//extern Instruction instructions[];

extern Architecture *ARCH_Z80;

void Z80_init(void);

#endif
