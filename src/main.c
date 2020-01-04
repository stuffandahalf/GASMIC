#include <stdio.h>
/*#ifdef _WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif*/
#include <getopt.h>
#include <as.h>
#include <targets.h>
#include <pseudo.h>
#include <util.h>
#include <arithmetic.h>
#include "as.h"

#define LINEBUFFERSIZE (256)
char buffer[LINEBUFFERSIZE];

static int configure(int argc, char *argv[]);
//static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void resolve_mnemonic_type(Line *l);
static void set_syntax_parser();
static void prepare_line_motorola(Line *l);
static void prepare_line_att(Line *l);
static void prepare_line_intel(Line *l);

Config configuration;
struct context *g_context;

//FILE *in;
FILE *out;
size_t address;
size_t address_mask;    // bits to mask the address to;
void (*parse_instruction)(Line *l);
//SymTab *undefined_symtab;

Architecture **architectures[] = { TARGETS };

int main(int argc, char **argv)
{
    INIT_TARGETS();

    //FILE *in;
    //out = stdout;
    configuration.arch = *architectures[0];
    configuration.in_fnames = NULL;
    configuration.in_fnamec = 0;
    configuration.syntax = configuration.arch->default_syntax;
	configuration.out_fname = "a.out";
    address = 0;

    if (!configure(argc, argv)) {
        goto early_exit;
    }

    //out = fopen(out_fname, "w+");
    //free(configuration.out_fname);
    configuration.out_fname = NULL;

    struct context init_cntxt;
    g_context = &init_cntxt;
    init_cntxt.line_num = 1;
    init_cntxt.parent = NULL;

    init_address_mask();
    set_syntax_parser();

    init_data_table();
    init_symbol_table();

    Line *l = salloc(sizeof(Line));

    if (configuration.in_fnamec < 0) {
        die("Invalid number of command line arguments.\n");
    } else if (configuration.in_fnamec == 0) {
        if ((init_cntxt.fname = saquire(strdup("stdin"))) == NULL) {
            die("Failed to duplicate file name \"stdin\"\n");
        }
        init_cntxt.fptr = stdin;
        assemble(l);
        sfree(init_cntxt.fname);
    } else {
        for (size_t i = 0; i < configuration.in_fnamec; i++) {
            if ((init_cntxt.fname = saquire(strdup(configuration.in_fnames[i]))) == NULL) {
                die("Failed to duplicate file name \"%s\"\n", configuration.in_fnames[i]);
            }
            if ((init_cntxt.fptr = fopen(init_cntxt.fname, "r")) == NULL) {
                die("Failed to open input file %s\n", configuration.in_fnames[i]);
            }
            init_cntxt.line_num = 1;
            assemble(l);
            fclose(init_cntxt.fptr);
            sfree(init_cntxt.fname);
        }
    }
    sfree(l);

    printdf("SYMBOLS\n");
    Symbol *sym = symtab->first;
    while (sym != NULL) {
        printdf("label: %s = %zu\n", sym->label, sym->value);

        sfree(sym->label);
        sym->label = NULL;
        Symbol *tmp = sym;
        sym = sym->next;
        sfree(tmp);
    }
    //sfree(symtab->first->label);
    sfree(symtab);
    symtab = NULL;
    //sym = NULL;

    printdf("DATATAB\n");
    Data *data = datatab->first;
    while (data != NULL) {
        printdf("data address: %zX,  ", data->address);
        switch (data->type) {
        case DATA_TYPE_EXPRESSION:
            //printdf("%" PRIu8 " bytes label \"%s\"\n", data->bytec, data->contents.symbol);
            //sfree(data->contents.symbol);
#ifndef NDEBUG
            printf("RPN expression: ");
            print_token_list(data->contents.rpn_expr);
#endif
            free_token_chain(data->contents.rpn_expr);
            break;
        case DATA_TYPE_BYTES:
#ifndef NDEBUG
            printf("%" PRIu8 " bytes: ", data->bytec);
            for (int i = 0; i < data->bytec; i++) {
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
            printdf("Empty data.\n");
            break;
        default:
            printdf("Garbage data.\n");
            break;
        }
        Data *tmp = data;
        data = data->next;
        sfree(tmp);
    }
    sfree(datatab);
    datatab = NULL;
    data = NULL;
    //fclose(out);

    g_context = NULL;

early_exit:
    DESTROY_TARGETS();

#if defined(_WIN32) && !defined(NDEBUG)
    getc(stdin);
#endif

	return 0;
}

void init_address_mask()
{
    address_mask = 0;
    for (int i = 0; i < configuration.arch->bytes_per_address * configuration.arch->byte_size; i++) {
        if (i) {
            address_mask <<= 1u;
        }
        address_mask |= 1u;
    }
    printdf("Address mask: %zX\n", address_mask);
}

void assemble(Line *l)
{
    while (fgets(buffer, LINEBUFFERSIZE, g_context->fptr) != NULL) {
        if (buffer[0] != '\0' && buffer[0] != '\n') {
            l->line_state = LINE_STATE_CLEAR;
            l->address_mode = ADDR_MODE_INVALID;
            l->addr_mode_post_op = POST_OP_NONE;
            l->arg_buf_size = 2;
            //l->argv = salloc(sizeof(char *) * l->arg_buf_size);
            l->argv = salloc(sizeof(LineArg) * l->arg_buf_size);
            l->argc = 0;

            parse_line(l, buffer);

#ifndef NDEBUG
            //printf("%s\t%s", l->label, l->mnemonic);
            if (l->line_state & FLAG(LINE_STATE_LABEL)) {
                printf("%s", l->label);
            }
            if (l->line_state & FLAG(LINE_STATE_MNEMONIC)) {
                if (l->line_state & FLAG(LINE_STATE_LABEL)) {
                    puts("\t");
                }
                printf("%s", l->mnemonic);
            }
            for (size_t i = 0; i < l->argc; i++) {
                printf("\t%s", l->argv[i].val.str);
            }
            puts("");
#endif
            if (l->line_state & FLAG(LINE_STATE_LABEL)) {      // If current line has a label
                add_label(l);
            }
            if (l->line_state & FLAG(LINE_STATE_MNEMONIC)) {   // If current line has a mnemonic
                str_to_upper(l->mnemonic);
                resolve_mnemonic_type(l);
            }

            sfree(l->argv);
        }
        g_context->line_num++;
    }
}

static int configure(int argc, char *argv[])
{
    const char *help_str = "Usage: %s [-m arch] [-o outfile] [-f outformat] [-e symfile]\n";
    int c;

    while ((c = getopt(argc, argv, "hm:o:f:e:")) != -1) {
        switch (c) {
        case 'm':	// architecture
            configuration.arch = find_arch(optarg);
            if (configuration.arch == NULL) {
                //free(configuration.out_fname);
                die("Unsupported architecture: %s\n", optarg);
            }
            break;
        case 'o':	// output file
            //free(configuration.out_fname);
            /*if ((configuration.out_fname = strdup(optarg)) == NULL) {
                die("Failed to allocate new output file name");
            }*/
            configuration.out_fname = optarg;
            break;
        case 'f':	// output file format
            break;
        case 'e':   // export symbol table
            break;
        case 'h':
            printf(help_str, argv[0]);
            return 0;
        case ':':
        //default:
            /*printdf("DEATH\n");
            printdf("%c\n", c);
            printdf("%s\n", optarg);*/
            die(help_str, argv[0]);
        }
    }

    printdf("argcount = %d\n", argc - optind);

    configuration.in_fnames = argv + sizeof(char) * optind;
    configuration.in_fnamec = argc - optind;

    return 1;
}

static void parse_line(Line *l, char *buffer)
{
    LineArg *la = NULL;
    enum arg_type arg_type = ARG_TYPE_UNPROCESSED;
    for (register char *c = buffer; *c != '\0'; c++) {
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
            if (!(l->line_state & FLAG(LINE_STATE_BRACKET))) {
                fail("']' requires '[' first.");
            }
        case '[':
            l->line_state ^= FLAG(LINE_STATE_BRACKET);
            break;

        case '\t':
        case ' ':
            if (l->line_state & (FLAG(LINE_STATE_SINGLE_QUOTE) | FLAG(LINE_STATE_DOUBLE_QUOTE) | FLAG(LINE_STATE_BRACKET))) {
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
            if (l->line_state & (FLAG(LINE_STATE_DOUBLE_QUOTE) | FLAG(LINE_STATE_SINGLE_QUOTE) | FLAG(LINE_STATE_BRACKET))) {
                break;
            }
            if (!(l->line_state & FLAG(LINE_STATE_MNEMONIC))) {
                fail("No mnemonic preceding argument.\n");
            }
            if (l->argc == l->arg_buf_size) {
                l->arg_buf_size += 2;
                l->argv = srealloc(l->argv, sizeof(LineArg) * l->arg_buf_size);
            }
            /*LineArg **/la = &(l->argv[l->argc++]);
            la->type = arg_type;
            //la->state = ARG_STATE_CLEAR;
            //la->addr_mode = ADDR_MODE_INVALID;
            la->val.str = buffer;
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
            } else {
                if (l->argc == l->arg_buf_size) {
                    l->arg_buf_size += 2;
                    l->argv = srealloc(l->argv, sizeof(LineArg) * l->arg_buf_size);
                }
                /*LineArg **/la = &(l->argv[l->argc++]);
                la->type = arg_type;
                la->val.str = buffer;
                arg_type = ARG_TYPE_UNPROCESSED;
                //*c = '\0';
            }
            buffer = c;
            buffer++;
            break;
        case ':':
            if (l->line_state & FLAG(LINE_STATE_LABEL)) {
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

            //printdf("parsed literal label = %s\n", l->label);
            l->line_state |= FLAG(LINE_STATE_LABEL);
            break;
        case ';':
            return;
        }
        //buffer++;     // Why doesnt this work?
    }
    if (l->line_state & (FLAG(LINE_STATE_SINGLE_QUOTE) | FLAG(LINE_STATE_DOUBLE_QUOTE))) {
        fail("Unmatched quote.\n");
    }
    if (l->line_state & FLAG(LINE_STATE_BRACKET)) {
        fail("Unmatched bracket.\n");
    }
}

static void resolve_mnemonic_type(Line *line)
{
    struct pseudo_instruction *pseudo_op;

    if (line->mnemonic[0] == '\0') {
        return;
    } else if (line->mnemonic[0] == '.') {
        prepare_line(line);
        parse_pseudo_op(line);
    } else if ((pseudo_op = get_pseudo_op(line)) != NULL) {
        prepare_line(line);
        pseudo_op->process(line);
    } else {
        //configuration.arch->parse_instruction(line);
		parse_instruction(line);
    }
}

static inline const struct instruction_register *instruction_supports_reg(const Instruction *instruction, const Register *reg)
{
	for (const struct instruction_register *ir = instruction->opcodes; ir->reg != NULL; ir++) {
		if (streq(reg->name, ir->reg->name)) {
			return ir;
		}
	}
	return NULL;
}

static void set_syntax_parser()
{
    switch (configuration.syntax) {
    case SYNTAX_MOTOROLA:
        parse_instruction = &prepare_line_motorola;
        break;
    case SYNTAX_ATT:
        fail("AT&T instruction syntax is not yet implemented.\n");
        parse_instruction = &prepare_line_att;
        break;
    case SYNTAX_INTEL:
        fail("Intel instruction syntax is not yet implemented.\n");
        parse_instruction = &prepare_line_intel;
        break;
    case SYNTAX_UNKNOWN:
        fail("Unrecognized syntax.\n");
    }
}

static void prepare_line_motorola(Line *l)
{
	/*const char *instr_mnem = NULL;
	const char *line_mnemonic = NULL;
	const Register *reg = NULL;
	const struct instruction_register *instruction_reg = NULL;
	struct LineArg *argcpy = NULL;  // store a copy of the arguments in case processing fails
    //size_t j;*/

	//char *unified_arg_str;

	if (l->argc == 0) {
	    l->address_mode = ADDR_MODE_INHERENT;
	    goto proceed;
	}

	size_t arg_len = 0;
	for (unsigned int i = 0; i < l->argc; i++) {
	    if (l->argv[i].type != ARG_TYPE_UNPROCESSED) {
	        fail("Instructions should not have complex arguments.\n");
	    }
	    arg_len += strlen(l->argv[i].val.str);
	}
	arg_len += l->argc - 1; // space for commas
	arg_len++;

	char *unified_arg_str = salloc(sizeof(char) * arg_len);

	//char *c = unified_arg_str;
	char *buffer_ptr = unified_arg_str;
	for (unsigned int i = 0; i < l->argc; i++) {
	    if (i) {
	        *buffer_ptr++ = ',';
	    }
        for (char *c = l->argv[i].val.str; *c != '\0'; c++) {
            *buffer_ptr++ = *c;
        }
	}
	*buffer_ptr = '\0';

	l->argc = 1;
	LineArg *la = &l->argv[0];
	la->val.str = unified_arg_str;
	la->type = ARG_TYPE_UNPROCESSED;

    char *arg_dup;
    char *reg_name;
    const Register *reg;
        // first try to figure out what kind of forced argument this is
    char *c = la->val.str;
    //la->val.str = NULL;
    char *tmp_c;
    switch (*c) {
    case '#':
        l->address_mode |= FLAG(ADDR_MODE_IMMEDIATE);
        c++;
        break;
    case '>':
        l->address_mode |= FLAG(ADDR_MODE_EXTENDED);
        c++;
        break;
    case '<':
        l->address_mode |= FLAG(ADDR_MODE_DIRECT);
        c++;
        break;
    case '[':
        l->address_mode |= FLAG(ADDR_MODE_INDIRECT);
        c++;
        for (tmp_c = c; *tmp_c != '\0' && *tmp_c != ']'; tmp_c++);
        if (*tmp_c == '\0') {
            fail("Unterminated bracket for indirect addressing.\n");
        }
        *tmp_c = '\0';
        tmp_c++;
        while(*tmp_c == ' ' || *tmp_c == '\t') {
            tmp_c++;
        }
        if (*tmp_c != ',' && *tmp_c != '\0') {
            fail("Unexpected characters after indirect argument.\n");
        }
        break;
    default:
        break;
    }

    la->val.str = c;
    buffer_ptr = c;
    struct {
        enum arg_type type;
        union {
            const Register *reg;
            struct token *expr;
        } val;
        int8_t pre_inc;
        int8_t post_inc;
    } arg_part = {
        ARG_TYPE_UNPROCESSED,
        NULL
    };

    bool end = false;
    while (!end) {
        switch (*c) {
        case ' ':
        case '\t':
            if (buffer_ptr == c) {
                buffer_ptr++;
            }
            break;
        case ',':
        case '\0':
            if (*c == '\0') {
                end = true;
            } else {
                *c = '\0';
            }

            // This doesnt quite work
            while (*buffer_ptr == '-' || *buffer_ptr == '+') {
                if (*buffer_ptr == '-') {
                    arg_part.pre_inc--;
                } else if (*buffer_ptr == '+') {
                    arg_part.pre_inc++;
                }
                buffer_ptr++;
            }

            /*for (tmp_c = c, tmp_c--; *tmp_c == '-' || *tmp_c == '+'; tmp_c--) {
                if (*buffer_ptr == '-') {
                    arg_part.pre_inc--;
                } else if (*buffer_ptr == '+') {
                    arg_part.pre_inc++;
                }
                *tmp_c = '\0';
            }*/

            arg_dup = strdup(buffer_ptr);
            if (arg_dup == NULL) {
                die("Failed to allocate new buffer for argument piece.\n");
            }
            arg_dup = str_trim(arg_dup);

            reg_name = strdup(arg_dup);
            if (reg_name == NULL) {
                die("Failed to allocate new buffer for register name.\n");
            }
            reg_name = str_to_upper(reg_name);

            if (*arg_dup == '\0') {
                arg_part.type = ARG_TYPE_EXPRESSION;
                arg_part.val.expr = parse_expression("0");
            } else if ((reg = find_reg(reg_name)) != NULL) {
                arg_part.type = ARG_TYPE_REGISTER;
                arg_part.val.reg = reg;
            } else {
                arg_part.type = ARG_TYPE_EXPRESSION;
                arg_part.val.expr = parse_expression(arg_dup);
            }

            free(reg_name);
            free(arg_dup);

            if (!end) { // not last argument
                // not last argument, check if indexed, handle appropriately
                l->address_mode |= FLAG(ADDR_MODE_INDEXED);   // more than one argument means indexed
                switch (arg_part.type) {
                case ARG_TYPE_EXPRESSION:
                    la->type = ARG_TYPE_INDEX_CONSTANT;
                    la->val.indexed.offset.expression = arg_part.val.expr;
                    break;
                case ARG_TYPE_REGISTER:
                    la->type = ARG_TYPE_INDEX_REGISTER;
                    la->val.indexed.offset.reg = arg_part.val.reg;
                    break;
                default:
                    fail("Invalid type for index offset.\n");
                    break;
                }
            } else {
                // last argument, check if arg type is indexed
                if (l->address_mode & FLAG(ADDR_MODE_INDEXED)) {
                    if (arg_part.type != ARG_TYPE_REGISTER) {
                        fail("Indexed addressing base must be a register.\n");
                    }
                    la->val.indexed.base = arg_part.val.reg;
                } else {
                    switch (l->address_mode) {
                    case ADDR_MODE_IMMEDIATE:
                    case ADDR_MODE_EXTENDED:
                    case ADDR_MODE_DIRECT:
                        if (arg_part.type != ARG_TYPE_EXPRESSION) {
                            fail("Immediate argument must be an expression.\n");
                        }
                        la->type = ARG_TYPE_EXPRESSION;
                        la->val.rpn_expr = arg_part.val.expr;
                        break;
                    //case ADDR_MODE_
                    }
                }
            }

            buffer_ptr = c, buffer_ptr++;
            break;
        }

        c++;
    }

proceed:
    ;

	//const Instruction **i = NULL;

	//const MC6x09_Instruction *i = NULL;
//	for (const Instruction **i = configuration.arch->instructions; *i != NULL; i++) {
//		if (!((*i)->architectures & FLAG(configuration.arch->value))) {
//			goto next_instruction;
//		}
//        instr_mnem = (*i)->mnemonic;
//        line_mnemonic = l->mnemonic;
//        while (*instr_mnem != '\0' && *line_mnemonic != '\0') {
//            if (*instr_mnem++ != *line_mnemonic++) {
//                goto next_instruction;
//            }
//        }
//
//        switch ((*i)->arg_order) {
//        case ARG_ORDER_NONE:
//        case ARG_ORDER_INTERREG:
//            if (*line_mnemonic != '\0') {
//                goto next_instruction;
//            }
//            if ((*i)->arg_order == ARG_ORDER_NONE) {
//                l->address_mode = ADDR_MODE_INHERENT;
//                goto instruction_found;
//            }
//
//            // check that the two arguments are registers and assign the argument
//            if (l->argc != 2) {
//                goto next_instruction;
//            }
//            for (size_t j = 0; j < l->argc; j++) {
//                if (l->argv[j].type != ARG_TYPE_UNPROCESSED) {
//                    goto next_instruction;
//                }
//                for (reg = &(configuration.arch->registers[1]); reg->name[0] != '\0'; reg++) {
//                    if (streq(l->argv[j].val.str, reg->name)) {
//                        l->argv[j].type = ARG_TYPE_REGISTER;
//                        l->argv[j].val.indexed.base = reg;
//                        break;
//                    }
//                }
//                if (l->argv[j].type != ARG_TYPE_REGISTER) {
//                    goto next_instruction;
//                }
//            }
//            l->address_mode = ADDR_MODE_INTERREGISTER;
//            goto instruction_found;
//
//            break;
//        case ARG_ORDER_FROM_REG:
//        case ARG_ORDER_TO_REG:
//            for (reg = &(configuration.arch->registers[1]); reg->name[0] != '\0'; reg++) {
//                if (streq(line_mnemonic, reg->name) &&
//                    (reg->arcs & FLAG(configuration.arch->value)) &&
//                    ((instruction_reg = instruction_supports_reg(*i, reg)) != NULL)) {
//                    goto evaluate_args;
//                    //goto instruction_found;
//                }
//            }
//            goto next_instruction;
//
//evaluate_args:
//            if (l->argc < 1) {
//                goto next_instruction;
//            }
//            if (l->argv->type != ARG_TYPE_UNPROCESSED) {
//                goto next_instruction;
//            }
//            argcpy = salloc(sizeof(LineArg));
//            memcpy(argcpy, l->argv, sizeof(LineArg));
//
//            char *c = l->argv->val.str;
//            char *tmp;
//            switch (*c) {
//            case '>':
//                l->address_mode |= FLAG(ADDR_MODE_EXTENDED);
//                c++;
//                break;
//            case '<':
//                l->address_mode |= FLAG(ADDR_MODE_DIRECT);
//                c++;
//                break;
//            case '#':
//                l->address_mode |= FLAG(ADDR_MODE_IMMEDIATE);
//                c++;
//                break;
//            case '[':
//                l->address_mode |= FLAG(ADDR_MODE_INDIRECT);
//                c++;
//                for (tmp = c; *tmp != '\0' && *tmp != ']'; tmp++);
//                if (*tmp == '\0') {
//                    fail("Missing terminating ']' character for indirect addressing mode.\n");
//                }
//
//                break;
//            /*case ',':
//                l->address_mode = ADDR_MODE_INDEXED;
//                c++;
//                break;*/
//            default:
//                break;
//            }
//
//            printdf("%s\n", c);
//            for (; *c != '\0'; c++) {
//                switch (*c) {
//                case ',':
//                    if (l->address_mode == ADDR_MODE_INDIRECT) {
//
//                    }
//                    if (l->address_mode != ADDR_MODE_INVALID) {
//                        fail("Cannot have indexed address mode");
//                    }
//                    break;
//                //case ''
//                }
//            }
//            //}
//            goto instruction_found;
//            break;
//        }
//
//next_instruction:
//		continue;
//	}
//
//	fail("Instruction not found.\n");
//
//instruction_found:
//    ;
//	//while (0);
//
//    prepare_line(l);
//

    Data *data = init_data(salloc(sizeof(Data)));

	//printdf("Instruction Register is %s\n", instruction_reg ? instruction_reg->reg->name : "NONE");
	//configuration.arch->process_line(l, instruction_reg, data);

	add_data(data);
}

static void prepare_line_att(Line *l)
{

}

static void prepare_line_intel(Line *l)
{

}
