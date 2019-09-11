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
/*#include <stdarg.h>
static inline int printdf(const char *fmt, ...) {
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

// This is to stop clang-tidy from complaining abouot signed enum types
#define FLAG(f) ((unsigned int)(f))

enum endian {
    ARCH_ENDIAN_BIG = 1,
    ARCH_ENDIAN_LITTLE = 2,
    ARCH_ENDIAN_MIXED = 3
};

enum syntax {
    SYNTAX_UNKNOWN,
    SYNTAX_MOTOROLA,
    SYNTAX_INTEL,
    SYNTAX_ATT,
};

enum arg_order {
    ARG_ORDER_NONE,
    ARG_ORDER_FROM_REG,
    ARG_ORDER_TO_REG,
    ARG_ORDER_INTERREG
};

#if 0
enum address_type {
    ADDRESS_TYPE_ABSOLUTE;
    ADDRESS_TYPE_RELATIVE;
};
struct address {
    enum address_type type;
    union {
        size_t absolute;
        struct {
            struct token *root_expr;
            size_t offset;
        };
    } value;
};
#endif

typedef struct symtab_entry {
    char *label;
    int64_t value;
    struct symtab_entry *next;
} Symbol;

typedef struct {
    Symbol *first;
    Symbol *last;
    Symbol *last_parent;
} SymTab;

enum data_type {
    DATA_TYPE_NONE,
    DATA_TYPE_EXPRESSION,
    DATA_TYPE_BYTES
};
typedef struct data_entry {
    enum data_type type;
    size_t address;
    uint8_t bytec;
    union {
        uint8_t *bytes;
        struct token *rpn_expr;
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
    unsigned int arcs;
} Register;

#define MAX_MNEMONIC_LEN (10)

enum address_mode {
	ADDR_MODE_INVALID,
	ADDR_MODE_INHERENT,
	ADDR_MODE_IMMEDIATE,
	ADDR_MODE_DIRECT,
	ADDR_MODE_INDIRECT,
	ADDR_MODE_EXTENDED,
	ADDR_MODE_INDEXED,
	ADDR_MODE_INDEXED_INDIRECT,
	ADDR_MODE_INTERREGISTER
};

typedef struct {
    char mnemonic[MAX_MNEMONIC_LEN];
    uint8_t architectures;
    uint8_t arg_order;
    struct instruction_register {
        const Register *reg;
        struct {
            enum address_mode mode;
			uint8_t opcode_size;	// in bytes
            uint64_t opcode;
        } addressing_modes[10];
    } opcodes[];
} Instruction;

enum arg_type {
    ARG_TYPE_UNPROCESSED,
    ARG_TYPE_STRING,
    ARG_TYPE_EXPRESSION,
    ARG_TYPE_REGISTER,
    ARG_TYPE_INDEX
};

enum arg_state {
    ARG_STATE_CLEAR = 0,
    ARG_STATE_BRACKET = (1u << 0u),
    ARG_STATE_COMMA = (1u << 1u)
};

typedef struct {
    enum arg_type type;
    enum arg_state state;
    union {
        char *str;
        struct token *rpn_expr;
        struct {
            const Register *reg;
            int16_t offset;
        } address;
        const Register *reg;
    } val;
} LineArg;

enum line_state {
    LINE_STATE_CLEAR =          0,
    LINE_STATE_LABEL =          (1u << 0u),
    LINE_STATE_MNEMONIC =       (1u << 1u),
    LINE_STATE_SINGLE_QUOTE =   (1u << 2u),
    LINE_STATE_DOUBLE_QUOTE =   (1u << 3u),
    LINE_STATE_BRACKET =        (1u << 4u)
};

enum address_post_op {
    POST_OP_NONE,
    POST_OP_INC_SINGLE,
    POST_OP_INC_DOUBLE,
    POST_OP_DEC_SINGLE,
    POST_OP_DEC_DOUBLE
};

typedef struct {
    char *label;
    char *mnemonic;
    LineArg *argv;
    size_t argc;
    uint8_t arg_buf_size;
    enum line_state line_state;
    enum address_mode address_mode;
    enum address_post_op addr_mode_post_op;
} Line;

typedef struct {
    char *name;
    int value;
    uint8_t byte_size;  // bits per byte
    uint8_t bytes_per_address;
    enum endian endianness;
    enum syntax default_syntax;
	const Register *registers;
	const Instruction **instructions;
	void (*process_line)(Line *line, const struct instruction_register *instr_reg, Data *data);
} Architecture;

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

static inline char *str_to_upper(char str[]) {
    for (char *c = str; *c != '\0'; c++) {
        *c = (char)toupper(*c);
    }
    return str;
}

#endif
