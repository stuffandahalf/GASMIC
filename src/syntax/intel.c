#include "../as.h"

static int
evaluate_intel_args(struct line *l)
{
	die("INTEL SYNTAX NOT IMPLEMENTED YET\n");

	return 0;
}

static int
process_intel_syntax(struct line *l)
{
	die("INTEL SYNTAX NOT IMPLEMENTED YET\n");

	return 0;
}

struct syntax_handler intel_syntax = {
	.evaluate_args = evaluate_intel_args,
	.handler = process_intel_syntax
};

