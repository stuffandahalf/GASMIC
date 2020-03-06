#ifndef GASMIC_TARGETS_H
#define GASMIC_TARGETS_H

#include <lang.h>

#include <targets/6x09/arch.h>
#include <targets/z80/arch.h>
/*#include <targets/6502/arch.h>*/

#define TARGETS \
	TARGET(MC6809) \
	TARGET(HD6309) \
	TARGET(Z80) \
	/*TARGET(I8080)*/ \
	/*TARGET(MOS6502)*/

#define TARGET(t) t ## _init();
static INLINE void init_targets() { TARGETS }
#undef TARGET

#define TARGET(t) t ## _destroy();
static INLINE void destroy_targets() { TARGETS }
#undef TARGET

/*extern Architecture **architectures[];*/

#endif /* GASMIC_TARGETS_H */
