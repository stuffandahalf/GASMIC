#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "as.h"
#include "smem.h"
#include "targets.h"
#include "formats.h"
#include "pseudo.h"
#include "arithmetic.h"
#include "parser.h"

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
//static void parse_line(struct line *l, char *buffer);
//static void evaluate_mnemonic(struct context *ctx, struct line *l);

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

//extern struct syntax_handler motorola_syntax;
//extern struct syntax_handler intel_syntax;
//extern struct syntax_handler att_syntax;
//struct syntax_handler *syntax_handlers[] = {
	//&motorola_syntax,
	//&intel_syntax,
	//&att_syntax
//};

#define TARGET(t) &ARCH_ ## t,
Architecture **architectures[] = { TARGETS NULL };  /* NULL terminated array of targets */
#undef TARGET

enum errorcode {
	ERR_MSG_UNKNOWN,
	ERR_MSG_ARGS,
	ERR_MSG_FOPEN,
	ERR_MSG_MEM,
	ERR_MSG_MAX
};
const char *errmsgs[] = {
	"Unknown error.",
	"Invalid number of command line arguments.",
	"Failed to open file.",
	"Failed to allocate memory."
};
size_t errmsgc = ELEM_COUNT(errmsgs);

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

	if ((rcd = configure(argc, argv))) {
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
		rcd = ERR_MSG_ARGS;
		goto cleanup;
	} else if (g_config.in_fnamec == 0) {
		if ((rcd = assemble("stdin", stdin, NULL)) < 0) {
			goto cleanup;
		}
	} else {
		for (i = 0; i < g_config.in_fnamec; i++) {
			FILE *fp = fopen(g_config.in_fnames[i], "r");
			if (!fp) {
				rcd = ERR_MSG_FOPEN;
				goto cleanup;
			}
			rcd = assemble(g_config.in_fnames[i], fp, NULL);
			fclose(fp);
			if (rcd) {
				goto cleanup;
			}
		}
	}

	/* TODO: Resolve references here */

