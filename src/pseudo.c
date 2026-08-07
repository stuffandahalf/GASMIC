#include <unistd.h>
#define GASMIC_CAN_SWITCH_DIR	1

#if defined(HAVE_FCNTL_H) && defined(HAVE_SYS_STAT_H)
#include <fcntl.h>
#include <sys/stat.h>
#define GASMIC_HAVE_POSIX_FILE_IO	1
#endif	/* defined(HAVE_FCNTL_H) && defined(HAVE_SYS_STAT_H) */

#include "as.h"
//#include "pseudo.h"
#include "arithmetic.h"
#include "lang.h"
#include "smem.h"

/*
 * struc?
 * .org
 * .equ
 * .include
 * .org
 * .resb
 * .fcb
 * .extern
 * .global
 * .ascii
 * .asciz
 * .extern
 * .global
 */

#if 0
static int pseudo_set_file(struct context *ctx, struct line *line);
#endif
static int pseudo_set_arch(struct context *ctx, struct line *line);
static int pseudo_set_byte(struct context *ctx, struct line *line);
static int pseudo_set_word(struct context *ctx, struct line *line);
static int pseudo_set_double(struct context *ctx, struct line *line);
static int pseudo_set_quad(struct context *ctx, struct line *line);
static int pseudo_reserve_bytes(struct context *ctx, struct line *line);
static int pseudo_reserve_words(struct context *ctx, struct line *line);
static int pseudo_reserve_doubles(struct context *ctx, struct line *line);
static int pseudo_reserve_quads(struct context *ctx, struct line *line);
static int pseudo_equ(struct context *ctx, struct line *line);
static int pseudo_include(struct context *ctx, struct line *line);
static int pseudo_insert(struct context *ctx, struct line *line);
static int pseudo_org(struct context *ctx, struct line *line);

// static struct pseudo_instruction pseudo_ops[] = {
// 	{ ".ARCH",		&pseudo_set_arch,			1 },
// 	{ ".FILE",		&pseudo_set_file,			1 },
// 
// 	{ ".DB",		&pseudo_set_byte,			-1 },
// 	{ ".DW",		&pseudo_set_word,			-1 },
// 	{ ".DD",		&pseudo_set_double,			-1 },
// 	{ ".DQ",		&pseudo_set_quad,			-1 },
// 
// 	{ ".RESB",		&pseudo_reserve_bytes,		1 },
// 	{ ".RESW",		&pseudo_reserve_words,		1 },
// 	{ ".RESD",		&pseudo_reserve_doubles,	1 },
// 	{ ".RESQ",		&pseudo_reserve_quads,		1 },
// 
// 	{ ".EQU",		&pseudo_equ,				1 },
// 	{ ".INCLUDE",	&pseudo_include,			1 },
// 	{ ".INSERT",	&pseudo_insert,				1 },
// 	{ ".ORG",		&pseudo_org,				1 },
// 	/*{ ".SYNTAX", &pseudo_syntax, 1 },*/
// 	{ NULL,			NULL,						0 }
// };

/* Define pseudo mnemonics */
#define PSEUDO_OP(name, callback, nargs) { \
	.mnemonic = (name), \
	.compatibility = -1, \
	.evaluate = callback \
	/*.forms = { \
		{ -1, 0, 0, {}, (nargs), (callback) } \
	}*/ \
}

static const struct mnemonic pseudo_ops[] = {
	PSEUDO_OP("ARCH", &pseudo_set_arch, 1),
	//PSEUDO_OP("FILE", &pseudo_set_file, 1),

	PSEUDO_OP("DB", &pseudo_set_byte, -1),
	PSEUDO_OP("DW", &pseudo_set_word, -1),
	PSEUDO_OP("DD", &pseudo_set_double, -1),
	PSEUDO_OP("DQ", &pseudo_set_quad, -1),

	PSEUDO_OP("RESB", &pseudo_reserve_bytes, 1),
	PSEUDO_OP("RESW", &pseudo_reserve_words, 1),
	PSEUDO_OP("RESD", &pseudo_reserve_doubles, 1),
	PSEUDO_OP("RESQ", &pseudo_reserve_quads, 1),

	PSEUDO_OP("EQU", &pseudo_equ, 1),
	PSEUDO_OP("INCLUDE", &pseudo_include, 1),
	PSEUDO_OP("INSERT", &pseudo_insert, 1),
	PSEUDO_OP("ORG", &pseudo_org, 1)
	// PSEUDO_OP("SYNTAX", &pseudo_syntax, 1)
};
static const size_t pseudo_op_sz = sizeof(pseudo_ops) / sizeof(struct mnemonic);

