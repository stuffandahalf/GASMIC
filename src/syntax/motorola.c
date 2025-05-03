#include <stdio.h>
#include <string.h>
#include "../as.h"

extern int evaluate_expr(struct line_arg *la, const char *buffer);

static int evaluate_start(struct line_arg *la, const char *buffer);
static int evaluate_indirect(struct line_arg *la, const char *buffer);


static int
evaluate_motorola_args(struct line *l)
{
	int i;
	for (i = 0; i < l->argc; i++) {
		struct line_arg *la = &l->argv[i];
		char *c, *buffer;
		int consumed = 0;

		/* argument type was set during parsing, no need to evaluate */
		if (la->type) {
			continue;
		}

		buffer = la->raw;
		consumed = evaluate_start(la, buffer);
		if (!consumed || consumed != strlen(buffer)) {
			die("failed to parse complete argument \"%s\"\n", buffer);
		}
	}
	return 0;
}

static int
evaluate_start(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed = 0;

	printf("buffer \"%s\"\n", buffer);
	i += countspaces(&buffer[i]);
	if ((consumed = evaluate_indirect(la, &buffer[i]))) {
		i += consumed;
	} else if ((consumed = evaluate_expr(la, &buffer[i]))) {
		i += consumed;
	} else {
		return 0;
	}
	i += countspaces(&buffer[i]);

	return i;
}

static int
evaluate_indirect(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;
	char *newBuf;

	if (la->type) {
		die("argument type already set\n");
	}

	/* \s*\[\s*<expr>\s*\]\s* */
	i += countspaces(&buffer[i]);
	if (buffer[i++] != '[') {
		return 0;
	}
	i += countspaces(&buffer[i]);
	consumed = evaluate_expr(la, &buffer[i]);
	if (!consumed) {
		return 0;
	}
	i += consumed;
	i += countspaces(&buffer[i]);
	if (buffer[i++] != ']') {
		return 0;
	}
	i += countspaces(&buffer[i]);

	/* indirect arg valid */
	/* update line arg with new data */

	return i;
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

