#ifndef GASMIC_PSEUDO_H
#define GASMIC_PSEUDO_H

#include "as.h"

//const struct mnemonic *get_pseudo_op(struct line *line);
//void parse_pseudo_op(struct line *line);
extern const struct mnemonic *pseudo_ops[];
extern const size_t pseudo_opc;

#endif /* GASMIC_PSEUDO_H */
