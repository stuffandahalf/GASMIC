#include <arithmetic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "arithmetic.h"

static struct operator operators[] = {
    { '(', 0 },
    { ')', 0 },
    { '/', 1 },
    { '*', 1 },
    { '+', 2 },
    { '-', 2 },
    { 0, 0 }
};

static struct operator *find_operator(char c)
{
    for (struct operator *op = operators; op->operator != 0; op++) {
        if (c == op->operator) {
            return op;
        }
    }
    return NULL;
}

// Implements the shunting yard algorithm
struct token *parse_expression(char *expr)
{
    struct token *out_stack_top = NULL;
    struct token *op_stack_top = NULL;
    const char *buffer = expr;
    size_t buffer_size = 0;
    bool stop = false;

    for (char *c = expr; !stop/* *c != '\0'*/; c++) {
        bool changed = false;

        double val = 0;
        char *endptr;

        struct operator *op = find_operator(*c);
        if (buffer_size > 0 && (op != NULL || *c == ' ' || *c == '\t' || *c == '\n' || *c == '\0')) {
            changed = true;
            struct token *tok = malloc(sizeof(struct token));
            val = strtod(buffer, &endptr);
            if (endptr == buffer) {
                tok->type = TOKEN_TYPE_SYMBOL;
                tok->value.symbol = malloc(sizeof(char) * (buffer_size + 1));
                strncpy(tok->value.symbol, buffer, buffer_size);
                tok->value.symbol[buffer_size] = '\0';
                /*buffer_size = 0;
                buffer = c;
                buffer++;*/
            }
            else if (endptr == c) {
                tok->type = TOKEN_TYPE_LITERAL;
                tok->value.number = val;
            }
            else {
                fprintf(stderr, "wtf is this.\n");
                exit(1);
            }

            tok->next = out_stack_top;
            out_stack_top = tok;
        }

        if (op != NULL) {
            changed = true;
            struct token *tok = malloc(sizeof(struct token));
            tok->type = TOKEN_TYPE_OPERATOR;
            tok->value.operator = op;

            if (op->operator == ')') {
                while (op_stack_top != NULL && op_stack_top->value.operator->operator != '(') {
                    struct token *top = op_stack_top;
                    op_stack_top = op_stack_top->next;
                    top->next = out_stack_top;
                    out_stack_top = top;
                    // TODO: add error handling
                }
                if (op_stack_top == NULL) {
                    // TODO: Throw error
                }
                op_stack_top = op_stack_top->next;
            }
            else {
                if (op->operator != '(') {
                    while (op_stack_top != NULL && op_stack_top->value.operator->precedence >= op->precedence && op_stack_top->value.operator->operator != '(') {
                        struct token *tmp = op_stack_top;
                        op_stack_top = op_stack_top->next;
                        tmp->next = out_stack_top;
                        out_stack_top = tmp;
                    }
                }
                tok->next = op_stack_top;
                op_stack_top = tok;
            }
            /*buffer_size = 0;
            buffer = c;
            buffer++;*/
        }

        if (changed) {
            buffer_size = 0;
            buffer = c;
            buffer++;
        }
        else {
            buffer_size++;
        }

        if (*c == '\0') {
            stop = true;
        }
    }

    while (op_stack_top != NULL) {
        struct token *top = op_stack_top;
        op_stack_top = op_stack_top->next;
        top->next = out_stack_top;
        out_stack_top = top;
    }

    return out_stack_top;
}

void free_token_stack(struct token *stack_top)
{
    struct token *tok = stack_top;
    while (tok != NULL) {
        struct token *next = tok->next;
        if (tok->type == TOKEN_TYPE_SYMBOL) {
            free(tok->value.symbol);
        }
        free(tok);
        tok = next;
    }
}