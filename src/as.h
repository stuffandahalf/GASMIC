#ifndef GASMIC_AS_H
#define GASMIC_AS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef __cplusplus
#define __STDC_FORMAT_MACROS
#endif
#include <inttypes.h>
#include <smem.h>

#ifdef _WIN32
#define fail(msg, ...) die("ERROR %ld: " msg, line_num, ##__VA_ARGS__)
#else
#include <ansistyle.h>
//#define fail(msg, ...) die("\033[0;31mERROR \033[0m%ld: " msg, line_num,  ##__VA_ARGS__)
#define fail(msg, ...) die(ANSI_COLOR_RED "ERROR" ANSI_COLOR_RESET " %ld: " msg, line_num,  ##__VA_ARGS__)
#endif

// test the equality of 2 strings
#define streq(__s1, __s2) !strcmp((const char *)(__s1), (const char *)(__s2))
// print to stderr
#define printef(fmt, ...) fprintf(stderr, fmt, ##__VA_ARGS__)

// print if debug build
#ifndef NDEBUG
#include <stdarg.h>
/*static inline int printdf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    int count = 0;

    count += printf("[%s:%d] >> ", __FILE__, __LINE__);
    count += vprintf(fmt, args);

    va_end(args);

    return count;
}*/
#define printdf(fmt, ...) printf("[%s:%d] >> " fmt, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define printdf(fmt, ...)
#endif

typedef enum {
    ARCH_ENDIAN_BIG = 1,
    ARCH_ENDIAN_LITTLE = 2,
    ARCH_ENDIAN_MIXED = 3
} endian_t;

/*#define ARCH_ENDIAN_BIG     (1)
#define ARCH_ENDIAN_LITTLE  (2)
#define ARCH_ENDIAN_MIXED   (3)*/

typedef enum {
    SYNTAX_UNKNOWN = 0,
    SYNTAX_MOTOROLA = 1,
    SYNTAX_INTEL = 2,
    SYNTAX_ATT = 3,
} syntax_t;

/*#define SYNTAX_UNKNOWN  (0)
#define SYNTAX_MOTOROLA (1)
#define SYNTAX_INTEL    (2)
#define SYNTAX_ATT      (3)*/

typedef enum {
    ARG_ORDER_NONE = 0,
    ARG_ORDER_FROM_REG = 1,
    ARG_ORDER_TO_REG = 2,
    ARG_ORDER_INTERREG = 3
} arg_order_t;

/*#define ARG_ORDER_NONE      (0)
#define ARG_ORDER_FROM_REG  (1)
#define ARG_ORDER_TO_REG    (2)
#define ARG_ORDER_INTERREG  (3)*/

typedef struct symtab_entry {
    char *label;
    size_t value;
    struct symtab_entry *next;
} Symbol;

typedef struct {
    Symbol *first;
    Symbol *last;
    Symbol *last_parent;
} SymTab;

/*#define DATA_TYPE_NONE  (0)
#define DATA_TYPE_LABEL (1)
#define DATA_TYPE_BYTES (2)*/

typedef enum {
    DATA_TYPE_NONE = 0,
    DATA_TYPE_LABEL = 1,
    DATA_TYPE_BYTES = 2
} data_type_t;
typedef struct data_entry {
    data_type_t type;
    size_t address;
    uint8_t bytec;  // This needs to become a union of uint8_t and char *label
    union {
        char *symbol;
        uint8_t *bytes;
    } contents;
    struct data_entry *next;
} Data;

typedef struct {
    Data *first;
    Data *last;
} DataTab;

typedef struct {
    char name[5];
    int width;
    int arcs;
} Register;

#define MAX_MNEMONIC_LEN (10)
//#define MAX_REGS (20)

/*#define ARCH_INSTRUCTION(arch, Topcode, reg_count, mode_count) \
    struct { \
        char mnemonic[MAX_MNEMONIC_LEN]; \
        uint8_t architectures; \
        uint8_t arg_order; \
        struct arch##_instruction_register { \
            const Register *reg; \
            struct { \
                uint8_t mode; \
                Topcode opcode; \
            } addressing_modes[mode_count]; \
        } registers[reg_count + 1]; \
    }*/


