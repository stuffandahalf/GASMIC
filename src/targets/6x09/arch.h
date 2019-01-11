#ifndef _ARCH_H
#define _ARCH_H

#include <as.h>

#define ENDIANNESS BIG_ENDIAN

#define MC6809 (1)
#define HD6309 (2)

#define REG_A       (0)
#define REG_B       (1)
#define REG_D       (2)
#define REG_X       (3)
#define REG_Y       (4)
#define REG_U       (5)
#define REG_S       (6)
#define REG_PC      (7)
#define REG_E       (8)
#define REG_F       (9)
#define REG_W       (10)
#define REG_Q       (11)
#define REG_V       (12)
#define REG_Z       (13)
#define REG_DP      (14)
#define REG_CC      (15)
#define REG_MD      (16)
#define REG_NONE    (17)

#define ADDR_MODE_INH   (0)
#define ADDR_MODE_IMM   (1)
#define ADDR_MODE_DIR   (2)
#define ADDR_MODE_IND   (4)
#define ADDR_MODE_EXT   (8)
#define ADDR_MODE_INTER (16)

#define ARG_ORDER_NONE      (0)
#define ARG_ORDER_FROM_REG  (1)
#define ARG_ORDER_TO_REG    (2)
#define ARG_ORDER_INTERREG  (3)

extern Architecture architectures[];
extern Instruction instructions[];

#endif
