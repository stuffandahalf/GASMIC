#include <pseudo.h>
#include <util.h>
#include <arithmetic.h>

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

static void pseudo_set_file(Line *line);
static void pseudo_set_arch(Line *line);
static void pseudo_set_byte(Line *line);
static void pseudo_set_word(Line *line);
static void pseudo_set_double(Line *line);
static void pseudo_set_quad(Line *line);
static void pseudo_reserve_bytes(Line *line);
static void pseudo_reserve_words(Line *line);
static void pseudo_reserve_doubles(Line *line);
static void pseudo_reserve_quads(Line *line);
static void pseudo_equ(Line *line);
static void pseudo_include(Line *line);
static void pseudo_insert(Line *line);
static void pseudo_org(Line *line);

static struct pseudo_instruction pseudo_ops[] = {
    { ".ARCH", &pseudo_set_arch,        1 },
    { ".FILE", &pseudo_set_file,        1 },

    { ".DB", &pseudo_set_byte,          -1 },
    { ".DW", &pseudo_set_word,          -1 },
    { ".DD", &pseudo_set_double,        -1 },
    { ".DQ", &pseudo_set_quad,          -1 },

    { ".RESB", &pseudo_reserve_bytes,   1 },
    { ".RESW", &pseudo_reserve_words,   1 },
    { ".RESD", &pseudo_reserve_doubles, 1 },
    { ".RESQ", &pseudo_reserve_quads,   1 },

    { ".EQU", &pseudo_equ,              1 },
    { ".INCLUDE", &pseudo_include,      1 },
    { ".INSERT", &pseudo_insert,        1 },
    { ".ORG", &pseudo_org,              1 },
    /*{ ".SYNTAX", &pseudo_syntax, 1 },*/
    { NULL, NULL,                     0 }
};

struct pseudo_instruction *get_pseudo_op(Line *line)
{
    struct pseudo_instruction *pseudo_op;

    for (pseudo_op = pseudo_ops; pseudo_op->instruction != NULL; pseudo_op++) {
        if ((pseudo_op->args == -1 || pseudo_ops->args == line->argc) &&
            streq(line->mnemonic, (*line->mnemonic == '.' ? pseudo_op->instruction : &pseudo_op->instruction[1]))) {
            return pseudo_op;
        }
    }
    return NULL;
}

void parse_pseudo_op(Line *line)
{
    struct pseudo_instruction *pseudo_inst = get_pseudo_op(line);
    if (pseudo_inst == NULL) {
        fail("Unable to find pseudo instruction %s that takes %zu arguments.\n", line->mnemonic, line->argc);
    }
    pseudo_inst->process(line);
}

static void pseudo_set_arch(Line *line)
{
    Architecture *arch;

    if (datatab->first != NULL) {
        fail("Cannot switch architecture after code.\n");
    }

    arch = find_arch(line->argv[0].val.str);
    if (arch == NULL) {
        fail("Failed to locate architecture %s.\n", line->argv[0].val.str);
    }
    configuration.arch = arch;
    init_address_mask();
    printdf(("%s\n", configuration.arch->name));
}

static void pseudo_set_file(Line *line)
{
    if (line->argv[0].type != ARG_TYPE_STRING) {
        die("File name must be a string.");
    }
    sfree(g_context->fname);
    if ((g_context->fname = saquire(strdup(line->argv[0].val.str))) == NULL) {
        fail("Failed to copy substitute file name.\n");
    }
}

#define pseudo_set_data(T, line) { \
    char *endptr; \
    T number; \
    Data *data; \
    size_t i; \
    for (i = 0; i < (line)->argc; i++) { \
        data = init_data(salloc(sizeof(Data))); \
        data->address = address & address_mask; \
        if ((line)->argv[i].type == ARG_TYPE_STRING) { \
            data->type = DATA_TYPE_BYTES; \
            data->bytec = strlen((line)->argv[i].val.str); \
            data->contents.bytes = salloc(sizeof(char) * data->bytec); \
            memcpy(data->contents.bytes, (line)->argv[i].val.str, data->bytec); \
        } else { \
            data->type = DATA_TYPE_EXPRESSION; \
            data->bytec = sizeof(T); \
            data->contents.rpn_expr = (line)->argv[i].val.rpn_expr; \
        } \
        address += data->bytec; \
        add_data(data); \
    } \
}