typedef enum {
	ADDR_MODE_INVALID,
	ADDR_MODE_INHERENT,
	ADDR_MODE_IMMEDIATE,
	ADDR_MODE_DIRECT,
	ADDR_MODE_INDIRECT,
	ADDR_MODE_EXTENDED,
	ADDR_MODE_INTERREGISTER
} addr_mode_t;

/*#define MC6809_ADDR_MODE_INVALID    (0)
#define MC6809_ADDR_MODE_INH        (1)
#define MC6809_ADDR_MODE_IMM        (2)
#define MC6809_ADDR_MODE_DIR        (3)
#define MC6809_ADDR_MODE_IND        (4)
#define MC6809_ADDR_MODE_EXT        (5)
#define MC6809_ADDR_MODE_INTER      (6)*/

typedef struct {
    char mnemonic[MAX_MNEMONIC_LEN];
    uint8_t architectures;
    uint8_t arg_order;
    //Register *registers[MAX_REGS];
    struct instruction_register {
        const Register *reg;
        struct {
            addr_mode_t mode;
			uint8_t opcode_size;	// in bytes
            uint64_t opcode;
        } addressing_modes[10];
    } opcodes[];
} Instruction;

typedef enum {
    ARG_TYPE_NONE,
    ARG_TYPE_REGULAR,
    ARG_TYPE_SYMBOL,
    ARG_TYPE_STRING,
    ARG_TYPE_UNPROCESSED
} arg_type_t;

/*#define ARG_TYPE_NONE 0
#define ARG_TYPE_REG  1
#define ARG_TYPE_SYM  2
#define ARG_TYPE_STR  3*/
#define ARG_STATE_BRACKET 1

typedef struct {
    arg_type_t type;
    //uint8_t addr_mode;
    union {
        char *str;
        Register *reg;
        Symbol *sym;
    } val;
} LineArg;

typedef enum {
    LINE_STATE_CLEAR = 0u,
    LINE_STATE_LABEL = 1u,
    LINE_STATE_MNEMONIC = 2u,
    LINE_STATE_SINGLE_QUOTE = 4u,
    LINE_STATE_DOUBLE_QUOTE = 8u,
    LINE_STATE_BRACKET = 16u
} line_state_t;
/*#define LINE_STATE_CLEAR        0u
#define LINE_STATE_LABEL        1u
#define LINE_STATE_MNEMONIC     2u
#define LINE_STATE_SINGLE_QUOTE 4u
#define LINE_STATE_DOUBLE_QUOTE 8u
#define LINE_STATE_BRACKET      16u*/

/*#define LINE_STATE_LABEL (1)
#define LINE_STATE_MNEMONIC (2)
#define LINE_STATE_QUOTE (4)
#define LINE_STATE_BRACKET (8)*/
typedef struct {
    char *label;
    char *mnemonic;
    LineArg *argv;
    //char **argv;
    size_t argc;
    uint8_t arg_buf_size;
    line_state_t line_state;
} Line;

typedef struct {
    char name[10];
    //void (*parse_instruction)(Line *l);
    int value;
    uint8_t byte_size;  // bits per byte
    uint8_t bytes_per_address;
    endian_t endianness;
    syntax_t default_syntax;
	const Register *registers;
	const Instruction **instructions;
} Architecture;

struct pseudo_instruction {
    char instruction[10];
    void (*process)(Line *line);
    size_t args;
};

typedef struct {
    char *out_fname;
    char **in_fnames;
    size_t in_fnamec;
    uint8_t syntax;
    Architecture *arch;
} Config;

extern size_t line_num;
extern size_t address;
extern size_t address_mask;
extern Config configuration;

void init_address_mask();
void assemble(FILE *in, Line *l);
Architecture *str_to_arch(const char arch_name[]);
struct pseudo_instruction *get_pseudo_op(Line *line);
void parse_pseudo_op(Line *line);

static inline char *str_to_upper(char str[]) {
    for (char *c = str; *c != '\0'; c++) {
        *c = (char)toupper(*c);
    }
    return str;
}

#endif
