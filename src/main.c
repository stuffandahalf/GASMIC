#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <getopt.h>
#define strdup _strdup
#else
#include <unistd.h>
#endif
#include <as.h>
#include <targets.h>
#include <util.h>

#define LINEBUFFERSIZE (256)
char buffer[LINEBUFFERSIZE];

static void configure(int argc, char *argv[]);
char *str_to_upper(char str[]);
//static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void parse_mnemonic(Line *l);
static void parse_instruction(Line *l);

Config configuration;
    
//FILE *in;
FILE *out;
size_t address;
size_t address_mask;    // bits to mask the address to;
size_t line_num;
//SymTab *undefined_symtab;

Architecture **architectures[] = { TARGETS };

int main(int argc, char **argv) {
    INIT_TARGETS();

    if ((configuration.out_fname = strdup("a.out")) == NULL) {
        die("Failed to duplicate string\n");
    }
    FILE *in;
    //out = stdout;
    configuration.arch = *architectures[0];
    configuration.in_fnames = NULL;
    configuration.in_fnamec = 0;
    configuration.syntax = configuration.arch->default_syntax;
    address = 0;
    line_num = 1;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    free(configuration.out_fname);
    configuration.out_fname = NULL;

    address_mask = 0;
    int i;
    for (i = 0; i < configuration.arch->bytes_per_address * configuration.arch->byte_size; i++) {
        if (i) {
            address_mask <<= 1;
        }
        address_mask |= 1;
    }
    printdf("Address mask: %lX\n", address_mask);

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
        unsigned int i;
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
        printdf("label: %s\t%ld\n", sym->label, sym->value);

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
        case DATA_TYPE_LABEL:
            printdf("%" PRIu8 " bytes label \"%s\"\n", data->bytec, data->contents.symbol);
            sfree(data->contents.symbol);
            break;
        case DATA_TYPE_BYTES:
#ifndef NDEBUG
            printf("%" PRIu8 " bytes ", data->bytec);
            int i;
            for (i = 0; i < data->bytec; i++) {
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

#ifdef _WIN32
    getc(stdin);
#endif

	return 0;
}

void assemble(FILE *in, Line *l) {
    while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {
        if (buffer[0] != '\0' || buffer[0] != '\n') {
            l->line_state = 0;
            l->arg_buf_size = 2;
            //l->argv = salloc(sizeof(char *) * l->arg_buf_size);
            l->argv = salloc(sizeof(LineArg) * l->arg_buf_size);
            l->argc = 0;
            
            parse_line(l, buffer);
            
#ifndef NDEBUG
            //printf("%s\t%s", l->label, l->mnemonic);
            if (l->line_state & LINE_STATE_LABEL) {
                printf("%s", l->label);
            }
            if (l->line_state & LINE_STATE_MNEMONIC) {
                if (l->line_state & LINE_STATE_LABEL) {
                    puts("\t");
                }
                printf("%s", l->mnemonic);
            }
            unsigned int i;
            for (i = 0; i < l->argc; i++) {
                printf("\t%s", l->argv[i].val.str);
            }
            puts("");
#endif
            if (l->line_state & LINE_STATE_LABEL) {      // If current line has a label
                add_label(l);
            }
            if (l->line_state & LINE_STATE_MNEMONIC) {   // If current line has a mnemonic
                str_to_upper(l->mnemonic);
                parse_mnemonic(l);
            }
            
            sfree(l->argv);
        }
        line_num++;
    }
}

static void configure(int argc, char *argv[]) {
    int c;

    while ((c = getopt(argc, argv, "m:o:f:")) != -1) {
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
        case '?':
        default:
            die("Usage: %s [-m arch] [-o outfile] [-f outformat]\n", argv[0]);
            break;
        }
    }
    
    printdf("argcount = %d\n", argc - optind);
    
    configuration.in_fnames = argv + sizeof(char) * optind;
    configuration.in_fnamec = argc - optind;
}

Architecture *str_to_arch(const char arch_name[]) {
    Architecture ***a;
    for (a = architectures; *a != NULL; a++) {
        if (streq(arch_name, (**a)->name)) {
            return **a;
        }
    }
    return NULL;
}

char *str_to_upper(char str[]) {
    register char *c;
    for (c = str; *c != '\0'; c++) {
        if (*c >= 'a' && *c <= 'z') {
            *c += 'A' - 'a';
        }
        else if (*c == '\n') {
            *c = '\0';
        }
    }
    return str;
}

