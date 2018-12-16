#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <as.h>
#include <arch.h>

#define LINEBUFFERSIZE 256
char buffer[LINEBUFFERSIZE];

static void configure(int argc, char *argv[]);
static Architecture *str_to_arch(const char arch_name[]);
static char *str_to_upper(char str[]);
static void trim_str(char str[]);
static void parse_line(Line *l, char *buffer);
static void add_label(Line *l);
static void parse_mnemonic(Line *l);

struct {
    char *out_fname;
    char **in_fnames;
    size_t in_fnamec;
    Architecture *arch;
} configuration;
    
FILE *in;
FILE *out;
size_t address;
size_t line_num;
SymTab *symtab;
SymTab *undefined_symtab;

int main(int argc, char **argv) {
    if ((configuration.out_fname = strdup("a.out")) == NULL) {
        die("Failed to duplicate string\n");
    }
    in = stdin;
    //out = stdout;
    configuration.arch = architectures;
    address = 0;
    line_num = 1;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    free(configuration.out_fname);
    configuration.out_fname = NULL;
    
    symtab = salloc(sizeof(SymTab));
    
    
    while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {        
        if (buffer[0] != '\0' || buffer[0] != '\n') {
            Line *l = salloc(sizeof(Line));
            parse_line(l, buffer);
            
            if (l->line_state & LABEL_STATE) {      // If current line has a label
                add_label(l);
            }
            if (l->line_state & MNEMONIC_STATE) {   // If current line has a mnemonic
                str_to_upper(l->mnemonic);
                /*Instruction *i = find_instruction(l->mnemonic);
                if (i != NULL) {
                    printf("%s, %X, %X\n", i->mne, i->base_opcode, i->regs);
                }*/
                parse_mnemonic(l);
            }
            
            //printf("address of l->argv is %p\n", l->argv);
            sfree(l->argv);
            //printf("address of l is %p\n", l);
            sfree(l);
        }
        line_num++;
    }
    
    puts("symtab");
    Symbol *sym = symtab->first;
    while (sym != NULL) {
        printf("label: %s\t%ld\n", sym->label, sym->value);
        sym = sym->next;
    }
    
    //printf("address of symtab is %p\n", symtab);
    sfree(symtab);
    
    //fclose(out);
    
	return 0;
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
    l->line_state = 0;
    l->arg_buf_size = 3;
    l->argv = salloc(sizeof(char *) * l->arg_buf_size);
    l->argc = 0;
    
    register char *c;
    for (c = buffer; *c != '\0'; c++) {
        switch (*c) {
        case '\n':
        case '\t':
        case ' ':
        case ',':
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
            break;
        case ':':
            if (l->line_state & MNEMONIC_STATE) {
                die("Error on line %ld. Label must occur at the beginning of a line.\n", line_num);
            }
            l->label = buffer;
            *c = '\0';
            buffer = c;
            buffer++;
            
            //printf("parsed label = %s\n", l->label);
            //label_set = 1;
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
        configuration.arch->parse_instruction(line);
        break;
    }
}
