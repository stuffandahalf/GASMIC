#ifndef MC6x09_ARCH_H
#define MC6x09_ARCH_H

#include <as.h>

//#define ENDIANNESS ARCH_BIG_ENDIAN

//#define DEFAULT_SYNTAX SYNTAX_MOTOROLA

#define MC6809 (1)
#define HD6309 (2)

#define MC6809_REG_NONE    (-1)
#define MC6809_REG_A       (0)
#define MC6809_REG_B       (1)
#define MC6809_REG_D       (2)
#define MC6809_REG_X       (3)
#define MC6809_REG_Y       (4)
#define MC6809_REG_U       (5)
#define MC6809_REG_S       (6)
#define MC6809_REG_PC      (7)
#define MC6809_REG_DP      (8)
#define MC6809_REG_CC      (9)
#define HD6309_REG_E       (10)
#define HD6309_REG_F       (11)
#define HD6309_REG_W       (12)
#define HD6309_REG_Q       (13)
#define HD6309_REG_V       (14)
#define HD6309_REG_Z       (15)
#define HD6309_REG_MD      (16)

#define MC6809_ADDR_MODE_INH   (0)
#define MC6809_ADDR_MODE_IMM   (1)
#define MC6809_ADDR_MODE_DIR   (2)
#define MC6809_ADDR_MODE_IND   (4)
#define MC6809_ADDR_MODE_EXT   (8)
#define MC6809_ADDR_MODE_INTER (16)

//typedef uint16_t addr_t;

extern size_t address;
//extern Architecture architectures[];
//extern Register registers[];
//extern Instruction instructions[];

extern Architecture *ARCH_MC6809;
extern Architecture *ARCH_HD6309;

void MC6809_init(void);
void HD6309_init(void);

#endif
