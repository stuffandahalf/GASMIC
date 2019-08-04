#ifndef GASMIC_UTIL_H
#define GASMIC_UTIL_H

#include <as.h>

extern SymTab *symtab;
extern DataTab *datatab;

int init_data_table();
int init_symbol_table();
Data *init_data(Data *data);
void add_label(Line *l);
void add_data(Data *d);

#endif //GASMIC_UTIL_H
