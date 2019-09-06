#include <util.h>
#include <arithmetic.h>
#include "arithmetic.h"

SymTab *symtab;
DataTab *datatab;

int init_symbol_table()
{
    if (symtab != NULL) {
        return 0;
    }
    symtab = salloc(sizeof(SymTab));
    symtab->first = NULL;
    symtab->last = NULL;

    return 1;
}

int init_data_table()
{
    if (datatab != NULL) {
        return 0;
    }

    datatab = salloc(sizeof(DataTab));
    datatab->first = NULL;
    datatab->last = NULL;

    return 1;
}

void add_label(Line *line)
{
    Symbol *sym = salloc(sizeof(Symbol));

    sym->next = NULL;
    if (line->label[0] == '.') {
        if (symtab->last == NULL) {
            fail("Local label cannot be defined before any non-local labels.\n");
        }

        sym->label = salloc(sizeof(char) * (strlen(symtab->last_parent->label) + strlen(line->label) + 1));

        register char *label = sym->label;
        register char *c;
        for (c = symtab->last_parent->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        for (c = line->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        *label = '\0';

        line->label = sym->label;
    }
    else {
        sym->label = salloc(strlen(line->label) + sizeof(char));
        strcpy(sym->label, line->label);
        symtab->last_parent = sym;
    }

    printdf("Parsed label = %s\n", sym->label);

    sym->value = address;

    Symbol *test_sym;
    for (test_sym = symtab->first; test_sym != NULL; test_sym = test_sym->next) {
        if (streq(test_sym->label, sym->label)) {
            fail("Symbol \"%s\" is already defined.\n", test_sym->label);
        }
    }

    if (symtab->first == NULL) {
        symtab->first = sym;
    }
    else {
        symtab->last->next = sym;
    }
    symtab->last = sym;
}

Data *init_data(Data *data)
{
    data->next = NULL;
    data->type = DATA_TYPE_NONE;
    data->bytec = 0;
    data->contents.bytes = NULL;
    return data;
}

void add_data(Data *data)
{
    if (datatab->first == NULL) {
        datatab->first = data;
    }
    else {
        datatab->last->next = data;
    }
    datatab->last = data;
    Data *next = datatab->last->next;
    while (next != NULL) {
        datatab->last = next;
        next = next->next;
    }
}

void prepare_line(Line *line)
{
    size_t i;
    struct token *tok = NULL;
    for (i = 0; i < line->argc; i++) {
        if (line->argv[i].type == ARG_TYPE_UNPROCESSED) {
            line->argv[i].type = ARG_TYPE_EXPRESSION;
            line->argv[i].val.rpn_expr = parse_expression(line->argv[i].val.str);
            if (arithmetic_status_code != ARITHMETIC_SUCCESS) {
                fail("Failed to parse expression.");
            }
            for (tok = line->argv[i].val.rpn_expr; tok != NULL; tok = tok->next) {
                if (tok->type == TOKEN_TYPE_SYMBOL && tok->value.symbol[0] == '.') {
                    size_t parent_size = strlen(symtab->last_parent->label);
                    size_t child_size = strlen(tok->value.symbol);
                    char *child = tok->value.symbol;

                    tok->value.symbol = malloc(sizeof(char) * (parent_size + child_size + 1));
                    if (tok->value.symbol == NULL) {
                        fail("Failed to reallocate buffer for local symbol in expression.\n");
                    }
                    strncpy(tok->value.symbol, symtab->last_parent->label, parent_size);
                    strncpy(&(tok->value.symbol[parent_size]), child, child_size + 1);
                    free(child);
                }
            }
        }
    }
}