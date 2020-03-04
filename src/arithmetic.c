#include <arithmetic.h>
#include <stdlib.h>
#include <string.h>
#include <lang.h>
/*#include <stdbool.h>*/

#ifdef __cplusplus
#define NEW(type) new type()
#define NEW_ARR(type, count) new type[(count)]
#define FREE(ptr) delete ptr
#define FREE_ARR(ptr) delete[] ptr
#else
#define NEW(type) malloc(sizeof(type));
#define NEW_ARR(type, count) calloc(sizeof(type), count)
#define FREE(ptr) free(ptr)
#define FREE_ARR(ptr) free(ptr)
#endif

enum arithmetic_status arithmetic_status_code;

static struct arithmetic_op operators[] = {
    { '(', 0 },
    { ')', 0 },
    { '/', 1 },
    { '*', 1 },
    { '%', 1 },
    { '+', 2 },
    { '-', 2 },
    { '^', 3 },
    { '&', 4 },
    { '|', 5 },
    { 0, 0 }
};

static struct arithmetic_op *find_operator(char c)
{
    struct arithmetic_op *op = NULL;
    for (op = operators; op->symbol != 0; op++) {
        if (c == op->symbol) {
            return op;
        }
    }
    return NULL;
}

/* Reverses a stack to become a linked list from bottom to top */
static struct token *stack_to_list(struct token *stack_top)
{
    struct token *prev = NULL;
    struct token *current = stack_top;
    struct token *next = NULL;

    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

#if __STDC_VERSION__ >= 199901L/* || defined(__cplusplus)*/
static INLINE bool is_whitespace(char c)
{
    return c == ' ' || c == '\t' || c == '\n';
}
#else
#define is_whitespace(c) (c == ' ' || c == '\t' || c == '\n')
#endif

/* Implements the shunting yard algorithm */
struct token *parse_expression(char *expr)
{
    struct token *out_stack_top = NULL;
    struct token *op_stack_top = NULL;
    struct token *tok;
    struct token *top;
    const char *buffer = expr;
    size_t buffer_size = 0;
    bool stop = false;
    char *c;

    arithmetic_status_code = ARITHMETIC_SUCCESS;

    for (c = expr; !stop/* *c != '\0'*/; c++) {
        bool changed = false;

        double val = 0;
        char *endptr;

        struct arithmetic_op *op = find_operator(*c);
        if (buffer_size > 0 && (op != NULL || is_whitespace(*c) || *c == '\0')) {
            changed = true;
			tok = NEW(struct token);
            val = strtod(buffer, &endptr);
            if (endptr == buffer) {
                tok->type = TOKEN_TYPE_SYMBOL;
				tok->value.symbol = NEW_ARR(char, buffer_size + 1);
                strncpy(tok->value.symbol, buffer, buffer_size);
                tok->value.symbol[buffer_size] = '\0';
            } else if (endptr == c) {
                tok->type = TOKEN_TYPE_LITERAL;
                tok->value.number = val;
            } else {
                fprintf(stderr, "wtf is this.\n");
                exit(1);
            }

            tok->next = out_stack_top;
            out_stack_top = tok;
        } else if (is_whitespace(*c)) {
            changed = true;
        }

        if (op != NULL) {
            changed = true;
			tok = NEW(struct token);
            tok->type = TOKEN_TYPE_OPERATOR;
            tok->value.op = op;

            if (op->symbol == ')') {
                while (op_stack_top->value.op->symbol != '(') {
                    struct token *top = op_stack_top;
                    op_stack_top = op_stack_top->next;
                    top->next = out_stack_top;
                    out_stack_top = top;
                    if (op_stack_top == NULL) {
                        free_token_chain(op_stack_top);
                        free_token_chain(out_stack_top);
                        arithmetic_status_code = ARITHMETIC_ERROR_UNMATCHED_PARENTHESIS;
                        return NULL;
                    }
                }
                op_stack_top = op_stack_top->next;
            } else {
                if (op->symbol != '(') {
                    while (op_stack_top != NULL &&
						op_stack_top->value.op->precedence >= op->precedence &&
						op_stack_top->value.op->symbol != '('
					) {
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
        } else {
            buffer_size++;
        }

        if (*c == '\0') {
            stop = true;
        }
    }

    while (op_stack_top != NULL) {
        if (op_stack_top->value.op->symbol == '(') {
            arithmetic_status_code = ARITHMETIC_ERROR_UNMATCHED_PARENTHESIS;
            free_token_chain(op_stack_top);
            free_token_chain(out_stack_top);
            return NULL;
        }

        top = op_stack_top;
        op_stack_top = op_stack_top->next;
        top->next = out_stack_top;
        out_stack_top = top;
    }

    return stack_to_list(out_stack_top);
}

void free_token_chain(struct token *stack_top)
{
    struct token *tok = stack_top;
    while (tok != NULL) {
        struct token *next = tok->next;
        if (tok->type == TOKEN_TYPE_SYMBOL) {
			FREE_ARR(tok->value.symbol);
        }
		FREE(tok);
        tok = next;
    }
}

void fprint_token_stack(FILE *fptr, struct token *stack_top)
{
    struct token *tok;
    for (tok = stack_top; tok != NULL; tok = tok->next) {
        switch (tok->type) {
        case TOKEN_TYPE_LITERAL:
            fprintf(fptr, "%f\n", tok->value.number);
            break;
        case TOKEN_TYPE_SYMBOL:
            fprintf(fptr, "%s\n", tok->value.symbol);
            break;
        case TOKEN_TYPE_OPERATOR:
            fprintf(fptr, "%c\n", tok->value.op->symbol);
            break;
        }
    }
}

void print_token_stack(struct token *stack_top)
{
    fprint_token_stack(stdout, stack_top);
}

void fprint_token_list(FILE *fptr, struct token *list)
{
    struct token *tok;
    for (tok = list; tok != NULL; tok = tok->next) {
        switch (tok->type) {
        case TOKEN_TYPE_LITERAL:
            fprintf(fptr, "%f ", tok->value.number);
            break;
        case TOKEN_TYPE_SYMBOL:
            fprintf(fptr, "%s ", tok->value.symbol);
            break;
        case TOKEN_TYPE_OPERATOR:
            fprintf(fptr, "%c ", tok->value.op->symbol);
            break;
        }
    }
    fprintf(fptr, "\n");
}

void print_token_list(struct token *list)
{
    fprint_token_list(stdout, list);
}
