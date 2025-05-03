#include "../as.h"

int
evaluate_expr(struct line_arg *la, const char *buffer)
{
	return evaluate_bitor(la, buffer);
}

int
evaluate_paren(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

	/* \s*\(\s*<expr>\s*\) */
	i += countspaces(&buffer[i]);
	if (buffer[i++] != '(') {
		return 0;
	}
	i += countspaces(&buffer[i]);
	if (!(consumed = evaluate_expr(la, &buffer[i]))) {
		return 0;
	}
	i += consumed;
	i += countspaces(&buffer[i]);
	if (buffer[i++] != ')') {
		return 0;
	}

	return i;
}

static inline
int
inrange(int n, int min, int max)
{
	return min <= n && n <= max;
}

int
evaluate_num(struct line_arg *la, const char *buffer)
{
	int i = 0;
	char c, base = 10, overflow = 0;
	unsigned long int n = 0;

	i += countspaces(&buffer[i]);

	if (buffer[i] == '0') {
		base = 8;
		i += 1;

		switch (buffer[i++]) {
		case 'x':
		case 'X':
			base = 16;
			overflow = 6;
			break;
		case 'b':
			base = 2;
			break;
		default:
			i -= 1;
			break;
		}
	}

loop:
	c = buffer[i];
	if (c == '\0') {
		return 0;
	}
	if (inrange(c, 'a', 'z')) {
		c += 'A' - 'a';
	}

	if (inrange(c, '0', '0' + base - overflow)) {
		n *= base;
		n += c - '0';
	} else if (overflow && inrange(c, 'A', 'A' + overflow)) {
		n *= base;
		n += c - 'A';
	} else {
		goto end;
	}
	i += 1;
	goto loop;

end:
	i += countspaces(&buffer[i]);
#ifndef NDEBUG
	printf("NUM \"%d\", remainder \"%s\"\n", n, &buffer[i]);
#endif

	return i;
}

int
evaluate_register(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;
	const Architecture *arch = g_config.arch;
	const Register *r, *registers = arch->registers;
	const char *c;

	for (r = registers; r->name[0] != '\0'; r++) {
		// skip unsupported registers
		if (!(r->arcs & arch->value)) {
			continue;
		}
		printf("%s\t%s\n", r->name, buffer);
		i = 0;
		for (c = r->name; *c != '\0'; c++) {
			if (*c != buffer[i++]) {
				break;
			}
		}
		if (*c == '\0') {
#ifndef NDEBUG
			printf("FOUND REG %s\n", r->name);
#endif
			// TODO: save reg
			return i;
		}
	}

	return 0;
}

int
evaluate_term(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

#ifndef NDEBUG
	printf("TERM \"%s\"\n", buffer);
#endif

	/* \s*(<paren>|<num>|<register>|<symbol>)\s* */
	i += countspaces(&buffer[i]);
	if ((consumed = evaluate_paren(la, &buffer[i]))) {
		/* handle parenthesis */
	} else if ((consumed = evaluate_num(la, &buffer[i]))) {
		/* handle num */
	} else if ((consumed = evaluate_register(la, &buffer[i]))) {
		/* handle register name */
#if 0
	} else if ((consumed = evaluate_symbol(la, &buffer[i]))) {
		/* handle symbol */
#endif
	} else {
		return 0;
	}
	i += consumed;

	return i;
}

#define MAX_UNARY 2
int
evaluate_postfix_unary(struct line_arg *la, const char *buffer)
{
	int i = 0, j = 0, consumed;
	int modinc = 0, mod = 0;

	i += countspaces(&buffer[i]);
	if (!(consumed = evaluate_term(la, &buffer[i]))) {
		return 0;
	}
	i += consumed;

	while (j < MAX_UNARY) {
		modinc = 0;
		i += countspaces(&buffer[i]);
		if (mod >= 0 && buffer[i] == '+') {
			mod += 1;
			modinc = 1;
		} else if (buffer <= 0 && buffer[i] == '-') {
			mod -= 1;
			modinc = 1;
		}
		if (modinc) {
			i++;
		} else {
			break;
		}
	}
	i += countspaces(&buffer[i]);

	/* TODO: process */

	return i;
}

int
evaluate_prefix_unary(struct line_arg *la, const char *buffer)
{
	int i = 0, j = 0, consumed;
	int mod = 0;

	while (j < MAX_UNARY) {
		i += countspaces(&buffer[i]);
		if (buffer[i] == '+') {
			mod += 1;
		} else if (buffer[i] == '-') {
			mod -= 1;
		}
		if (mod != 0) {
			i++;
		} else {
			break;
		}
	}
	i += countspaces(&buffer[i]);
	if (!(consumed = evaluate_postfix_unary(la, &buffer[i]))) {
		return 0;
	}
	i += consumed;

	/* TODO: process */

	return i;
}

/* \s*(<name>\s*[operators]\s*)<next>\s* */
#define BINARY_OPERATOR(name, next, operators) \
int \
evaluate_##name(struct line_arg *la, const char *buffer) \
{ \
	const char ops[] = operators; \
	int i = 0, j, consumed; \
	 \
	i += countspaces(&buffer[i]); \
	if (!(consumed = evaluate_##next(la, &buffer[i]))) { \
		return 0; \
	} \
	i += consumed; \
	i += countspaces(la, &buffer[i]); \
	for (j = 0; j < sizeof(ops); j++) { \
		if (buffer[i] == ops[j]) { \
			/* store operator */ \
			break; \
		} \
	} \
	if (j != sizeof(ops)) { \
		i += 1; \
	} \
	i += countspaces(&buffer[i]); \
	if (j == sizeof(ops)) { \
		return i; \
	} \
	if (!(consumed = evaluate_##name(la, &buffer[i]))) { \
		return 0; \
	} \
	i += consumed; \
	i += countspaces(&buffer[i]); \
	 \
	return i; \
}

#define MULTIPLICATIVE_OPERATORS { '*', '/', '%' }
//BINARY_OPERATOR(multiplicative, prefix_unary, MULTIPLICATIVE_OPERATORS);
BINARY_OPERATOR(multiplicative, term, MULTIPLICATIVE_OPERATORS);
#define ADDITIVE_OPERATORS { '+', '-' }
BINARY_OPERATOR(additive, multiplicative, ADDITIVE_OPERATORS);
#define AND_OPERATOR { '&' }
BINARY_OPERATOR(bitand, additive, AND_OPERATOR);
#define XOR_OPERATOR { '^' }
BINARY_OPERATOR(bitxor, bitand, XOR_OPERATOR);
#define OR_OPERATOR { '|' }
BINARY_OPERATOR(bitor, bitxor, OR_OPERATOR);

