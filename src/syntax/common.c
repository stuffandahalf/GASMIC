#include "../as.h"

int
evaluate_expr(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

	i += countspaces(buffer);
	if ((buffer[i] == '(')) {
		if (!(consumed = evaluate_paren(la, &buffer[i]))) {
			return 0;
		}
		i += consumed;
	}
	i += countspaces(buffer);

	return i;
}

int
evaluate_paren(struct line_arg *la, const char *buffer)
{
	int i = 0, consumed;

	/* \s*\(\s*<expr>\s*\) */
	i += countspaces(buffer);
	if (buffer[i++] != '(') {
		return 0;
	}
	i += countspaces(buffer);
	if (!(consumed = evaluate_expr(la, &buffer[i]))) {
		return 0;
	}
	i += consumed;
	i += countspaces(buffer);
	if (buffer[i++] != ')') {
		return 0;
	}

	return i;
}

int
evaluate_term(struct line_arg *la, const char *buffer)
{
	return 0;
}

