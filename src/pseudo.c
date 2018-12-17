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

struct pseudo_instruction {
    char instruction[10];
    void (*process)(Line *line);
    size_t args;
};

static struct pseudo_instruction pseudo_ops[] = {
    { ".EQU", &pseudo_equ, 1},
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
        die("Error on line %ld: pseudo instruction .EQU requires a label on the same line\n", line_num);
    }
    if (line->argc != 1) {
        die("Error on line %ld: invalid number of arguments for pseudo instruction .EQU\n", line_num);
    }
    
    //symtab->last->value = 
}
