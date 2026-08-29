#include <stdio.h>
#include <unistd.h>
#include "as.h"
#include "smem.h"
#include "targets.h"
#include "formats.h"
#include "pseudo.h"
#include "arithmetic.h"

/*
 * For each input file
 *   open new context
 *   parse line into opcode + args
 *   evaluate args
 *   translate opcode + args -> binary
 *   add binary to context
 *   generate output file in specified format
 * if link
 *   combine all processed files into output format
 */

#define LINEBUFFERSIZE (256)
char buffer[LINEBUFFERSIZE];

static int configure(int argc, char *const argv[]);
/*static void trim_str(char str[]);*/
static void parse_line(struct line *l, char *buffer);
static void evaluate_mnemonic(struct context *ctx, struct line *l);

struct configuration g_config;
struct context *g_context;

FILE *out;
size_t address = 0;
size_t address_mask;	/* bits to mask the address to;*/
/*SymTab *undefined_symtab;*/

/*extern line_processor process_motorola_syntax;
extern line_processor process_intel_syntax;
extern line_processor process_att_syntax;

line_processor *syntax_handlers[] = {
	&process_motorola_syntax,
	&process_intel_syntax,
	&process_att_syntax,
	NULL
};*/

extern struct syntax_handler motorola_syntax;
//extern struct syntax_handler intel_syntax;
//extern struct syntax_handler att_syntax;
struct syntax_handler *syntax_handlers[] = {
	&motorola_syntax,
	//&intel_syntax,
	//&att_syntax
};

#define TARGET(t) &ARCH_ ## t,
Architecture **architectures[] = { TARGETS NULL };  /* NULL terminated array of targets */
#undef TARGET

#define FAILTO(err, tgt) \
	fprintf(stderr, "ERROR (%d): %s\n", (err), errmsgs[err < errmsgc ? (err) : 0]); \
	goto tgt;

#define ERR_MSG_UNKNOWN		0
#define ERR_MSG_ARGS		1
#define ERR_MSG_FOPEN		2
const char *errmsgs[] = {
	"Unknown error.",
	"Invalid number of command line arguments.",
	"Failed to open file."
};
size_t errmsgc = sizeof(errmsgs) / sizeof(errmsgs[0]);

int
main(int argc, char *const argv[])
{
	int rcd = 0;
	size_t i;
#ifndef NDEBUG
	struct symbol *sym, *tmp_sym;
#endif
	//Data *data, *tmp_data;

	init_targets();

	if ((rcd = configure(argc, argv)) < 0) {
		goto cleanup;
	}

#if 0
	init_address_mask();
	//set_syntax_parser();

	init_data_table();
#endif

	/* establish new context and handle file io */
	fprintf(stderr, "in_fnamec = %zd\n", g_config.in_fnamec);
	if (g_config.in_fnamec < 0) {
		FAILTO(ERR_MSG_ARGS, cleanup);
	} else if (g_config.in_fnamec == 0) {
		if ((rcd = assemble("stdin", stdin, NULL)) < 0) {
			FAILTO(-rcd, cleanup);
		}
	} else {
		for (i = 0; i < g_config.in_fnamec; i++) {
			FILE *fp = fopen(g_config.in_fnames[i], "r");
			if (!fp) {
				FAILTO(ERR_MSG_FOPEN, cleanup);
			}
			if ((rcd = assemble(g_config.in_fnames[i], fp, NULL)) < 0) {
				FAILTO(-rcd, cleanup);
			}

			fclose(fp);
		}
	}

	/* TODO: Resolve references here */

#ifndef NDEBUG
	printdf(("SYMBOLS\n"));
	sym = symtab.first;
	while (sym != NULL) {
		printdf(("%s = %" PRId64 "\n", sym->label, sym->value));

		sfree(sym->label);
		sym->label = NULL;
		tmp_sym = sym;
		sym = sym->next;
		sfree(tmp_sym);
	}
#endif /* NDEBUG */

	/*sfree(symtab->first->label)*/;
	/*sym = NULL;*/

#if 0
	printdf(("DATATAB\n"));
	data = datatab->first;
	while (data != NULL) {
		printdf(("data address: " SZXFMT ",  ", data->address));
		switch (data->type) {
		case DATA_TYPE_EXPRESSION:
			/*printdf("%" PRIu8 " bytes label \"%s\"\n", data->bytec, data->contents.symbol);*/
			/*sfree(data->contents.symbol);*/
#ifndef NDEBUG
			printf("RPN expression: ");
			print_token_list(data->contents.rpn_expr);
#endif
			free_token_chain(data->contents.rpn_expr);
			break;
		case DATA_TYPE_BYTES:
#ifndef NDEBUG
			printf("%" PRIu8 " bytes: ", data->bytec);
			for (i = 0; i < data->bytec; i++) {
				if (i) {
					printf(", ");
				}
				printf("%" PRIX8, data->contents.bytes[i]);
			}
			printf("\n");
#endif
			sfree(data->contents.bytes);
			break;
		case DATA_TYPE_NONE:
#ifndef NDEBUG
			printf("Empty data.\n");
#endif
			break;
		default:
#ifndef NDEBUG
			printf("Garbage data.\n");
#endif
			break;
		}
		tmp_data = data;
		data = data->next;
		sfree(tmp_data);
	}
	sfree(datatab);
	datatab = NULL;
	data = NULL;
	/*close(out);*/
#endif

cleanup:
	g_config.in_fnames = NULL;
	g_config.in_fnamec = 0;
	g_config.in_fname_size = 0;

	destroy_targets();
	g_context = NULL;
	release();
	if (rcd < 0) {
		rcd *= -1;
	}
	return rcd;
}





