//
// Created by ubuntu on 2019-09-03.
//

#ifndef GASMIC_ARITHMETIC_H
#define GASMIC_ARITHMETIC_H

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
void free_token_stack(struct token *stack_top);

#endif //GASMIC_ARITHMETIC_H
