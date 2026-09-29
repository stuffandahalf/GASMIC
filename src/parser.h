#ifndef GASMIC_PARSER_H
#define GASMIC_PARSER_H 1

#define RULE(name) int parse_ ## name (struct line *, const char *)

#define LCASE_RNG "abcdefghijklmnopqrstuvwxyz"
#define UCASE_RNG "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
#define NUM_RNG "0123456789"

#define ALPHA_RNG LCASE_RNG UCASE_RNG
#define ALPHANUM_RNG NUM_RNG ALPHA_RNG
#define IDENT_RNG ALPHANUM_RNG

struct line;

typedef int (*parse_token)(struct line *, const char *);

RULE(line);
RULE(label);
RULE(string);
RULE(expr);

/* parser utilities */
#define SEQ_CASEINSENSITIVE 1
int consume_seq(const char *, const char *, int);
int consume_range(const char *, const char *, int);
int consume_spaces(const char *);

#endif /* GASMIC_PARSER_H */
