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
Instruction *find_instruction(const char *mnem);

char *out_fname;
FILE *in;
FILE *out;
Architecture *arch;
size_t address;
size_t line_num;

int main(int argc, char **argv) {
    out_fname = strdup("a.out");
    in = stdin;
    //out = stdout;
    arch = architectures;
    address = 0;
    line_num = 0;

    configure(argc, argv);

    //out = fopen(out_fname, "w+");
    free(out_fname);
    
    while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {
        //str_to_upper(buffer);
        //Instruction *i = find_instruction(str_to_upper(buffer));
        //printf("buffer after conversion contains %s\n", buffer);
        
        if (buffer[0] != '\0' || buffer[0] != '\n') {
            Line *l;
            salloc(l, sizeof(Line));
            parse_line(l, buffer);
            
            printf("parsed line: %s\t%s\n", l->label, l->mnemonic);
            
            /*Instruction *i = find_instruction(buffer);
            if (i != NULL) {
                printf("%s, %X, %X\n", i->mne, i->base_opcode, i->regs);
            }*/
            
            free(l->argv);
            free(l);
        }
        line_num++;
    }
    
    //fclose(out);

    /*printf("%s\n", find_instruction("ADC")->mne);
    printf("%s\n", arch->name);*/
    
	return 0;
}

static void configure(int argc, char *argv[]) {
    int c;
    while ((c = getopt(argc, argv, "m:o:f:")) != -1) {
        switch (c) {
        case 'm':
            arch = str_to_arch(optarg);
            if (arch == NULL) {
                die("Unsupported architecture: %s\n", optarg);
            }
            break;
        case 'o':
            free(out_fname);
            out_fname = strdup(optarg);
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

static char *str_to_upper(char str[]) {
    char *c;
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
    #define LABEL_STATE (1)
    #define MNEMONIC_STATE (2)
    
    l->line_state = 0;
    l->arg_buf_size = 3;
    //l->argv = malloc(sizeof(char *) * l->arg_buf_size);
    salloc(l->argv, sizeof(char *) * l->arg_buf_size);
    l->argc = 0;
    
    char *c;
    for (c = buffer; *c != '\0'; c++) {
        switch (*c) {
        case ',':
            
            break;
        case '\n':
        case '\t':
        case ' ':
        //case ',':
            if (c != buffer) {              // Otherwise save the current token
                if (!(l->line_state & MNEMONIC_STATE)) {    // if mnemonic is not set
                    l->mnemonic = buffer;
                    l->line_state |= MNEMONIC_STATE;
                }
                else {                                      // Argument
                    //printf("wtf is this %s\n", buffer);
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
                die("Label must occur at the beginning of a line");
            }
            l->label = buffer;
            *c = '\0';
            buffer = c;
            buffer++;
            
            printf("parsed label = %s\n", l->label);
            //label_set = 1;
            l->line_state |= LABEL_STATE;
            break;
        case ';':
            return;
        }
    }
}

Instruction *find_instruction(const char *mnem) {
    for (Instruction *i = instructions; i->mne[0] != '\0'; i++) {
        printf("%s\t%s\n", mnem, i->mne);
        if (streq(mnem, i->mne)) {
            return i;
        }
    }
    return NULL;
}
