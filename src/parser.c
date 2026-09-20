#include <stdio.h>
#include <ctype.h>
#include "as.h"
#include "parser.h"
#include "pseudo.h"

static RULE(mnemonic);
static RULE(args);

int
consume_seq(const char *line, const char *seq, int flags)
{
	int i = 0;
	char l, s;

	while (line[i] != '\0' && seq[i] != '\0') {
		l = line[i];
		s = seq[i];
		if (flags & SEQ_CASEINSENSITIVE) {
			l = toupper(l);
			s = toupper(s);
		}
		if (l != s) {
			break;
		}
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
consume_comment(const char *line)
{
	int i = 0;

	i += consume_seq(line + i, ";", 1);
	if (i) {
		while (!consume_range(line + i, "\n\0", 1)) {
			i++;
		}
	}

#ifndef NDEBUG
	if (i) {
		fprintf(stderr, "COMMENT %.*s\n", i, line);
	}
#endif

	return i;
}

int
parse_line(struct line *l, const char *buffer)
{
	int i = 0, ms;

	i += consume_spaces(buffer + i);
	i += parse_label(l, buffer + i);
	i += consume_spaces(buffer + i);
	i += ms = parse_mnemonic(l, buffer + i);
	i += consume_spaces(buffer + i);
	if (ms) {
		/* parse_args */
		i += parse_args(l, buffer + i);
		//i += consume_spaces(buffer + i);
	}
	i += consume_comment(buffer + i);

	if (buffer[i] != '\n' && buffer[i] != '\0') {
#ifndef NDEBUG
		fprintf(stderr, "LINE END? = %c (%d), REMAINING %s\n", buffer[i], buffer[i], &buffer[i]);
#endif
		return 0;
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

int
parse_label(struct line *l, const char *buffer)
{
	int i = 0, ls, c;

	i += parse_identifier(l, buffer + i);

	i += c = consume_seq(buffer + i, ".", 0);
	if (c) {
		i += parse_identifier(l, buffer + i);
	}
	ls = i;

	/* TODO: Should a space be permitted between label and ':'? */
	//i += consume_spaces(buffer + i);
	i += c = consume_seq(buffer + i, ":", 0);
	if (!c) {
		return 0;
	}
	if (!ls) {
		/* TODO: set line error, invalid label */
		return 0;
	}

	/* resolve label */

	return i;
}

static int
parse_mnemonic(struct line *l, const char *buffer)
{
	int i = 0, il = 0, tl = 0;
	int j, k;

	struct {
		const struct mnemonic **tab;
		size_t tabsz;
		const char *prefix;
	} tabs[] = {
		{ pseudo_ops, pseudo_opc, "." },
		{ g_config.arch->instructions, g_config.arch->instructionc, NULL }
	};
	size_t tabsz = sizeof(tabs) / sizeof(tabs[0]);

	for (j = 0; !il && j < tabsz; j++) {
		il = 0, tl = i;
		if (tabs[j].prefix) {
			tl += consume_seq(buffer + i, tabs[j].prefix, 0);
		}
		for (k = 0; !il && k < tabs[j].tabsz; k++) {
			il = consume_seq(buffer + tl, tabs[j].tab[k]->mnemonic,
					SEQ_CASEINSENSITIVE);

			if (il) {
				l->mnemonic = tabs[j].tab[k];
			}
		}
	}

	i += tl + il;

	return i;
}

static int
parse_args(struct line *l, const char *buffer)
{
	int i = 0, c;
#ifndef NDEBUG
	int ai;
#endif

	do {
		i += consume_spaces(buffer + i);
#ifndef NDEBUG
		ai = i;
#endif
		i += c = g_config.arch->parse_arg(l, buffer + i);
#ifndef NDEBUG
		fprintf(stderr, "ARG = %.*s\n", c, &buffer[ai]);
#endif
		i += consume_spaces(buffer + i);
	} while ((i += c = consume_seq(buffer + i, ",", 0)), c);
	
	return i;
}

int
parse_string(struct line *l, const char *buffer)
{
	int esc = 0;
	static const char *q = "\"";

	int i = 0, c;
	if (!(i += c = consume_seq(buffer + i, q, 0), c)) {
		return 0;
	}
	while (!consume_seq(buffer + i, q, 0) || esc) {
		esc = !esc && buffer[i] == '\\';
		i++;
	}
	if (!(i += c = consume_seq(buffer + i, q, 0), c)) {
		return 0;
	}
	return i;
}