/*struct pseudo_instruction *
get_pseudo_op(struct line *line)
{
	struct pseudo_instruction *pseudo_op;

	for (pseudo_op = pseudo_ops; pseudo_op->instruction != NULL; pseudo_op++) {
		if ((pseudo_op->args == -1 || pseudo_ops->args == line->argc) &&
			streq(line->mnemonic, (*line->mnemonic == '.' ? pseudo_op->instruction : &pseudo_op->instruction[1]))) {
			return pseudo_op;
		}
	}
	return NULL;
}*/

const struct mnemonic *
get_pseudo_op(struct line *line)
{
	int i;
	char *line_mnemonic = line->mnemonic;
	
	if (*line_mnemonic == '.') {
		line_mnemonic++;
	}

	for (i = 0; i < pseudo_op_sz; i++) {
		printf("NEXT OP %s\n", pseudo_ops[i].mnemonic);

		const struct mnemonic *pseudo_op = &pseudo_ops[i];
		if (!strcmp(line_mnemonic, pseudo_op->mnemonic)) {
			return pseudo_op;
		}
	}

	return NULL;
}

void
parse_pseudo_op(struct context *ctx, struct line *line)
{
	//struct pseudo_instruction *pseudo_inst = get_pseudo_op(line);
	const struct mnemonic *pseudo_op = get_pseudo_op(line);
	if (pseudo_op == NULL) {
		fail("Unable to find pseudo instruction %s that takes " SZuFMT " arguments.\n", line->mnemonic, line->argc);
	}
	pseudo_op->evaluate(ctx, line);
}


static int
pseudo_set_arch(struct context *ctx, struct line *line)
{
	const Architecture *arch;

	if (datatab->first != NULL) {
		fail("Cannot switch architecture after code.\n");
	}

	arch = find_arch(line->argv[0].str);
	if (arch == NULL) {
		fail("Failed to locate architecture %s.\n", line->argv[0].str);
	}
	g_config.arch = arch;
	init_address_mask();
	printdf(("%s\n", g_config.arch->name));

	return 0;
}

#if 0
static int
pseudo_set_file(struct context *ctx, struct line *line)
{
	if (line->argv[0].type != ARG_TYPE_STRING) {
		die("File name must be a string.");
	}
	sfree(g_context->fname);
	if ((g_context->fname = saquire(str_clone(line->argv[0].str))) == NULL) {
		fail("Failed to copy substitute file name.\n");
	}

	return 0;
}
#endif

#define pseudo_set_data(T, line) { \
	Data *data; \
	size_t i; \
	int c = 0; \
	for (i = 0; i < (line)->argc; i++) { \
		data = init_data(salloc(sizeof(Data))); \
		data->address = address & address_mask; \
		if ((line)->argv[i].type == ARG_TYPE_STRING) { \
			data->type = DATA_TYPE_BYTES; \
			data->bytec = strlen((line)->argv[i].str); \
			data->contents.bytes = salloc(sizeof(uint8_t) * data->bytec); \
			memcpy(data->contents.bytes, (line)->argv[i].str, data->bytec); \
		} else { \
			data->type = DATA_TYPE_EXPRESSION; \
			data->bytec = sizeof(T); \
			data->contents.rpn_expr = (line)->argv[i].rpn_expr; \
		} \
		c += data->bytec; \
		address += data->bytec; \
		add_data(data); \
	} \
	return c; \
}

static int pseudo_set_byte(struct context *ctx, struct line *line) { pseudo_set_data(uint8_t, line); }
static int pseudo_set_word(struct context *ctx, struct line *line) { pseudo_set_data(uint16_t, line); }
static int pseudo_set_double(struct context *ctx, struct line *line) { pseudo_set_data(uint32_t, line); }
static int pseudo_set_quad(struct context *ctx, struct line *line) { pseudo_set_data(uint64_t, line); }

#undef pseudo_set_data

/* TODO: Need to resolve this */
#define pseudo_reserve_data(T, line) { \
	/*if (line->argc != 1) { \
		fail("Reserving bytes requires one parameter.\n"); \
	} \*/ \
	long count = 0; \
	/*char *end;*/ \
	Data *data = init_data(salloc(sizeof(Data))); \
	data->type = DATA_TYPE_BYTES; \
	return count; \
}

