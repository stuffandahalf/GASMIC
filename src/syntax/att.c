#include "../as.h"
#include "../smem.h"

static int
evaluate_att_syntax(struct line *l)
{
	die("AT&T SYNTAX NOT YET IMPLEMENTED\n");

	return 0;
}

static int
process_att_syntax(struct line *l)
{
	die("AT&T SYNTAX NOT YET IMPLEMENTED\n");

	return 0;
}

struct syntax_handler att_syntax = {
	.evaluate_args = evaluate_att_syntax,
	.handler = process_att_syntax
};

