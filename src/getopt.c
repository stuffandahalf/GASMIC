#include <getopt.h>

int optind = 1;
static int optcharind = 1;
char *optarg = NULL;
int optopt;

int getopt(int argc, char *const *argv, const char *arglist)
{
    const char *arg;
    char *c;
    size_t arglen;
    int retval;

    if (optind == argc) {
		return -1;
	}
	
	arg = NULL;
	c = argv[optind];
	arglen = strlen(c);
	if (arglen == 2) {
		optind++;
	} else {
		optcharind++;
	}

	retval = 0;

	if (*c != '-') {
		return '?';
	}

	for (arg = arglist; *arg != '\0'; arg++) {
		if (c[optcharind] == *arg) {
			retval = c[optcharind++];
			arg++;
			break;
		}
	}

	switch (*arg) {
	case ':':
		if (optcharind < arglen) {
			return '?';
		} else if (optind == argc) {
			return ':';
		} else {
			optarg = argv[optind++];
			return retval;
		}
		break;
	case '\0':
		return '?';
	default:
		return retval;
	}
	
}

int getopt_stateless(int argc, char **argv, getopt_t *opts)
{
	return -1;
}
