#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include "as.h"
#include "parser.h"
#include "pseudo.h"

static RULE(mnemonic);
static RULE(args);
static RULE(arg);
static RULE(number);

int
consume_seq(const char *line, const char *seq, int flags)
{
	int i = 0;
	char l, s;

	//printf("SEQ \"%p\", CHAR '%c' (%d)\n", seq, line[i], line[i]);
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
		while (line[i] != '\0' && !consume_range(line + i, "\n", 0)) {
			i++;
		}
	}

#if 0
#ifndef NDEBUG
	if (i) {
		fprintf(stderr, "COMMENT \"%.*s\"\n", i, line);
	}
#endif
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
	//i += consume_spaces(buffer + i);
	if (ms) {
		i += parse_args(l, buffer + i);
		//i += consume_spaces(buffer + i);
	}
	i += consume_spaces(buffer + i);
	i += consume_comment(buffer + i);

	if (buffer[i] != '\n' && buffer[i] != '\0') {
#ifndef NDEBUG
		fprintf(stderr, "LINE END [%d]? = %c (%d), REMAINING %s\n", i, buffer[i], buffer[i], &buffer[i]);
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
	//add_label(buffer, ls);

	return i;
}

static int
parse_mnemonic(struct line *l, const char *buffer)
{
	int i = 0, il = 0, tl = 0;
	int j, k;

	struct {
		const struct mnemonic *tab;
		size_t tabsz;
		const char *prefix;
	} tabs[] = {
		{ pseudo_ops, pseudo_opc, "." },
		{ g_config.arch->instructions, g_config.arch->instructionc, NULL }
	};
	size_t tabsz = ELEM_COUNT(tabs);

	for (j = 0; !il && j < tabsz; j++) {
		il = 0, tl = i;
		if (tabs[j].prefix) {
			tl += consume_seq(buffer + i, tabs[j].prefix, 0);
		}
		for (k = 0; !il && k < tabs[j].tabsz; k++) {
			il = consume_seq(buffer + tl, tabs[j].tab[k].mnemonic,
					SEQ_CASEINSENSITIVE);

			if (il) {
				l->mnemonic = &tabs[j].tab[k];
			}
		}
	}

	if (l->mnemonic == NULL) {
		return 0;
	}

	i += tl + il;

	return i;
}

static int
parse_args(struct line *l, const char *buffer)
{
	int i = 0, c;

	i += consume_spaces(buffer + i);
	if (!i) {
		/* junk characters after mnemonic */
		return 0;
	}
	do {
		i += consume_spaces(buffer + i);
		i += c = parse_arg(l, buffer + i);
		i += consume_spaces(buffer + i);
	} while ((i += c = consume_seq(buffer + i, ",", 0)), c);
	
	return i;
}

static int
parse_arg(struct line *l, const char *buffer)
{
	int o = 0, i = 0;

	const parse_token tokens[] = {
		parse_string,
		parse_expr,
		g_config.arch->parse_arg
	};
	size_t tokenc = ELEM_COUNT(tokens);

	if (l->argc == l->argsz) {
		/* need to allocate new argument */
		if (!l->argsz) {
			l->argsz = 1;
		} else {
			l->argsz += 2;
		}
		l->argv = realloc(l->argv, g_config.arch->argsz * l->argsz);
		if (!l->argv) {
			/* failed to allocate arguments */
			return 0;
		}
	}

	while (!o && i < tokenc) {
		o += tokens[i++](l, buffer + o);
	}

	if (o) {
		/* need to indicate a successful parse */
		l->argc++;
	}

	return o;
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
	while (buffer[i] != '\0' && (!consume_seq(buffer + i, q, 0) || esc)) {
		esc = !esc && buffer[i] == '\\';
		i++;
	}
	if (!(i += c = consume_seq(buffer + i, q, 0), c)) {
		return 0;
	}
	return i;
}

int
parse_expr(struct line *l, const char *buffer)
{
	return parse_number(l, buffer);
}

static int
parse_number(struct line *l, const char *buffer)
{
	/* would prefer if octal could be 0 prefixed like in C */
	static const struct basedef basetab[] = {
		{ .prefix = "0x", .base = 16 },
		{ .prefix = "0b", .base = 2 },
		{ .prefix = "0o", .base = 8 }
	};
	static size_t basetabsz = ELEM_COUNT(basetab);

	int i = 0, j, k, b = 10, c = 0, s = 0;
	long n = 0;
	char range[] = NUM_RNG UCASE_RNG;
	char *p;

	struct { const struct basedef *tab; size_t tabsz; } tabtab[] = {
		{ g_config.arch->basetab, g_config.arch->basetabc },
		{ basetab, basetabsz }
	};
	size_t tabtabc = ELEM_COUNT(tabtab);

	for (k = 0; !c && k < tabtabc; k++) {
		if (!tabtab[k].tab) {
			continue;
		}
		for (j = 0; !c && j < tabtab[k].tabsz; j++) {
			if ((i += c = consume_seq(buffer + i, tabtab[k].tab[j].prefix, SEQ_CASEINSENSITIVE)), c) {
				b = tabtab[k].tab[j].base;
			}
		}
	}

	/* cap characters that can be used for digits */
	range[b] = '\0';
	while (buffer[i + s] != '\0' && (p = strchr(range, toupper(buffer[i + s])))) {
		c = p - range;
		n = n * b + c;

		s++;
	};

	if (s == 0) {
		return 0;
	}

	i += s;
	ARG(l->argv, l->argc)->type = ARG_TYPE_NUM;
	ARG(l->argv, l->argc)->num = n;

	return i;
}

