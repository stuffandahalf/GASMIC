#include <stdio.h>
#include "../as.h"

static void evaluate_start(struct line_arg *la, char **buffer);
static void evaluate_indirect(struct line_arg *la, char **buffer);
static void evaluate_expr(struct line_arg *la, char **buffer);


static int
evaluate_motorola_args(struct line *l)
{
	int i;
	for (i = 0; i < l->argc; i++) {
		struct line_arg *la = &l->argv[i];
		char *c, *buffer;

		/* argument type was set during parsing, no need to evaluate */
		if (la->type) {
			continue;
		}

		buffer = la->raw;
		evaluate_start(la, &buffer);
	}
	return 0;
}

static void
evaluate_start(struct line_arg *la, char **buffer)
{
	if (**buffer == '[') {

	} else {
		evaluate_expr(la, buffer);
	}
}

static void
evaluate_indirect(struct line_arg *la, char **buffer)
{
	char *newBuf;

	if (la->type) {
		die("argument type already set\n");
	}
	if (**buffer != '[') {
		die("Indirect buffer must start with '['\n");
	}
	
	/* strip off leading '[' */
	newBuf = *buffer + 1;
	evaluate_expr(la, &newBuf);
	if (*newBuf != ']') {
		die("failed to parse indirect expression \"%s\"\n", la->raw);
	}
	la->type = ARG_TYPE_INDIRECT;
	newBuf++;
	*buffer = newBuf;
}

static void
evaluate_expr(struct line_arg *la, char **buffer)
{
}

#if 0
static int
_evaluate_motorola_args(struct line *l)
{
	int i;
	for (i = 0; i < l->argc; i++) {
		struct line_arg *la = &l->argv[i];
		const char *buffer = NULL;
		uint8_t state = 0;
		char *c;
		//char *ap = NULL, *c;
		//uint8_t type = ARG_TYPE_UNPROCESSED;

		/* argument type was set during parsing, no need to evaluate */
		if (la->type) {
			continue;
		}

		printef("ARG %d\t\"", i);
		buffer = la->raw;
		for (c = la->raw; *c != '\0'; c++) {
			printef("%c", *c);
			switch (*c) {
			case '[': /* start indirect */
				if (la->type) {
					die("argument type already set, cannot be indirect\n");
				}
				if (state & FLAG(ARG_STATE_BRACKET)) {
					die("nested indirection is unsupported\n");
				}
				if (buffer != c) {
					die("opening '[' cannot appear in the middle of an argument\n");
				}
				state |= ARG_STATE_BRACKET;
				la->type = ARG_TYPE_INDIRECT;
				break;
			case ']': /* end indirect */
				if (!(state & FLAG(ARG_STATE_BRACKET))) {
					die("unexpected indirection terminator\n");
				}
				state &= ~ARG_STATE_BRACKET;
				*c = '\0';
				/* parse indirect address from buffer */
				buffer = c + 1;
				break;
			case '+':
			case '-':
				/* pre/post inc/dec */
				if (buffer == c) {
					buffer++;
				} else {
					*c = '\0';
				}
				break;
			case '#':
				/* immediate */
				break;
			case '$': /* hex */
			case '@': /* octal */
			case '%': /* binary */
				/* numeric base */
				break;
			default:
				break;
			}
		}
		/* no argument type was determined, probably unset relative offset */
		if (la->type == ARG_TYPE_UNPROCESSED) {
			la->type = ARG_TYPE_SIGNED;
			la->num = 0;
		}
		printef("\"\n");
	}
	//printef("\n");

	return 0;
}
#endif

static int
process_motorola_syntax(struct line *l)
{
	int i;
	printf("HERE %s", l->mnemonic);
	for (i = 0; i < l->argc; i++) {
		printf("\t(%d) \"%s\"", i, l->argv[i].str);
	}
	//printf("\n");

	// process arguments and set operand type(s) accordingly
	if (l->argc == 0) {
		l->address_mode = ADDR_MODE_INHERENT;
		goto complete;
	}

	for (i = 0; i < l->argc; i++) {
		switch (l->argv[i].str[0]) {
		case '#':
			if (l->argc > 1) {
				die("Immediate instructions take one argument\n");
			}
			l->argv[i].str++;
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

	return 0;
}

struct syntax_handler motorola_syntax = {
	.evaluate_args = evaluate_motorola_args,
	.handler= process_motorola_syntax
};