void
init_address_mask()
{
	int i;
	address_mask = 0;
	for (i = 0; i < g_config.arch->bytes_per_address * g_config.arch->byte_size; i++) {
		if (i) {
			address_mask <<= 1u;
		}
		address_mask |= 1u;
	}
	printdf(("Address mask: " SZXFMT "\n", address_mask));
}

int
assemble(const char *fname, FILE *fp, struct context *parent)
{
	struct line l;
	struct context ctx = { fname, fp, parent, 0 };

	while (fgets(buffer, LINEBUFFERSIZE, fp) != NULL) {
		ctx.line_num++;
		if (buffer[0] == '\0' || buffer[0] == '\n') {
			continue;
		}

		/* initialize line state */
		l.line_state = LINE_STATE_CLEAR;
		l.address_mode = ADDR_MODE_INVALID;
		l.addr_mode_post_op = POST_OP_NONE;
		l.argc = 0;

		/* process line */
		parse_line(&l, buffer);
#ifndef NDEBUG
		fprintf(stderr, "%zu\t", ctx.line_num);
		if (l.line_state & LINE_STATE_LABEL) {
			fprintf(stderr, "%s:", l.label);
		} else {
			fprintf(stderr, "\t");
		}
		fprintf(stderr, "\t");
		if (l.line_state & LINE_STATE_MNEMONIC) {
			fprintf(stderr, "%s", l.mnemonic);
			for (int i = 0; i < l.argc; i++) {
				if (!i) {
					fprintf(stderr, "\t");
				} else {
					fprintf(stderr, ", ");
				}
				fprintf(stderr, "%s", l.argv[i].raw);
			}
		}
		fprintf(stderr, "\n");
#endif

		if (l.line_state & FLAG(LINE_STATE_LABEL)) {	  /* If current line has a label */
			add_label(&l);
		}
		if (l.line_state & FLAG(LINE_STATE_MNEMONIC)) {   /* If current line has a mnemonic */
			//g_config.syntax.evaluate_args(&l);
			//syntax_handlers[g_config.syntax]->evaluate_args(&l);
			//evaluate_args(&l);

			evaluate_mnemonic(&ctx, &l);
		}
	}
	return 0;
}

