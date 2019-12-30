#ifndef GASMIC_UTIL_H
#define GASMIC_UTIL_H

#include <as.h>

extern SymTab *symtab;
extern DataTab *datatab;

int init_data_table();
int init_symbol_table();
Data *init_data(Data *data);
void add_label(Line *line);
void add_data(Data *data);

void prepare_line(Line *line);

char *strdup(const char *src);
char *str_to_upper(char *str);

const Architecture *find_arch(const char *arch_name);
const Register *find_reg(const char *name);

#endif /* GASMIC_UTIL_H */
