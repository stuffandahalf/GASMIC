#include <util.h>
#include <arithmetic.h>
#include "arithmetic.h"

extern Architecture **architectures[];

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

/*
 * Resolve a label into it's complete representation
 * returns a salloced char *.
 */
char *resolve_symbol(char *symbol)
{
    size_t parent_len, local_len;
    char *symbol_ptr;
    char *complete_symbol = NULL;
    char *c;

    if (*symbol != '.') {
        complete_symbol = salloc(sizeof(char) * (strlen(symbol) + 1));
        strcpy(complete_symbol, symbol);

        return complete_symbol;
    }

    if (symtab->last_parent == NULL) {
        fail("Local label cannot be defined before any non-local labels.\n");
    }

    parent_len = strlen(symtab->last_parent->label);
    local_len = strlen(symbol);
    complete_symbol = salloc(sizeof(char) * (parent_len + local_len + 1));

    symbol_ptr = complete_symbol;
    for (c = symtab->last_parent->label; *c != '\0'; c++) {
        *symbol_ptr++ = *c;
    }
    for (c = symbol; *c != '\0'; c++) {
        *symbol_ptr++ = *c;
    }
    *symbol_ptr = '\0';
    printdf(("Symbol \"%s\" resolved to \"%s\"\n", symbol, complete_symbol));

    return complete_symbol;
}


void add_label(Line *line)
{
    Symbol *test_sym;

    Symbol *sym = salloc(sizeof(Symbol));

    sym->next = NULL;
    sym->label = resolve_symbol(line->label);

    if (line->label[0] == '.') {
        line->label = sym->label;
    } else {
        symtab->last_parent = sym;
    }

    /*if (line->label[0] == '.') {
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
    }*/

    /*printdf("Parsed label = %s\n", sym->label);*/

    sym->value = address;

    for (test_sym = symtab->first; test_sym != NULL; test_sym = test_sym->next) {
        if (streq(test_sym->label, sym->label)) {
            fail("Symbol \"%s\" is already defined.\n", test_sym->label);
        }
    }

    if (symtab->first == NULL) {
        symtab->first = sym;
    } else {
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
    Data *next;

    if (datatab->first == NULL) {
        datatab->first = data;
    } else {
        datatab->last->next = data;
    }
    datatab->last = data;
    next = datatab->last->next;
    while (next != NULL) {
        datatab->last = next;
        next = next->next;
    }
}

void prepare_line(Line *line)
{
#if 1
    size_t i;
    struct token *tok;

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
                    strncpy(&tok->value.symbol[parent_size], child, child_size + 1);
                    free(child);
                }
            }
        }
    }
#endif
}

char *strdup(const char *src)
{
    char *new = malloc(sizeof(char) * (strlen(src) + 1));
    if (new == NULL) {
        return NULL;
    }
    strcpy(new, src);
    return new;
}

char *str_to_upper(char *str)
{
    for (char *c = str; *c != '\0'; c++) {
        *c = (char)toupper(*c);
    }
    return str;
}

char *str_trim(char *str)
{
    int done = 0;
    char *c;
    char *end;

    while (*str == ' ' || *str == '\t') str++;  // find first non whitespace character

    end = str;  // starting from the first whitespace character:
    while (!done) {
        while (*end != ' ' && *end != '\t' && *end != '\0') {   // check if the character is not a whitespace character
            end++;
        }

        if (*end == '\0') { // if the current character is the end of the string
            return str;
        }

        for (c = end; *c == '\t' || *c == ' '; c++);    // look for the next non-whitespace character;

        if (*c == '\0') {
            *end = '\0';
            done = 1;
        } else {
            end = c;
        }
    }

    return str;
}

const Architecture *find_arch(const char *arch_name)
{
    for (Architecture ***a = architectures; *a != NULL; a++) {
        if (streq(arch_name, (**a)->name)) {
            return **a;
        }
    }
    return NULL;
}

const Register *find_reg(const char *name)
{
    for (const Register *r = configuration.arch->registers; *r->name != '\0'; r++) {
        if (streq(name, r->name)) {
            return r;
        }
    }
    return NULL;
}
