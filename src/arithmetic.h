#ifndef GASMIC_ARITHMETIC_H
#define GASMIC_ARITHMETIC_H

#include <stdio.h>

enum arithmetic_status {
    ARITHMETIC_SUCCESS,
    ARITHMETIC_ERROR_INSUFFICIENT_OPERANDS,
    ARITHMETIC_ERROR_UNMATCHED_PARENTHESIS
};
extern enum arithmetic_status arithmetic_status_code;

enum token_type {
    TOKEN_TYPE_END,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_LITERAL,
    TOKEN_TYPE_SYMBOL
};

struct token {
    enum token_type type;
    struct token *next;
    union {
        struct operator *operator;
        double number;
        char *symbol;
    } value;
};

struct operator {
    char operator;
    unsigned char precedence;
};

struct token *parse_expression(char *expr);
void free_token_chain(struct token *stack_top);
void fprint_token_stack(FILE *fptr, struct token *stack_top);
void fprint_token_list(FILE *fptr, struct token *list);
void print_token_stack(struct token *stack_top);
void print_token_list(struct token *list);

#endif /* GASMIC_ARITHMETIC_H */
