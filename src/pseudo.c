#include <as.h>

/*
 * struc?
 * .equ
 * .include
 * .org
 * .resb
 * .fcb
 * .extern
 * .global
 * .ascii
 * .asciz
 */

static void pseudo_equ(Line *line);
static void pseudo_include(Line *line);

struct pseudo_instruction {
    char instruction[10];
    void (*process)(Line *line);
    size_t args;
};

static struct pseudo_instruction pseudo_ops[] = {
    { ".EQU", &pseudo_equ, 1},
    { ".INCLUDE", &pseudo_include, 1},
    { "", NULL, 0 }
};

void parse_pseudo_op(Line *line) {
    struct pseudo_instruction *pseudo_inst;
    for (pseudo_inst = pseudo_ops; pseudo_inst->instruction[0] != '\0'; pseudo_inst++) {
        if (streq(line->mnemonic, pseudo_inst->instruction)) {
            pseudo_inst->process(line);
            return;
        }
    }
    die("Error on line %ld: unable to find pseudo instruction %s\n", line_num, line->mnemonic);
}

static void pseudo_equ(Line *line) {
    if (!(line->line_state & LABEL_STATE)) {
        die("Error on line %ld. Pseudo instruction .EQU requires a label on the same line\n", line_num);
    }
    if (line->argc != 1) {
        die("Error on line %ld. Invalid number of arguments for pseudo instruction .EQU\n", line_num);
    }
    
    char *num_end;
    symtab->last->value = strtol(line->argv[0], &num_end, 0);
    if (line->argv[0] == num_end) {
        die("Error on line %ld. Failed to parse given value\n", line_num);
    }
}

static void pseudo_include(Line *line) {
    FILE *included_file;
    if ((included_file = fopen(line->argv[0], "r")) == NULL) {
        die("Failed to open included file \"%s\"\n", line->argv[0]);
    }
    
    //Line *new_line = salloc(sizeof(Line));
    assemble(included_file, line);
    fclose(included_file);
    //sfree(new_line);
    
    // need to reassign argv because assemble frees it but we return back to assemble.
    line->argc = 1;
    line->argv = salloc(line->argc * sizeof(char *));
}
