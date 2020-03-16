#ifndef GASMIC_GETOPT_H
#define GASMIC_GETOPT_H

/*#include <stdbool.h>*/
#include "lang.h"
/*#include <string.h>*/

typedef struct {
	char *longopt;
	char shortopt;
	bool hasarg;
} opt_t;

typedef struct {
	opt_t *opts;
	char *optarg;
	int optind;
} getopt_t;

int getopt(int argc, char *const *argv, const char *arglist);
int getopt_stateless(int argc, char **argv, getopt_t *optstruct);

extern int optind;
extern char *optarg;

#endif /* GASMIC_GETOPT_H */
