#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <as.h>
#include <arch.h>

typedef struct {
    char *label;
    char *mnemonic;
    char **args;
} Line;

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
            Line *l = malloc(sizeof(Line));
            parse_line(l, buffer);
            
            /*Instruction *i = find_instruction(buffer);
            if (i != NULL) {
                printf("%s, %X, %X\n", i->mne, i->base_opcode, i->regs);
            }*/
            
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
    unsigned char line_state = 0;
    #define LABEL_STATE (1)
    #define MNEMONIC_STATE (2)
    
    unsigned char label_set = 0;
    
    char *c;
    for (c = buffer; *c != '\0'; c++) {
        switch (*c) {
        case '\n':
        case '\t':
        case ' ':
            if (c == buffer) {
                buffer++;
            }
            else {
                *c = '\0';
                buffer = c;
                buffer++;
                if ((line_state & LABEL_STATE) && !(line_state & MNEMONIC_STATE)) {
                    l->mnemonic = buffer;
                    printf("parsed mnemonic = %s\n", l->mnemonic);
                    line_state |= MNEMONIC_STATE;
                }
                else {
                    printf("%s\n", buffer);
                }
            }
            break;
        case ':':
            l->label = buffer;
            *c = '\0';
            buffer = c;
            buffer++;
            
            line_state |= LABEL_STATE;
            
            printf("parsed label = %s\n", l->label);
            label_set = 1;
            break;
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
