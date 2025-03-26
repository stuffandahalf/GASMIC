#include <stdio.h>
#include "../as.h"


void
process_motorola_syntax(struct line *l)
{
	int i;
	printf("HERE %s", l->mnemonic);
	for (i = 0; i < l->argc; i++) {
		printf("\t(%d) \"%s\"", i, l->argv[i].val.str);
	}
	//printf("\n");

	// process arguments and set operand type(s) accordingly
	if (l->argc == 0) {
		l->address_mode = ADDR_MODE_INHERENT;
		goto complete;
	}

	for (i = 0; i < l->argc; i++) {
		switch (l->argv[i].val.str[0]) {
		case '#':
			if (l->argc > 1) {
				die("Immediate instructions take one argument\n");
			}
			l->argv[i].val.str++;
			l->address_mode = ADDR_MODE_IMMEDIATE;
			//parse_arg(&l->argv[i]);
			break;
		/*case '(':
			break;*/
		default:
			break;
		}
	}

complete:
	printf("\tADDR MODE %d\n", l->address_mode);
}

