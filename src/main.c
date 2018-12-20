#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <as.h>
#include <arch.h>

#define LINEBUFFERSIZE 256
char buffer[LINEBUFFERSIZE];

static void configure(int argc, char *argv[]);
static void assemble(FILE *in, Line *l);
static Architecture *str_to_arch(const char arch_name[]);
static char *str_to_upper(char str[]);
static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void add_label(Line *l);
static void parse_mnemonic(Line *l);

struct {
    char *out_fname;
    char **in_fnames;
    ssize_t in_fnamec;
    Architecture *arch;
} configuration;
    
//FILE *in;
FILE *out;
size_t address;
size_t line_num;
SymTab *symtab;
SymTab *undefined_symtab;

extern void smem_diagnostic();

int main(int argc, char **argv) {
    if ((configuration.out_fname = strdup("a.out")) == NULL) {
        die("Failed to duplicate string\n");
    }
    FILE *in;
    //out = stdout;
    configuration.arch = architectures;
    configuration.in_fnames = NULL;
    configuration.in_fnamec = 0;
    address = 0;
    line_num = 1;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    free(configuration.out_fname);
    configuration.out_fname = NULL;
    
    symtab = salloc(sizeof(SymTab));
    symtab->first = NULL;
    symtab->last = NULL;
    
    Line *l = salloc(sizeof(Line));
    
    if (configuration.in_fnamec < 0) {
        die("");
    }
    else if (configuration.in_fnamec == 0) {
        assemble(stdin, l);
    }
    else {
        int i;
        for (i = 0; i < configuration.in_fnamec; i++) {
            if ((in = fopen(configuration.in_fnames[i], "r")) == NULL) {
                die("Failed to open input file %s\n", configuration.in_fnames[i]);
            }
            assemble(in, l);
            fclose(in);
        }
    }
    sfree(l);
    
    puts("symtab");
    Symbol *sym = symtab->first;
    while (sym != NULL) {
        printf("label: %s\t%ld\n", sym->label, sym->value);
        
        sfree(sym->label);
        sym->label = NULL;
        Symbol *tmp = sym;
        sym = sym->next;
        sfree(tmp);
    }
    //sfree(symtab->first->label);
    sfree(symtab);
    symtab = NULL;
    
    //fclose(out);
    
	return 0;
}

void assemble(FILE *in, Line *l) {
    while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {        
        if (buffer[0] != '\0' || buffer[0] != '\n') {
            l->line_state = 0;
            l->arg_buf_size = 3;
            l->argv = salloc(sizeof(char *) * l->arg_buf_size);
            l->argc = 0;
            
            parse_line(l, buffer);
            
            #ifdef DEBUG
            //printf("%s\t%s", l->label, l->mnemonic);
            if (l->line_state & LABEL_STATE) {
                printf("%s", l->label);
            }
            if (l->line_state & MNEMONIC_STATE) {
                if (l->line_state & LABEL_STATE) {
                    puts("\t");
                }
                printf("%s", l->mnemonic);
            }
            int i;
            for (i = 0; i < l->argc; i++) {
                printf("\t%s", l->argv[i]);
            }
            puts("");
            #endif
            
            if (l->line_state & LABEL_STATE) {      // If current line has a label
                add_label(l);
            }
            if (l->line_state & MNEMONIC_STATE) {   // If current line has a mnemonic
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
    
    printf("argcount = %d\n", argc - optind);
    
    configuration.in_fnames = argv + sizeof(char) * optind;
    configuration.in_fnamec = argc - optind;
}

static Architecture *str_to_arch(const char arch_name[]) {
    for (Architecture *a = architectures; a->name[0] != '\0'; a++) {
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
            l->line_state ^= QUOTE_STATE;
            if (l->line_state & QUOTE_STATE) {
                buffer++;
            }
            else {
                *c = '\0';
            }
            break;
        case '\n':
        case '\t':
        case ' ':
        case ',':
            if (!(l->line_state & QUOTE_STATE)) {
                if (c != buffer) {
                    if (!(l->line_state & MNEMONIC_STATE)) { // if mnemonic is not set
                        if (*c == ',') {
                            die("Error on line %ld. Unexpected ',' character\n", line_num);
                        }
                        l->mnemonic = buffer;
                        l->line_state |= MNEMONIC_STATE;
                    }
                    else { // Argument
                        if (l->argc == l->arg_buf_size) {
                            l->arg_buf_size += 2;
                            srealloc(l->argv, sizeof(char *) * l->arg_buf_size);
                        }
                        l->argv[l->argc++] = buffer;
                    }
                    
                    *c = '\0';
                    buffer = c;
                }
                buffer++;
            }
            break;
        case ':':
            if (l->line_state & MNEMONIC_STATE) {
                die("Error on line %ld. Label must occur at the beginning of a line.\n", line_num);
            }
            l->label = buffer;
            *c = '\0';
            buffer = c;
            buffer++;
            
            #ifdef DEBUG
            printf("parsed literal label = %s\n", l->label);
            #endif
            l->line_state |= LABEL_STATE;
            break;
        case ';':
            return;
        }
        //buffer++;     // Why doesnt this work?
    }
}

static void add_label(Line *l) {
    Symbol *sym = salloc(sizeof(Symbol));
    sym->next = NULL;
    if (l->label[0] == '.') {
        if (symtab->last == NULL) {
            die("Error on line %ld. local label cannot be defined before any non-local labels.", line_num);
        }
        // Count characters
        size_t characters = 0;
        register char *c;
        for (c = symtab->last->label; *c != '.' && *c != '\0'; c++) {
            characters++;
        }
        for (c = l->label; *c != '\0'; c++) {
            characters++;
        }
        characters++;
        
        // Allocate that many bytes for the label
        sym->label = salloc(sizeof(char) * characters);
        
        // Save the complete label
        characters = 0;
        for (c = symtab->last->label; *c != '.' && *c != '\0'; c++) {
            sym->label[characters++] = *c;
        }
        for (c = l->label; *c != '\0'; c++) {
            sym->label[characters++] = *c;
        }
        sym->label[characters] = '\0';
        l->label = sym->label;
    }
    else {
        sym->label = salloc(strlen(l->label) + 1);
        strcpy(sym->label, l->label);
    }
    
    printf("Parsed label = %s\n", sym->label);
    
    sym->value = address;
    
    if (symtab->first == NULL) {
        symtab->first = sym;
    }
    else {
        symtab->last->next = sym;
    }
    symtab->last = sym;
}


static void parse_mnemonic(Line *line) {
    switch(line->mnemonic[0]) {
    case '.':
        parse_pseudo_op(line);
        break;
    default:
        //printf("process_instruction is at %p\n", configuration.arch->parse_instruction);
        configuration.arch->parse_instruction(line);
        printf("%p\n", &(configuration.arch->parse_instruction));
        
        break;
    }
}