static void parse_line(Line *l, char *buffer) {    
    register char *c;
    for (c = buffer; *c != '\0'; c++) {
        switch (*c) {
        case '"':
        case '\'':
            l->line_state ^= LINE_STATE_QUOTE;
            if (l->line_state & LINE_STATE_QUOTE) {
                //buffer++;
            }
            else {
                *c = '\0';
            }
            break;
        case ']':
            if (!(l->line_state & LINE_STATE_BRACKET)) {
                fail("']' requires '[' first.");
            }
        case '[':
            l->line_state ^= LINE_STATE_BRACKET;
            break;
        case '\n':
        case '\t':
        case ' ':
        case ',':   // Might be worth making this seperate
            if (!(l->line_state & LINE_STATE_QUOTE) && !(l->line_state & LINE_STATE_BRACKET)) {
                if (c != buffer) {
                    if (!(l->line_state & LINE_STATE_MNEMONIC)) { // if mnemonic is not set
                        if (*c == ',') {
                            fail("Unexpected ',' character.\n");
                        }
                        l->mnemonic = buffer;
                        l->line_state |= LINE_STATE_MNEMONIC;
                    }
                    else { // Argument
                        if (l->argc == l->arg_buf_size) {
                            l->arg_buf_size += 2;
                            //srealloc(l->argv, sizeof(char *) * l->arg_buf_size);
                            l->argv = srealloc(l->argv, sizeof(LineArg) * l->arg_buf_size);
                        }
                        LineArg *la = &(l->argv[l->argc++]);
                        la->type = ARG_TYPE_STR;
                        la->val.str = buffer;
                    }
                    
                    *c = '\0';
                    buffer = c;
                }
                buffer++;
            }
            break;
        case ':':
            if (l->line_state & LINE_STATE_MNEMONIC) {
                fail("Label must occur at the beginning of a line.\n");
            }
            l->label = buffer;
            *c = '\0';
            buffer = c;
            buffer++;
            
            printdf("parsed literal label = %s\n", l->label);
            l->line_state |= LINE_STATE_LABEL;
            break;
        case ';':
            return;
        }
        //buffer++;     // Why doesnt this work?
    }
    if (l->line_state & LINE_STATE_QUOTE) {
        fail("Unmatched quote.\n");
    }
    if (l->line_state & LINE_STATE_BRACKET) {
        //die("Error on line %ld. Unmatched bracket\n", line_num);
        fail("Unmatched bracket.\n");
    }
}


static void parse_mnemonic(Line *line) {
    struct pseudo_instruction *pseudo_op;
    
    if (line->mnemonic[0] == '.') {
        parse_pseudo_op(line);
    }
    else if ((pseudo_op = get_pseudo_op(line)) != NULL) {
        pseudo_op->process(line);
    }
    else {
        //configuration.arch->parse_instruction(line);
		parse_instruction(line);
    }
}

static inline const struct instruction_register *instruction_supports_reg(const Instruction *instruction, const Register *reg) {
	const struct instruction_register *ir = NULL;
	for (ir = instruction->opcodes; ir->reg != NULL; ir++) {
		if (streq(reg->name, ir->reg->name)) {
			return ir;
		}
	}
	return NULL;
}
static void parse_instruction(Line *l) {
	const char *mnem = NULL;
	const char *line = NULL;
	const Register *reg = NULL;
	const struct instruction_register *instruction_reg = NULL;

	const Instruction **i = NULL;

	//const MC6x09_Instruction *i = NULL;
	for (i = configuration.arch->instructions; i != NULL; i++) {
		if (!((*i)->architectures & configuration.arch->value)) {
			goto next_instruction;
		}

		switch (configuration.syntax) {
		case SYNTAX_MOTOROLA:
			mnem = (*i)->mnemonic;
			line = l->mnemonic;
			while (*mnem != '\0' && *line != '\0') {
				if (*mnem++ != *line++) {
					goto next_instruction;
				}
			}

			switch ((*i)->arg_order) {
			case ARG_ORDER_NONE:
			case ARG_ORDER_INTERREG:
				if (*line != '\0') {
					goto next_instruction;
				}
				goto instruction_found;
				break;
			case ARG_ORDER_FROM_REG:
			case ARG_ORDER_TO_REG:
				for (reg = &(configuration.arch->registers[1]); reg->name[0] != '\0'; reg++) {
					if (streq(line, reg->name) && (reg->arcs & configuration.arch->value) && ((instruction_reg = instruction_supports_reg(*i, reg)) != NULL)) {
						goto instruction_found;
					}
				}
				break;
			}
			break;
		case SYNTAX_INTEL:
		case SYNTAX_ATT:
			fail("Selected syntax is not yet implemented.\n");
			break;
		case SYNTAX_UNKNOWN:
		default:
			fail("Invalid syntax selected.\n");
			break;
		}

next_instruction:
		continue;
	}

	fail("Instruction not found.\n");

instruction_found:
    ;
	//while (0);

	Data *assembled = init_data(salloc(sizeof(Data)));
	/*assembled->next = NULL;
	assembled->type = DATA_TYPE_NONE;
	assembled->bytec = 0;
	assembled->contents.bytes = NULL;*/
	//process_instruction(l, instruction_reg, reg, assembled);
	printdf("Instruction Register is %s\n", instruction_reg ? instruction_reg->reg->name : "NONE");

	add_data(assembled);
}