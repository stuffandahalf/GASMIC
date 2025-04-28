#include "../as.h"

int
evaluate_expr(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

#if 0
	i += countspaces(buffer);
	if ((buffer[i] == '(')) {
		if (!(consumed = evaluate_paren(la, &buffer[i]))) {
			return 0;
		}
		i += consumed;
	} else if ((consumed = evaluate_term(la, &buffer[i]))) {
		/* while true, find operator and term */

	}
	i += countspaces(buffer);

	return i;
#endif
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

int
evaluate_term(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

	/* \s*(<paren>|<num>|<register>|<symbol>)\s* */
	i += countspaces(&buffer[i]);
	if ((consumed = evaluate_paren(la, &buffer[i]))) {
		/* handle parenthesis */
#if 0
	} else if ((consumed = evaluate_num(la, &buffer[i]))) {
		/* handle num */
	} else if ((consumed = evaluate_register(la, &buffer[i]))) {
		/* handle register name */
	} else if ((consumed = evaluate_symbol(la, &buffer[i]))) {
		/* handle symbol */
#endif
	} else {
		return 0;
	}

	return i;
}

#define MAX_UNARY 2
int
evaluate_postfix_unary(struct line_arg *la, const char *buffer)
{
	int i = 0, j, consumed;
	int mod = 0;

	i += countspaces(&buffer[i]);
	if (!(consumed = evaluate_term(la, &buffer[i]))) {
		return 0;
	}
	i += consumed;

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
	if ((consumed = evaluate_##name(la, buffer))) { \
		i += consumed; \
		i += countspaces(&buffer[i]); \
		for (j = 0; j < sizeof(ops); j++) { \
			if (buffer[i] == ops[j]) { \
				break; \
			} \
		} \
		if (j == sizeof(ops)) { \
			return 0; \
		} \
		i++; \
		i += countspaces(&buffer[i]); \
	} \
	if (!(consumed = evaluate_##next(la, &buffer[i]))) { \
		return 0; \
	} \
	i += countspaces(la, &buffer[i]); \
	 \
	return i; \
}

#define MULTIPLICATIVE_OPERATORS { '*', '/', '%' }
BINARY_OPERATOR(multiplicative, prefix_unary, MULTIPLICATIVE_OPERATORS);
#define ADDITIVE_OPERATORS { '+', '-' }
BINARY_OPERATOR(additive, multiplicative, ADDITIVE_OPERATORS);
#define AND_OPERATOR { '&' }
BINARY_OPERATOR(bitand, additive, AND_OPERATOR);
#define XOR_OPERATOR { '^' }
BINARY_OPERATOR(bitxor, bitand, XOR_OPERATOR);
#define OR_OPERATOR { '|' }
BINARY_OPERATOR(bitor, bitxor, OR_OPERATOR);