static int
configure(int argc, char *const argv[])
{
	static const char *const help_str = "Usage: %s [-m arch] [-o outfile] [-f outformat] [-e symfile]\n";
	int c;

	g_config.arch = *architectures[0];
	g_config.syntax = g_config.arch->default_syntax;
	g_config.out_fname = "a.out";
	g_config.in_fname_size = 1;
	g_config.in_fnamec = 0;
	g_config.in_fnames = salloc(sizeof(char *) * g_config.in_fname_size);
	g_config.export_fname = NULL;

	while ((c = getopt(argc, argv, "-hm:o:f:e:")) != -1) {
		switch (c) {
		case 'm':	/* architecture */
			g_config.arch = find_arch(optarg);
			if (g_config.arch == NULL) {
				/*free(g_config.out_fname);*/
				die("Unsupported architecture: %s\n", optarg);
			}
			break;
		case 'o':	/* output file */
			/*free(g_config.out_fname);*/
			/*if ((g_config.out_fname = strdup(optarg)) == NULL) {
				die("Failed to allocate new output file name");
			}*/
			g_config.out_fname = optarg;
			break;
		case 'f':	/* output file format */
			break;
		case 'e':   /* export symbol table */
			g_config.export_fname = optarg;
			break;
		case 'h':
		case '?':
			printf(help_str, argv[0]);
			return 0;
		}
	}

	printdf(("argcount = %d\n", argc - optind));

	g_config.in_fnames = argv + sizeof(char) * optind;
	g_config.in_fnamec = argc - optind;

	return 1;
}

static void
parse_line(struct line *l, char *buffer)
{
	register char *c;
	struct line_arg *la = NULL;
	enum arg_type arg_type = ARG_TYPE_UNPROCESSED;
	for (c = buffer; *c != '\0'; c++) {
		switch (*c) {
		case '"':
			if (l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) {
				break;
			} else if (!(l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) && c != buffer) {
				fail("Quotes must occur at the beginning of a field.\n");
			}
			l->line_state ^= FLAG(LINE_STATE_DOUBLE_QUOTE);
			if (l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) {
				arg_type = ARG_TYPE_STRING;
				buffer++;
			} else {
				*c = '\0';
			}
			break;
		case '\'':
			if (l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) {
				break;
			} else if (!(l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) && c != buffer) {
				fail("Quotes must occur at the beginning of a field.\n");
			}
			l->line_state ^= FLAG(LINE_STATE_SINGLE_QUOTE);
			if (l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) {
				arg_type = ARG_TYPE_STRING;
				buffer++;
			} else {
				*c = '\0';
			}
			break;
		case ']':
			if (l->line_state & FLAG(LINE_STATE_BOUNDED)) {
				break;
			}
			if (!(l->line_state & FLAG(LINE_STATE_BRACKET))) {
				fail("']' requires '[' first.");
			}
		case '[':
			if (l->line_state & FLAG(LINE_STATE_BOUNDED)) {
				break;
			}
			l->line_state ^= FLAG(LINE_STATE_BRACKET);
			break;

		case '\t':
		case ' ':
			if (l->line_state & FLAG(LINE_STATE_BOUNDED)) {
				break;
			}
			if (c == buffer) {
				buffer++;
			}
			/*else if (l->line_state & LINE_STATE_MNEMONIC) {
				fail("Mnemonic already set.\n");
			}*/
			else if (!(l->line_state & FLAG(LINE_STATE_MNEMONIC))) {
				*c = '\0';
				l->mnemonic = buffer;
				l->line_state |= FLAG(LINE_STATE_MNEMONIC);
				buffer = c;
				buffer++;
			}
			break;
		case ',':
			/*if (c == buffer) {
				break;
			}*/
			if (l->line_state & FLAG(LINE_STATE_BOUNDED)) {
				break;
			}
			if (!(l->line_state & FLAG(LINE_STATE_MNEMONIC))) {
				fail("No mnemonic preceding argument.\n");
			}
			if (l->argc == LINE_ARG_MAX) {
				fail("Too many arguments provided. (max %d)\n", LINE_ARG_MAX);
			}
			la = &(l->argv[l->argc++]);
			la->type = arg_type;
			//la->state = ARG_STATE_CLEAR;
			//la->addr_mode = ADDR_MODE_INVALID;
			la->str = buffer;
			*c = '\0';
			buffer = c;
			buffer++;
			arg_type = ARG_TYPE_UNPROCESSED;
			break;
		case '\n':
			if (l->line_state & (FLAG(LINE_STATE_SINGLE_QUOTE) | FLAG(LINE_STATE_DOUBLE_QUOTE))) {
				fail("Unterminated string constant.\n");
			}
			*c = '\0';
			if (!(l->line_state & FLAG(LINE_STATE_MNEMONIC))) {
				l->mnemonic = buffer;
				buffer = c;
				buffer++;
				l->line_state |= FLAG(LINE_STATE_MNEMONIC);
				str_to_upper(l->mnemonic);
			} else {
				if (l->argc == LINE_ARG_MAX) {
					fail("Too many arguments provided. (max %d)\n", LINE_ARG_MAX);
				}
				la = &(l->argv[l->argc++]);
				la->type = arg_type;
				la->str = buffer;
				arg_type = ARG_TYPE_UNPROCESSED;
				/* *c = '\0';*/
			}
			buffer = c;
			buffer++;
			break;
		case ':':
			if (l->line_state & FLAG(LINE_STATE_BOUNDED)) {
				break;
			} else if (l->line_state & FLAG(LINE_STATE_LABEL)) {
				fail("Invalid label.\n");
			} else if (l->line_state & FLAG(LINE_STATE_MNEMONIC)) {
				fail("Label must occur at the beginning of a line.\n");
			} else if (arg_type == ARG_TYPE_STRING) {
				fail("Label cannot be a string literal.\n");
			}
			l->label = buffer;
			*c = '\0';
			buffer = c;
			buffer++;

			/*printdf("parsed literal label = %s\n", l->label);*/
			l->line_state |= FLAG(LINE_STATE_LABEL);
			break;
		case ';':
			return;
		}
		/*buffer++;	 // Why doesnt this work?*/
	}
	if (l->line_state & (FLAG(LINE_STATE_SINGLE_QUOTE) | FLAG(LINE_STATE_DOUBLE_QUOTE))) {
		fail("Unmatched quote.\n");
	}
	if (l->line_state & FLAG(LINE_STATE_BRACKET)) {
		fail("Unmatched bracket.\n");
	}

	//syntax_handlers[g_config.syntax]->evaluate_args(l);
}

