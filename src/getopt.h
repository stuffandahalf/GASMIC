#ifndef GASMIC_GETOPT_H
#define GASMIC_GETOPT_H

struct option {
	const char	*name;
	int 		has_arg;
	int 		*flag;
	int 		val;
};

int getopt(int argc, char *const argv[], const char *arglist);
int getopt_long(int argc, char *const argv[],
	const char *optstring,
	const struct option *longopts, int *longindex);

extern int optind;
extern int opterr;
extern int optopt;
extern char *optarg;

#endif /* GASMIC_GETOPT_H */