#ifndef NDEBUG
	printdf(("SYMBOLS\n"));
	sym = symtab.first;
	while (sym != NULL) {
		printdf(("%s = %" PRId64 "\n", sym->label, sym->value));

		//sfree(sym->label);
		//sym->label = NULL;
		tmp_sym = sym;
		sym = sym->next;
		free(tmp_sym);
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
#ifdef GNU_GETOPT
	g_config.in_fnamesz = 0;
	free(g_config.in_fnames);
#endif
	g_config.in_fnames = NULL;
	g_config.in_fnamec = 0;

	destroy_targets();
	g_context = NULL;
	release();
	if (rcd != 0) {
		if (rcd < 0 || rcd >= ERR_MSG_MAX) {
			rcd = 0;
		}
		fprintf(stderr, "ERROR (%d): %s\n", (rcd), errmsgs[rcd]);
		rcd = 1;
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
	//size_t length = 0;

	while (fgets(buffer, LINEBUFFERSIZE, fp) != NULL) {
		ctx.line_num++;
		if (buffer[0] == '\0' || buffer[0] == '\n') {
			continue;
		}
#if 0
#ifndef NDEBUG
		char *c = strchr(buffer, '\n');
		if (c) {
			*c = '\0';
		}
		fprintf(stderr, "BUFFER[%zu] = \"%s\"\n", strlen(buffer), buffer);
#endif
#endif
		//length = strlen(buffer);

		/* initialize line state */
		l.line_state = LINE_STATE_CLEAR;
		l.label = NULL;
		l.mnemonic = NULL;
		l.argv = NULL;
		l.argc = 0;
		l.argsz = 0;

		/* process line */
		if (!parse_line(&l, buffer)) {
			/* TODO: handle errors uniformly */
			fprintf(stderr, "Failed to parse line: %s", buffer);
			continue;
		}

#ifndef NDEBUG
		fprintf(stderr, "%zu\t", ctx.line_num);
		if (l.label) {
			fprintf(stderr, "%s:", l.label->label);
		} else {
			fprintf(stderr, "\t");
		}
		fprintf(stderr, "\t");
		//if (l.line_state & LINE_STATE_MNEMONIC) {
		if (l.mnemonic != NULL) {
			fprintf(stderr, "%s", l.mnemonic->mnemonic);
			for (int i = 0; i < l.argc; i++) {
				if (!i) {
					fprintf(stderr, "\t");
				} else {
					fprintf(stderr, ", ");
				}
				switch (ARG(l.argv, i)->type) {
				case ARG_TYPE_STRING:
					fprintf(stderr, "\"%s\"", ARG(l.argv, i)->str);
					free(ARG(l.argv, i)->str);
					break;
				case ARG_TYPE_NUM:
					fprintf(stderr, "%ld", ARG(l.argv, i)->num);
					break;
				default:
					fprintf(stderr, "?");
					break;
				}
			}
		}
		fprintf(stderr, "\n");
#endif

#if 0
		if (l.line_state & FLAG(LINE_STATE_LABEL)) {	  /* If current line has a label */
			add_label(&l);
		}
#endif
#if 1
		if (l.mnemonic) {
			/* TODO: not ready for this yet */
			//l.mnemonic->evaluate(&ctx, &l);
		}
#else
		if (l.line_state & FLAG(LINE_STATE_MNEMONIC)) {   /* If current line has a mnemonic */
			//g_config.syntax.evaluate_args(&l);
			//syntax_handlers[g_config.syntax]->evaluate_args(&l);
			//evaluate_args(&l);

			//evaluate_mnemonic(&ctx, &l);
		}
#endif

		if (l.argv) {
			free(l.argv);
		}
	}
	return 0;
}

#ifdef GNU_GETOPT
#define ARG_PREFIX "-"
#else
#define ARG_PREFIX "+"
#endif

static int
configure(int argc, char *const argv[])
{
	static const char *const help_str = "Usage: %s "
		"[-D symbol=value]... "
		"[-m arch] "
		"[-o outfile] "
		"[-f outformat] "
		"[-e symfile]\n";
	static const char *const arg_str = ARG_PREFIX "D:e:f:hm:o:";

	int rcd = 0, c;

	g_config.arch = *architectures[0];
	g_config.syntax = g_config.arch->default_syntax;
	g_config.out_fname = "a.out";
	g_config.in_fnames = NULL;
	g_config.in_fnamec = 0;
#ifdef GNU_GETOPT
	g_config.in_fnamesz = 0;
#endif
	g_config.export_fname = NULL;

	while ((c = getopt(argc, argv, arg_str)) != -1) {
		switch (c) {
		case 'D':	/* define symbol */
			break;
		case 'e':   /* export symbol table */
			g_config.export_fname = optarg;
			break;
		case 'f':	/* output file format */
			break;
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
#ifdef GNU_GETOPT
		case '\1': /* position-independent files */
			if (g_config.in_fnamec == g_config.in_fnamesz) {
				g_config.in_fnamesz += 2;
				g_config.in_fnames = realloc(g_config.in_fnames,
						sizeof(g_config.in_fnames[0]) * g_config.in_fnamesz);
				if (g_config.in_fnames == NULL) {
					rcd = ERR_MSG_MEM;
					goto err;
				}
			}
			g_config.in_fnames[g_config.in_fnamec++] = optarg;
			break;
#endif
		case 'h':
		case '?':
			printf(help_str, argv[0]);
			return 0;
		}
	}

	printdf(("argcount = %d\n", argc - optind));

#ifndef GNU_GETOPT
	g_config.in_fnames = argv + sizeof(char) * optind;
	g_config.in_fnamec = argc - optind;
#else
err:
	if (rcd) {
		free(g_config.in_fnames);
		g_config.in_fnamesz = 0;
		g_config.in_fnames = NULL;
		g_config.in_fnamec = 0;
	}
#endif
	return rcd;
}

