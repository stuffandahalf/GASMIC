#include "util.h"

SymTab *symtab;
DataTab *datatab;

int init_symbol_table() {
    if (symtab != NULL) {
        return 0;
    }
    symtab = salloc(sizeof(SymTab));
    symtab->first = NULL;
    symtab->last = NULL;

    return 1;
}

int init_data_table() {
    if (datatab != NULL) {
        return 0;
    }

    datatab = salloc(sizeof(DataTab));
    datatab->first = NULL;
    datatab->last = NULL;

    return 1;
}

void add_label(Line *l) {
    Symbol *sym = salloc(sizeof(Symbol));

    sym->next = NULL;
    if (l->label[0] == '.') {
        if (symtab->last == NULL) {
            fail("Local label cannot be defined before any non-local labels.\n");
        }

        sym->label = salloc(sizeof(char) * (strlen(symtab->last_parent->label) + strlen(l->label) + 1));

        register char *label = sym->label;
        register char *c;
        for (c = symtab->last_parent->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        for (c = l->label; *c != '\0'; c++) {
            *(label++) = *c;
        }
        *label = '\0';

        l->label = sym->label;
    }
    else {
        sym->label = salloc(strlen(l->label) + sizeof(char));
        strcpy(sym->label, l->label);
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

Data *init_data(Data *data) {
    data->next = NULL;
    data->type = DATA_TYPE_NONE;
    data->bytec = 0;
    data->contents.bytes = NULL;
    return data;
}

void add_data(Data *data) {
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