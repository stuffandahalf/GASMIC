#ifndef GASMIC_MC6x09_ARCH_H
#define GASMIC_MC6x09_ARCH_H

#include <as.h>

#define MC6809 (1)
#define HD6309 (2)

/*#define MC6809_REG_INVALID (-1)
#define MC6809_REG_NONE    (0)
#define MC6809_REG_A       (1)
#define MC6809_REG_B       (2)
#define MC6809_REG_D       (3)
#define MC6809_REG_X       (4)
#define MC6809_REG_Y       (5)
#define MC6809_REG_U       (6)
#define MC6809_REG_S       (7)
#define MC6809_REG_PC      (8)
#define MC6809_REG_DP      (9)
#define MC6809_REG_CC      (10)
#define HD6309_REG_E       (11)
#define HD6309_REG_F       (12)
#define HD6309_REG_W       (13)
#define HD6309_REG_Q       (14)
#define HD6309_REG_V       (15)
#define HD6309_REG_Z       (16)
#define HD6309_REG_MD      (17)*/
typedef enum {
    MC6809_REG_INVALID = -1,
    MC6809_REG_NONE,
    MC6809_REG_A,
    MC6809_REG_B,
    MC6809_REG_D,
    MC6809_REG_X,
    MC6809_REG_Y,
    MC6809_REG_U,
    MC6809_REG_S,
    MC6809_REG_PC,
    MC6809_REG_DP,
    MC6809_REG_CC,
    HD6309_REG_E,
    HD6309_REG_F,
    HD6309_REG_W,
    HD6309_REG_Q,
    HD6309_REG_V,
    HD6309_REG_Z,
    HD6309_REG_MD
} mc6x09_register_t;

//extern size_t address;

extern Architecture *ARCH_MC6809;
extern Architecture *ARCH_HD6309;

void MC6809_init(void);
void MC6809_destroy(void);
void HD6309_init(void);
void HD6309_destroy(void);

#endif