const struct mnemonic *
match_instruction(struct line *line, const struct mnemonic **m, const char *prefix)
{
	//const struct mnemonic **m = g_config.arch->instructions;

	const char *match = line->mnemonic;
	if (prefix != NULL && strcasestr(match, prefix) == 0) {
		match += strlen(prefix);
	}
	if (*match == '\0') {
		return NULL;
	}

	printf("TEST \"%s\", %p -> %p -> \"%s\"\n", match, m, *m, (*m)->mnemonic);
	while (*m != NULL && strcmp((*m)->mnemonic, match) /* && CHECK COMPATIBILITY */) {
		m++;
	}

	return *m;
}

static void
evaluate_mnemonic(struct context *ctx, struct line *line)
{
	const struct mnemonic *m = NULL;

	if (!line->mnemonic || line->mnemonic[0] == '\0') {
		return;
	}

	m = match_instruction(line, pseudo_ops, ".");
	if (m == NULL) {
		m = match_instruction(line, g_config.arch->instructions, NULL);
	}
	if (m == NULL) {
		die("INVALID MNEMONIC \"%s\"\n", line->mnemonic);
	}
	m->evaluate(ctx, line);
	//m = match_instruction(line, g_config.arch->instructions) !=
	/*if ((m = get_pseudo_op(line)) != NULL) {
		//m->forms[0].callback(line);
	} else if ((m = get_instruction(line)) != NULL) {
		printf("FOUND INSTRUCTION %s\n", m->mnemonic);
		//process_inruction(line);
	}*/
}

