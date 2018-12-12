#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <as.h>
#include <arch.h>

#define LINEBUFFERSIZE 256

char buffer[LINEBUFFERSIZE];

void configure(int argc, char *argv[]);
static Architecture *str_to_arch(const char arch_name[]);
Instruction *find_instruction(const char *mnem);

FILE *in;
FILE *out;
Architecture *arch;

int main(int argc, char **argv) {
    in = stdin;
    out = stdout;
    arch = architectures;
    
    configure(argc, argv);
    
    /*printf("%s\n", instructions[0].mne);
    
    while (fgets(buffer, LINEBUFFERSIZE, in) != NULL) {
        fprintf(out, "%s", buffer);
    }*/
    
    printf("%s\n", find_instruction("ADC")->mne);
    printf("%s\n", arch->name);
    
	return 0;
}

void configure(int argc, char *argv[]) {
    int c;
    while ((c = getopt(argc, argv, "m:o:f:")) != -1) {
        switch (c) {
            case 'm':
                arch = str_to_arch(optarg);
                if (arch == NULL) {
                    die("Unsupported architecture");
                }
                break;
            case 'o':
                out = fopen(optarg, "w+");
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

Instruction *find_instruction(const char *mnem) {
    for (Instruction *i = instructions; i->mne[0] != '\0'; i++) {
        if (streq(mnem, i->mne)) {
            return i;
        }
    }
    return NULL;
}
