#ifndef GASMIC_PSEUDO_H
#define GASMIC_PSEUDO_H

#include <as.h>

struct pseudo_instruction {
	/*char instruction[10];*/
	char *instruction;
	void (*process)(Line *line);
	short int args;
};

struct pseudo_instruction *get_pseudo_op(Line *line);
void parse_pseudo_op(Line *line);

#endif /* GASMIC_PSEUDO_H */
