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

static struct {
    char instruction[10];
    void (*process)(Line *line);
    size_t args;
} pseudo_ops[] = {
    { ".EQU", &pseudo_equ, 1},
    { "", NULL, 0 }
};

void parse_pseudo_op(Line *line) {
    
}

static void pseudo_equ(Line *line) {
    
}
