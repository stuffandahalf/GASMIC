#ifndef TARGETS_H
#define TARGETS_H

#include <targets/6x09/arch.h>
#include <targets/z80/arch.h>

#define str(s) #s
#define INIT_TARGET(target) target##_init() 
#define INIT_TARGETS() { \
    INIT_TARGET(MC6809); \
    INIT_TARGET(HD6309); \
    INIT_TARGET(Z80); \
}
#define TARGETS &ARCH_MC6809, &ARCH_HD6309, &ARCH_Z80, NULL

//extern Architecture **architectures[];

#endif