/*#define pseudo_set_data(T, line) { \
    char *send; \
    T number; \
    Data *data; \
    size_t i; \
    size_t j; \
    for (i = 0; i < line->argc; i++) { \
        data = salloc(sizeof(Data)); \
        data->type = DATA_TYPE_BYTES; \
        data->next = NULL; \
        data->address = address & address_mask; \
        printdf("%c\n", line->argv[i].val.str[0]); \
        if (line->argv[i].val.str[0] == '"' || line->argv[i].val.str[0] == '\'') { \
            data->bytec = strlen(line->argv[i].val.str) - 1; \
            data->contents.bytes = salloc(sizeof(uint8_t) * data->bytec); \
            uint8_t *current_byte = data->contents.bytes; \
            char *j; \
            for (j = &line->argv[i].val.str[1]; *j != '\0'; j++) { \
                number = (*j) & 0xFF; \
                *current_byte++ = number; \
            } \
            address += data->bytec; \
        } \
        else { \
            data->bytec = sizeof(T); \
            number = (T)strtol(line->argv[i].val.str, &send, 0); \
            if (*send != '\0') { \
                data->type = DATA_TYPE_SYMBOL; \
                data->contents.symbol = line->argv[i].val.str; \
                *//*fail("Unrecognized data %s\n.", line->argv[i].val.str);*//* \
            } \
            else { \
                data->contents.bytes = salloc(sizeof(T)); \
                if (configuration.arch->endianness == ARCH_ENDIAN_BIG) { \
                    printdf("big endian\n"); \
                    for (j = sizeof(T) - 1; j >= 0; j--) { \
                        data->contents.bytes[j] = number & 0xFF; \
                        number >>= 8; \
                    } \
                } \
                else if (configuration.arch->endianness == ARCH_ENDIAN_LITTLE) { \
                    printdf("little endian\n"); \
                    for (j = 0; j < sizeof(T); j++) { \
                        data->contents.bytes[j] = number & 0xFF; \
                        number >>= 8; \
                    } \
                } \
            } \
            address += sizeof(T); \
        } \
        add_data(data); \
    } \
}*/

static void pseudo_set_byte(Line *line) { pseudo_set_data(uint8_t, line); }
static void pseudo_set_word(Line *line) { pseudo_set_data(uint16_t, line); }
static void pseudo_set_double(Line *line) { pseudo_set_data(uint32_t, line); }
static void pseudo_set_quad(Line *line) { pseudo_set_data(uint64_t, line); }

#define pseudo_reserve_data(T, line) { \
    /*if (line->argc != 1) { \
        fail("Reserving bytes requires one parameter.\n"); \
    } \*/ \
    long count = 0; \
    char *end; \
    Data *data = init_data(salloc(sizeof(Data))); \
    data->type = DATA_TYPE_BYTES; \
}

static void pseudo_reserve_bytes(Line *line) { pseudo_reserve_data(uint8_t, line); }
static void pseudo_reserve_words(Line *line) { pseudo_reserve_data(uint16_t, line); }
static void pseudo_reserve_doubles(Line *line) { pseudo_reserve_data(uint32_t, line); }
static void pseudo_reserve_quads(Line *line) { pseudo_reserve_data(uint64_t, line); }

static void pseudo_equ(Line *line)
{
    char *num_end;

    if (!(line->line_state & FLAG(LINE_STATE_LABEL))) {
        fail("Pseudo instruction .EQU requires a label on the same line.\n");
    }
    if (line->argc != 1) {
        fail("Invalid number of arguments for pseudo instruction .EQU.\n");
    }

    symtab->last->value = strtol(line->argv[0].val.str, &num_end, 0);
    /*if (line->argv[0] == num_end) {*/
    if (*num_end != '\0') {
        fail("Failed to parse given value.\n");
    }
}

static void pseudo_include(Line *line)
{
    struct context included_context;
    Line *inc_line;

    if (line->argv[0].type != ARG_TYPE_STRING) {
        fail("File name is not a string. Did you forget to surround the file name in quotes?\n");
    }

    included_context.parent = g_context;
    /*included_context.fptr = included_file; */
    included_context.line_num = 1;
    if ((included_context.fname = saquire(strdup(line->argv[0].val.str))) == NULL) {
        fail("Failed to duplicate file name \"%s\"\n", line->argv[0].val.str);
    }

    /*FILE *included_file;*/
    if ((included_context.fptr = fopen(included_context.fname, "r")) == NULL) {
        fail("Failed to open included file \"%s\".\n", included_context.fname);
    }
    
    inc_line = salloc(sizeof(Line));
    g_context = &included_context;

    assemble(inc_line);
    fclose(included_context.fptr);

    g_context = g_context->parent;
    sfree(inc_line);
    sfree(included_context.fname);

    /* need to reassign argv because assemble frees it but we return back to assemble. */
    /*line->argc = 1; */
    /*line->argv = salloc(line->argc * sizeof(char *));*/
}

/*
 * Inserts the raw bytes of this file into the resulting binary
 */
static void pseudo_insert(Line *line)
{
    FILE *inserted_file;
}

static void pseudo_org(Line *line)
{
    char *lend;
    size_t new_address = strtoul(line->argv[0].val.str, &lend, 0) & address_mask;
    if (*lend == '\0') {
        printdf(("new address is 0xzX\n", new_address));
        address = new_address;
    } else {
        fail("Value is not a number.\n");
    }
}
