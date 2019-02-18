#include <as.h>
#include <arch.h>

/*
 * struc?
 * .org
 * .equ
 * .include
 * .org
 * .resb
 * .fcb
 * .extern
 * .global
 * .ascii
 * .asciz
 * .extern
 * .global
 */

static void pseudo_arch(Line *line);
static void pseudo_set_byte(Line *line);
static void pseudo_set_word(Line *line);
static void pseudo_set_double(Line *line);
static void pseudo_set_quad(Line *line);
static void pseudo_equ(Line *line);
static void pseudo_include(Line *line);
static void pseudo_org(Line *line);

static struct pseudo_instruction pseudo_ops[] = {
    { ".ARCH", &pseudo_arch, 1 },
    { ".DB", &pseudo_set_byte, -1 },
    /*{ ".DW", &pseudo_set_word, -1 },
    { ".DD", &pseudo_set_double, -1 },
    { ".DQ", &pseudo_set_quad, -1 },*/
    { ".EQU", &pseudo_equ, 1 },
    { ".INCLUDE", &pseudo_include, 1 },
    { ".ORG", &pseudo_org, 1 },
    //{ ".SYNTAX", &pseudo_syntax, 1 },
    { "", NULL, 0 }
};

struct pseudo_instruction *get_pseudo_op(Line *line) {
    struct pseudo_instruction *pseudo_op;
    for (pseudo_op = pseudo_ops; *pseudo_op->instruction != '\0'; pseudo_op++) {
        if ((pseudo_op->args == -1 || pseudo_ops->args == line->argc)
                && streq(line->mnemonic, (*line->mnemonic == '.' ? pseudo_op->instruction : &pseudo_op->instruction[1]))) {
            return pseudo_op;
        }
    }
    return NULL;
}

void parse_pseudo_op(Line *line) {
    /*struct pseudo_instruction *pseudo_inst;
    for (pseudo_inst = pseudo_ops; pseudo_inst->instruction[0] != '\0'; pseudo_inst++) {
        if (streq(line->mnemonic, pseudo_inst->instruction)) {
            pseudo_inst->process(line);
            return;
        }
    }*/
    struct pseudo_instruction *pseudo_inst = get_pseudo_op(line);
    if (pseudo_inst == NULL) {
        //die("Error on line %ld: unable to find pseudo instruction %s\n", line_num, line->mnemonic);
        fail("Unable to find pseudo instruction %s.\n", line->mnemonic);
    }
    pseudo_inst->process(line);
}

static void pseudo_arch(Line *line) {    
    if (datatab->first == NULL) {
        Architecture *arch = str_to_arch(line->argv[0].val.str);
        if (arch == NULL) {
            //die("Failed to locate architecture %s\n", line->argv[0].val.str);
            fail("Failed to locate architecture %s.\n", line->argv[0].val.str);
        }
        configuration.arch = arch;
        printf("%s\n", configuration.arch->name);
    }
    else {
        //die("Cannot switch architecture after code\n");
        fail("Cannot switch architecture after code.\n");
    }
}

static void pseudo_set_byte(Line *line) {
    char *send;
    uint8_t byte;
    unsigned int i;
    for (i = 0; i < line->argc; i++) {
        byte = strtol(line->argv[i].val.str, &send, 0) & 0xFF;
        if (send == line->argv[i].val.str) {
            //fail("
            char *j;
            for (j = line->argv[i].val.str; *j != '\0'; j++) {
                byte = (*j) & 0xFF;
                #ifdef DEBUG
                printf("%u\t", byte);
                #endif
                // add byte to datatab
            }
        }
        else if (*send != '\0') {
            fail("Cannot allocate bytes for %s\n", line->argv[i].val.str);
        }
        else {
            //byte &= 0xFF;
            #ifdef DEBUG
            printf("%u\t", byte);
            #endif
            // add byte to datatab
        }
    }
    #ifdef DEBUG
    printf("\n");
    #endif
}

static void pseudo_set_double(Line *line) {
    
}

static void pseudo_equ(Line *line) {
    if (!(line->line_state & LABEL_STATE)) {
        //die("Error on line %ld. Pseudo instruction .EQU requires a label on the same line\n", line_num);
        fail("Pseudo instruction .EQU requires a label on the same line.\n");
    }
    if (line->argc != 1) {
        //die("Error on line %ld. Invalid number of arguments for pseudo instruction .EQU\n", line_num);
        fail("Invalid number of arguments for pseudo instruction .EQU.\n");
    }
    
    char *num_end;
    symtab->last->value = strtol(line->argv[0].val.str, &num_end, 0);
    //if (line->argv[0] == num_end) {
    if (*num_end != '\0') {
        //die("Error on line %ld. Failed to parse given value\n", line_num);
        fail("Failed to parse given value.\n");
    }
}

static void pseudo_include(Line *line) {
    FILE *included_file;
    if ((included_file = fopen(line->argv[0].val.str, "r")) == NULL) {
        //die("Failed to open included file \"%s\" on line %ld\n", line->argv[0].val.str, line_num);
        fail("Failed to open included file \"%s\".\n", line->argv[0].val.str);
    }
    
    Line *inc_line = salloc(sizeof(Line));
    
    //assemble(included_file, line);
    assemble(included_file, inc_line);
    fclose(included_file);
    //sfree(new_line);
    
    // need to reassign argv because assemble frees it but we return back to assemble.
    //line->argc = 1;
    //line->argv = salloc(line->argc * sizeof(char *));
}

static void pseudo_org(Line *line) {
    char *lend;
    address = strtol(line->argv[0].val.str, &lend, 0);
    if (lend == line->argv[0].val.str || *lend != '\0') {
        //die("value is not a number on line %ld\n", line_num);
        fail("Value is not a number.\n");
    }
}
