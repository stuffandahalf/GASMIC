#include <ctype.h>
#include "parser.h"

static RULE(label);
static RULE(mnemonic);

int
consume_seq(const char *line, const char *seq)
{
	int i = 0;
	while (line[i] != '\0' && seq[i] != '\0' && line[i] == seq[i]) {
		i++;
	}

	if (seq[i] != '\0') {
		return 0;
	}

	return i;
}


int
consume_range(const char *line, const char *range, int count)
{
	int i = 0, j;
	while (line[i] != '\0' && (count == 0 || i < count)) {
		for (j = 0; range[j] != '\0'; j++) {
			if (line[i] == range[j]) {
				i++;
				break;
			}
		}
		if (range[j] == '\0') {
			return i;
		}
	}

	return i;
}

int
consume_spaces(const char *line)
{
	int i = 0;
	while (line[i] != '\0' && isspace(line[i])) {
		i++;
	}
	return i;
}

int
parse_line(struct line *l, const char *buffer)
{
	int i = 0;
	int ls, ms;

	i += consume_spaces(buffer + i);
	i += ls = parse_label(l, buffer + i);
	i += consume_spaces(buffer + i);
	i += ms = parse_mnemonic(l, buffer + i);
	i += consume_spaces(buffer + i);
	if (ms) {
		/* parse_args */
		//i += parse_args(l, buffer + i);
		//i += consume_spaces(buffer + i);
	}

	return i;
}

static int
parse_identifier(struct line *l, const char *buffer)
{
	int i = 0;

	i += consume_range(buffer + i, ALPHA_RNG "_", 1);
	if (i) {
		i += consume_range(buffer + i, ALPHANUM_RNG "_", 0);
	}

	return i;
}

static int
parse_label(struct line *l, const char *buffer)
{
	int i = 0, ls, c;

	i += parse_identifier(l, buffer + i);

	i += c = consume_seq(buffer + i, ".");
	if (c) {
		i += parse_identifier(l, buffer + i);
	}
	ls = i;

	i += consume_spaces(buffer + i);
	i += c = consume_seq(buffer + i, ":");
	if (!c) {
		return 0;
	}

	/* resolve label */
	/* save label */

#ifndef NDEBUG
	fprintf(stderr, "LABEL = %.*s\n", ls, buffer);
#endif

	return i;
}

static int
parse_mnemonic(struct line *l, const char *buffer)
{
	int i = 0;
	i += consume_seq(buffer + i, ".");
	i += consume_range(buffer + i, ALPHA_RNG, 0);

	/* mnemonic lookup */

#ifndef NDEBUG
	if (i) {
		fprintf(stderr, "MNEMONIC = %.*s\n", i, buffer);
	}
#endif


	return i;
}

