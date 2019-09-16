#include <stdio.h>
#ifdef _WIN32
#include <getopt.h>
#else
#include <unistd.h>
#endif
#include <as.h>
#include <targets.h>
#include <pseudo.h>
#include <util.h>
#include <arithmetic.h>

#define LINEBUFFERSIZE (256)
char buffer[LINEBUFFERSIZE];

static void configure(int argc, char *argv[]);
//static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void resolve_mnemonic_type(Line *l);
static void set_syntax_parser();
static void parse_instruction_motorola(Line *l);
static void parse_instruction_att(Line *l);
static void parse_instruction_intel(Line *l);

Config configuration;

//FILE *in;
FILE *out;
size_t address;
size_t address_mask;    // bits to mask the address to;
size_t line_num;
void (*parse_instruction)(Line *l);
//SymTab *undefined_symtab;

Architecture **architectures[] = { TARGETS };

int main(int argc, char **argv)
{
#if 0
    //struct token *rpn_expr = parse_expression("1 *(2 +3)");
    struct token *rpn_expr = parse_expression("(_start * 3) + tmp");
    //struct token *rpn_expr = parse_expression("(_start * 3 + tmp");
    print_token_list(rpn_expr);
    printf("%d\n", arithmetic_status_code);
    free_token_chain(rpn_expr);
    return 0;
#endif

    INIT_TARGETS();

    FILE *in;
    //out = stdout;
    configuration.arch = *architectures[0];
    configuration.in_fnames = NULL;
    configuration.in_fnamec = 0;
    configuration.syntax = configuration.arch->default_syntax;
	configuration.out_fname = "a.out";
    address = 0;
    line_num = 1;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    //free(configuration.out_fname);
    configuration.out_fname = NULL;

    init_address_mask();
    set_syntax_parser();

    init_data_table();
    init_symbol_table();

    Line *l = salloc(sizeof(Line));

    if (configuration.in_fnamec < 0) {
        die("Invalid number of command line arguments.\n");
    }
    else if (configuration.in_fnamec == 0) {
        assemble(stdin, l);
    }
    else {
        size_t i;
        for (i = 0; i < configuration.in_fnamec; i++) {
            if ((in = fopen(configuration.in_fnames[i], "r")) == NULL) {
                die("Failed to open input file %s\n", configuration.in_fnames[i]);
            }
            assemble(in, l);
            fclose(in);
        }
    }
    sfree(l);

    printdf("SYMBOLS\n");
    Symbol *sym = symtab->first;
    while (sym != NULL) {
        printdf("label: %s = %ld\n", sym->label, sym->value);

        sfree(sym->label);
        sym->label = NULL;
        Symbol *tmp = sym;
        sym = sym->next;
        sfree(tmp);
    }
    //sfree(symtab->first->label);
    sfree(symtab);
    symtab = NULL;
    sym = NULL;

    printdf("DATATAB\n");
    Data *data = datatab->first;
    while (data != NULL) {
        printdf("data address: %lX,  ", data->address);
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
            int i;
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

    DESTROY_TARGETS();

#if defined(_WIN32) && !defined(NDEBUG)
    getc(stdin);
#endif

	return 0;
}

void init_address_mask()
{
    address_mask = 0;
    int i;
    for (i = 0; i < configuration.arch->bytes_per_address * configuration.arch->byte_size; i++) {
        if (i) {
            address_mask <<= 1u;
        }
        address_mask |= 1u;
    }
    printdf("Address mask: %lX\n", address_mask);
}

void assemble(FILE *in, Line *l)
{
a     while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {
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
            size_t i;
            for (i = 0; i < l->argc; i++) {
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
        line_num++;
    }
}

static void configure(int argc, char *argv[])
{
    int c;

    while ((c = getopt(argc, argv, "hm:o:f:")) != -1) {
        switch (c) {
        case 'm':	// architecture
            configuration.arch = str_to_arch(optarg);
            if (configuration.arch == NULL) {
                free(configuration.out_fname);
                die("Unsupported architecture: %s\n", optarg);
            }
            break;
        case 'o':	// output file
            free(configuration.out_fname);
            if ((configuration.out_fname = strdup(optarg)) == NULL) {
                die("Failed to allocate new output file name");
            }
            break;
        case 'f':	// output file format
            break;
        case 'h':
            die("Usage: %s [-m arch] [-o outfile] [-f outformat]\n", argv[0]);
            break;
        default:
            goto exit;
            break;
        /*case '?':
        default:
            die("Usage: %s [-m arch] [-o outfile] [-f outformat]\n", argv[0]);
            break;*/
        }
    }
exit:

    printdf("argcount = %d\n", argc - optind);

    configuration.in_fnames = argv + sizeof(char) * optind;
    configuration.in_fnamec = argc - optind;
}

Architecture *str_to_arch(const char arch_name[])
{
    Architecture ***a;
    for (a = architectures; *a != NULL; a++) {
        if (streq(arch_name, (**a)->name)) {
            return **a;
        }
    }
    return NULL;
}

static void parse_line(Line *l, char *buffer)
{
    register char *c;
    LineArg *la = NULL;
    enum arg_type arg_type = ARG_TYPE_UNPROCESSED;
    for (c = buffer; *c != '\0'; c++) {
        switch (*c) {
        case '"':
            if (l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) {
                break;
            }
            else if (!(l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) && c != buffer) {
                fail("Quotes must occur at the beginning of a field.\n");
            }
            l->line_state ^= FLAG(LINE_STATE_DOUBLE_QUOTE);
            if (l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) {
                arg_type = ARG_TYPE_STRING;
                buffer++;
            }
            else {
                *c = '\0';
            }
            break;
        case '\'':
            if (l->line_state & FLAG(LINE_STATE_DOUBLE_QUOTE)) {
                break;
            }
            else if (!(l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) && c != buffer) {
                fail("Quotes must occur at the beginning of a field.\n");
            }
            l->line_state ^= FLAG(LINE_STATE_SINGLE_QUOTE);
            if (l->line_state & FLAG(LINE_STATE_SINGLE_QUOTE)) {
                arg_type = ARG_TYPE_STRING;
                buffer++;
            }
            else {
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
            la->state = ARG_STATE_CLEAR;
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
            }
            else {
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

//        case '\n':
//        case '\t':
//        case ' ':
//        case ',':   // Might be worth making this separate
//            if (!(l->line_state & LINE_STATE_SINGLE_QUOTE || l->line_state & LINE_STATE_DOUBLE_QUOTE) && !(l->line_state & LINE_STATE_BRACKET)) {
//                if (c != buffer) {
//                    if (!(l->line_state & LINE_STATE_MNEMONIC)) { // if mnemonic is not set
//                        if (*c == ',') {
//                            fail("Unexpected ',' character.\n");
//                        }
//                        else if (arg_type == ARG_TYPE_STRING) {
//                            fail("Mnemonic cannot be string literal.\n");
//                        }
//                        l->mnemonic = buffer;
//                        l->line_state |= LINE_STATE_MNEMONIC;
//                    }
//                    else { // Argument
//                        if (l->argc == l->arg_buf_size) {
//                            l->arg_buf_size += 2;
//                            //srealloc(l->argv, sizeof(char *) * l->arg_buf_size);
//                            l->argv = srealloc(l->argv, sizeof(LineArg) * l->arg_buf_size);
//                        }
//                        LineArg *la = &(l->argv[l->argc++]);
//                        la->type = arg_type;
//                        la->val.str = buffer;
//                        arg_type = ARG_TYPE_UNPROCESSED;
//                    }
//
//                    *c = '\0';
//                    buffer = c;
//                }
//                buffer++;
//            }
//            break;
        case ':':
            if (l->line_state & FLAG(LINE_STATE_LABEL)) {
                fail("Invalid label.\n");
            }
            else if (l->line_state & FLAG(LINE_STATE_MNEMONIC)) {
                fail("Label must occur at the beginning of a line.\n");
            }
            else if (arg_type == ARG_TYPE_STRING) {
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
        //die("Error on line %ld. Unmatched bracket\n", line_num);
        fail("Unmatched bracket.\n");
    }
}

static void resolve_mnemonic_type(Line *line)
{
    struct pseudo_instruction *pseudo_op;

    if (line->mnemonic[0] == '\0') {
        return;
    }
    else if (line->mnemonic[0] == '.') {
        prepare_line(line);
        parse_pseudo_op(line);
    }
    else if ((pseudo_op = get_pseudo_op(line)) != NULL) {
        prepare_line(line);
        pseudo_op->process(line);
    }
    else {
        //configuration.arch->parse_instruction(line);
		parse_instruction(line);
    }
}

static inline const struct instruction_register *instruction_supports_reg(const Instruction *instruction, const Register *reg)
{
	const struct instruction_register *ir = NULL;
	for (ir = instruction->opcodes; ir->reg != NULL; ir++) {
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
        parse_instruction = &parse_instruction_motorola;
        break;
    case SYNTAX_ATT:
        fail("AT&T instruction syntax is not yet implemented.\n");
        parse_instruction = &parse_instruction_att;
        break;
    case SYNTAX_INTEL:
        fail("Intel instruction syntax is not yet implemented.\n");
        parse_instruction = &parse_instruction_intel;
        break;
    case SYNTAX_UNKNOWN:
        fail("Unrecognized syntax.\n");
    }
}

static void parse_instruction_motorola(Line *l)
{
	const char *instr_mnem = NULL;
	const char *line_mnemonic = NULL;
	const Register *reg = NULL;
	const struct instruction_register *instruction_reg = NULL;
	struct LineArg *argcpy = NULL;  // store a copy of the arguments in case processing fails
    size_t j;

	const Instruction **i = NULL;

	//const MC6x09_Instruction *i = NULL;
	for (i = configuration.arch->instructions; *i != NULL; i++) {
		if (!((*i)->architectures & FLAG(configuration.arch->value))) {
			goto next_instruction;
		}
        instr_mnem = (*i)->mnemonic;
        line_mnemonic = l->mnemonic;
        while (*instr_mnem != '\0' && *line_mnemonic != '\0') {
            if (*instr_mnem++ != *line_mnemonic++) {
                goto next_instruction;
            }
        }

        switch ((*i)->arg_order) {
        case ARG_ORDER_NONE:
        case ARG_ORDER_INTERREG:
            if (*line_mnemonic != '\0') {
                goto next_instruction;
            }
            if ((*i)->arg_order == ARG_ORDER_NONE) {
                l->address_mode = ADDR_MODE_INHERENT;
                goto instruction_found;
            }

            // check that the two arguments are registers and assign the argument
            if (l->argc != 2) {
                goto next_instruction;
            }
            for (j = 0; j < l->argc; j++) {
                if (l->argv[j].type != ARG_TYPE_UNPROCESSED) {
                    goto next_instruction;
                }
                for (reg = &(configuration.arch->registers[1]); reg->name[0] != '\0'; reg++) {
                    if (streq(l->argv[j].val.str, reg->name)) {
                        l->argv[j].type = ARG_TYPE_REGISTER;
                        l->argv[j].val.reg = reg;
                        break;
                    }
                }
                if (l->argv[j].type != ARG_TYPE_REGISTER) {
                    goto next_instruction;
                }
            }
            l->address_mode = ADDR_MODE_INTERREGISTER;
            goto instruction_found;

            break;
        case ARG_ORDER_FROM_REG:
        case ARG_ORDER_TO_REG:
            for (reg = &(configuration.arch->registers[1]); reg->name[0] != '\0'; reg++) {
                if (streq(line_mnemonic, reg->name) &&
                    (reg->arcs & FLAG(configuration.arch->value)) &&
                    ((instruction_reg = instruction_supports_reg(*i, reg)) != NULL)) {
                    goto evaluate_args;
                    //goto instruction_found;
                }
            }
            goto next_instruction;

evaluate_args:
            if (l->argc != 1) {
                goto next_instruction;
            }
            //for (j = 0; j < l->argc; j++) {
            if (l->argv->type != ARG_TYPE_UNPROCESSED) {
                goto next_instruction;
            }
            argcpy = salloc(sizeof(LineArg));
            memcpy(argcpy, l->argv, sizeof(LineArg));

            const char *c = l->argv->val.str;
            switch (*c) {
            case '>':
                l->address_mode = ADDR_MODE_EXTENDED;
                c++;
                break;
            case '<':
                l->address_mode = ADDR_MODE_DIRECT;
                c++;
                break;
            case '#':
                l->address_mode = ADDR_MODE_IMMEDIATE;
                c++;
                break;
            /*case ',':
                l->address_mode = ADDR_MODE_INDEXED;
                c++;
                break;*/
            default:
                break;
            }

            printdf("%s\n", c);
            for (; *c != '\0'; c++) {
                switch (*c) {
                case ',':
                    if (l->address_mode == ADDR_MODE_INDIRECT) {

                    }
                    if (l->address_mode != ADDR_MODE_INVALID) {
                        fail("Cannot have indexed address mode")
                    }
                    break;
                //case ''
                }
            }
            //}
            goto instruction_found;
            break;
        }

next_instruction:
		continue;
	}

	fail("Instruction not found.\n");

instruction_found:
    ;
	//while (0);

    prepare_line(l);

	Data *data = init_data(salloc(sizeof(Data)));
	/*assembled->next = NULL;
	assembled->type = DATA_TYPE_NONE;
	assembled->bytec = 0;
	assembled->contents.bytes = NULL;*/
	//process_instruction(l, instruction_reg, reg, assembled);
	printdf("Instruction Register is %s\n", instruction_reg ? instruction_reg->reg->name : "NONE");

	configuration.arch->process_line(l, instruction_reg, data);

	add_data(data);
}

static void parse_instruction_att(Line *l)
{

}

static void parse_instruction_intel(Line *l)
{

}