static int pseudo_reserve_bytes(struct context *ctx, struct line *line) { pseudo_reserve_data(uint8_t, line); }
static int pseudo_reserve_words(struct context *ctx, struct line *line) { pseudo_reserve_data(uint16_t, line); }
static int pseudo_reserve_doubles(struct context *ctx, struct line *line) { pseudo_reserve_data(uint32_t, line); }
static int pseudo_reserve_quads(struct context *ctx, struct line *line) { pseudo_reserve_data(uint64_t, line); }

#undef pseudo_reserve_data

static int
pseudo_equ(struct context *ctx, struct line *line)
{
	char *num_end;

	if (!(line->line_state & FLAG(LINE_STATE_LABEL))) {
		fail("Pseudo instruction .EQU requires a label on the same line.\n");
	}
	if (line->argc != 1) {
		fail("Invalid number of arguments for pseudo instruction .EQU.\n");
	}

	/* TODO: replace this with rpn arithmetic parsing */
	symtab.last->value = strtol(line->argv[0].str, &num_end, 0);
	/*if (line->argv[0] == num_end) {*/
	if (*num_end != '\0') {
		fail("Failed to parse given value.\n");
	}

	return 0;
}

static int
pseudo_include(struct context *ctx, struct line *line)
{
	FILE *fp;

	if (line->argv[0].type != ARG_TYPE_STRING) {
		//fail("File name is not a string. Did you forget to surround the file name in quotes?\n");
		// TODO: error, arg is not a string
		return -1;
	}

	fp = fopen(line->argv[0].str, "r");
	if (!fp) {
		// TODO: error failed to open file
		return -1;
	}

	assemble(line->argv[0].str, fp, ctx);
	fclose(fp);

	return 0;
}

/*
 * Inserts the raw bytes of this file into the resulting binary
 */
static int
pseudo_insert(struct context *ctx, struct line *line)
{
	Data *file_data;
#if defined(GASMIC_HAVE_POSIX_FILE_IO)
	int fd;
	off_t size;
	struct stat fstatus;

	if (line->argv[0].type != ARG_TYPE_STRING) {
		fail("Inserted file argument is not a string path.\n");
	}

	fd = open(line->argv[0].str, O_RDONLY);
	if (fd < 0) {
		fail("Failed to open file.\n");
	}

	if (fstat(fd, &fstatus) < 0) {
		fail("Failed to get file specifications.\n");
	}

	size = fstatus.st_size;
#else /* defined(GASMIC_HAVE_POSIX_FILE_IO) */
	FILE *inserted_file;
	long size;

	if (line->argv[0].type != ARG_TYPE_STRING) {
		fail("Inserted file argument is not a string path.\n");
	}

	inserted_file = fopen(line->argv[0].str, "rb");
	if (inserted_file == NULL) {
		fail("Failed to open file \"%s\" to be inserted. Does the file exists?\n", line->argv[0].str);
	}

	size = fsize(inserted_file);
#endif /* defined(GASMIC_HAVE_POSIX_FILE_IO) */

	while (size > 0) {
		file_data = init_data(salloc(sizeof(Data)));
		file_data->bytec = (uint8_t)((size > 255) ? 255 : size);
		file_data->address = address;
		file_data->type = DATA_TYPE_BYTES;
		file_data->contents.bytes = salloc(sizeof(uint8_t) * file_data->bytec);

#if defined(GASMIC_HAVE_POSIX_FILE_IO)
		read(fd, file_data->contents.bytes, file_data->bytec);
#else
		fread(file_data->contents.bytes, sizeof(uint8_t), file_data->bytec, inserted_file);
#endif

		add_data(file_data);

		address += file_data->bytec;
		size -= file_data->bytec;
	}

#if defined(GASMIC_HAVE_POSIX_FILE_IO)
	close(fd);
#else
	fclose(inserted_file);
#endif /* defined(GASMIC_HAVE_POSIX_FILE_IO) */

	printdf(("Inserted fname is %s\n", line->argv[0].str));

	return 0;
}

static int 
pseudo_org(struct context *ctx, struct line *line)
{
	char *lend;
	size_t new_address = strtoul(line->argv[0].str, &lend, 0) & address_mask;
	if (*lend == '\0') {
		/*printdf(("new address is 0xzX\n", new_address));*/
		printdf(("new address is 0x" SZXFMT "\n", new_address));
		address = new_address;
	} else {
		fail("Value is not a number.\n");
	}

	return 0;
}

