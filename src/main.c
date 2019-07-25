#include <stdio.h>
#include <string.h>
#include <stdint.h>
#ifdef _WIN32
#include <getopt.h>
#define strdup _strdup
#else
#include <unistd.h>
#endif
#include <as.h>
#include <targets/6x09/arch.h>


#define LINEBUFFERSIZE (256)
char buffer[LINEBUFFERSIZE];

static void configure(int argc, char *argv[]);
char *str_to_upper(char str[]);
//static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void add_label(Line *l);
static void parse_mnemonic(Line *l);

Config configuration;
    
//FILE *in;
FILE *out;
addr_t address;
size_t line_num;
SymTab *symtab;
DataTab *datatab;
//SymTab *undefined_symtab;

#ifndef NDEBUG
extern void smem_diagnostic(void);
#endif

int main(int argc, char **argv) {
	if ((configuration.out_fname = strdup("a.out")) == NULL) {
        die("Failed to duplicate string\n");
    }
    FILE *in;
    //out = stdout;
    configuration.arch = architectures;
    configuration.in_fnames = NULL;
    configuration.in_fnamec = 0;
    configuration.syntax = DEFAULT_SYNTAX;
    address = 0;
    line_num = 1;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    free(configuration.out_fname);
    configuration.out_fname = NULL;
    
    symtab = salloc(sizeof(SymTab));
    symtab->first = NULL;
    symtab->last = NULL;
    
    datatab = salloc(sizeof(DataTab));
    datatab->first = NULL;
    datatab->last = NULL;
    
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
            printdf("%d bytes label \"%s\"\n", data->bytec, data->contents.symbol);
            free(data->contents.symbol);
            break;
        case DATA_TYPE_BYTES:
#ifndef NDEBUG
            printf("%d bytes ", data->bytec);
            int i;
            for (i = 0; i < data->bytec; i++) {
                printf("%X ", data->contents.bytes[i]);
            }
            printf("\n");
#endif
            sfree(data->contents.bytes);
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
        case 'm':
            configuration.arch = str_to_arch(optarg);
            if (configuration.arch == NULL) {
                free(configuration.out_fname);
                die("Unsupported architecture: %s\n", optarg);
            }
            break;
        case 'o':
            free(configuration.out_fname);
            if ((configuration.out_fname = strdup(optarg)) == NULL) {
                die("Failed to allocate new output file name");
            }
            break;
        case 'f':
            break;
        case 0:
            break;
        }
    }
    
    printdf("argcount = %d\n", argc - optind);
    
    configuration.in_fnames = argv + sizeof(char) * optind;
    configuration.in_fnamec = argc - optind;
}

Architecture *str_to_arch(const char arch_name[]) {
    Architecture *a;
    for (a = architectures; a->name[0] != '\0'; a++) {
        if (streq(arch_name, a->name)) {
            return a;
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

static void add_label(Line *l) {
    Symbol *sym = salloc(sizeof(Symbol));    
    
    sym->next = NULL;
    if (l->label[0] == '.') {
        if (symtab->last == NULL) {
            fail("Local label cannot be defined before any non-local labels.\n");
        }
        
        sym->label = salloc(sizeof(char) * (strlen(symtab->last_parent->label) + strlen(l->label) + 1));
        
        register char *label = sym->label;
        register char *c;
        for (c = symtab->last_parent->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        for (c = l->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        *label = '\0';
        
        l->label = sym->label;
    }
    else {
        sym->label = salloc(strlen(l->label) + sizeof(char));
        strcpy(sym->label, l->label);
        symtab->last_parent = sym;
    }
    
    printdf("Parsed label = %s\n", sym->label);
    
    sym->value = address;
    
    Symbol *test_sym;
    for (test_sym = symtab->first; test_sym != NULL; test_sym = test_sym->next) {
        if (streq(test_sym->label, sym->label)) {
            fail("Symbol \"%s\" is already defined.\n", test_sym->label);
        }
    }
    
    if (symtab->first == NULL) {
        symtab->first = sym;
    }
    else {
        symtab->last->next = sym;
    }
    symtab->last = sym;
}

void add_data(Data *data) {
    if (datatab->first == NULL) {
        datatab->first = data;
    }
    else {
        datatab->last->next = data;
    }
    datatab->last = data;
    Data *next = datatab->last->next;
    while (next != NULL) {
        datatab->last = next;
        next = next->next;
    }
}

static void parse_mnemonic(Line *line) {
    struct pseudo_instruction *pseudo_op;
    
    switch(line->mnemonic[0]) {
    case '.':
        parse_pseudo_op(line);
        break;
    default:
        if ((pseudo_op = get_pseudo_op(line)) != NULL) {
            pseudo_op->process(line);
        }
        else {
            configuration.arch->parse_instruction(line);
        }
        break;
    }
}
