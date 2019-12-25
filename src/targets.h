#ifndef GASMIC_TARGETS_H
#define GASMIC_TARGETS_H

#include <targets/6x09/arch.h>
#include <targets/z80/arch.h>
//#include <targets/6502/arch.h>

#define str(s) #s
#define INIT_TARGET(target) target##_init() 
#define INIT_TARGETS() { \
    INIT_TARGET(MC6809); \
    INIT_TARGET(HD6309); \
    INIT_TARGET(Z80); \
    /*INIT_TARGET(MOS6502);*/ \
}
#define DESTROY_TARGET(target) target##_destroy()
#define DESTROY_TARGETS() { \
    DESTROY_TARGET(MC6809); \
    DESTROY_TARGET(HD6309); \
    DESTROY_TARGET(Z80); \
    /*DESTROY_TARGET(MOS6502);*/ \
}
#define TARGETS &ARCH_MC6809, &ARCH_HD6309, &ARCH_Z80, /*&ARCH_MOS6502,*/ NULL

//extern Architecture **architectures[];

#endif /* GASMIC_TARGETS_H */
