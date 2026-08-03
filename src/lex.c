#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "as.h"
#include "parser.h"

/* single character tokens */
static const char *scchars = "';:,[]()*/|^&\n";
static const int sctoks[] = {
	TOKEN_APOSTROPHE,
	TOKEN_SEMICOLON,
	TOKEN_COLON,
	TOKEN_COMMA,
	TOKEN_LSQBR,
	TOKEN_RSQBR,
	TOKEN_LPAREN,
	TOKEN_RPAREN,
	TOKEN_MUL,
	TOKEN_DIV,
	TOKEN_OR,
	TOKEN_XOR,
	TOKEN_AND,
	TOKEN_NEWLINE
};

/* tokens containing one or two characters */
static const char *tcchars = "+-";
static const int tc1toks[] = { TOKEN_PLUS, TOKEN_MINUS };
static const int tc2toks[] = { TOKEN_PPLUS, TOKEN_MMINUS };

/* numeric chars */
static const char *basechars = "dbx";
static const int basenums[] = { 10, 2, 16 };
static const char *digitchars = "0123456789abcdef";

static const char *
strnchr(const char *str, unsigned int n, char c) {
	int i;
	for (i = 0; i < n && str[i] != '\0'; i++) {
		if (str[i] == c) {
			return &str[i];
		}
	}
	return NULL;
}

enum lexstate {
	STATE_START = 0,
	STATE_TWOCHAR,
	STATE_NUM_BASE,
	STATE_NUM
};

int
lex (FILE *fp, char **tokstr) {
	void *ptr;
	int state = 0;
	int i = 0, p = 0, c = 0;
	int base = 10, n = 0;

	while (!feof(fp)) {
		p = c;
		c = getc(fp);
		switch (state) {
		case STATE_START:
			if ((ptr = strchr(scchars, c)) != NULL) {
				i = (int)((char *)ptr - scchars);
				return sctoks[i];
			}
			
			if ((ptr = strchr(tcchars, c)) != NULL) {
				i = (int)((char *)ptr - tcchars);
				state = STATE_TWOCHAR;
				continue;
			}

			if (c == '0') {
				base = 8;
				state = STATE_NUM_BASE;
			} else if (c == '$') {
				base = 16;
				state = STATE_NUM;
			}


			break;
		case STATE_TWOCHAR:
			if (c == p) {
				return tc2toks[i];
			} else {
				return tc1toks[i];
			}
			break;
		case STATE_NUM_BASE:
			if ((ptr = strchr(basechars, c)) != NULL) {
				i = (int)((char *)ptr - basechars);
				base = basenums[i];
				state = STATE_NUM;
				break;
			}
			/* fall through */
		case STATE_NUM:
			state = STATE_NUM;
			c = tolower(c);
			if ((ptr = strnchr(digitchars, base, c)) == NULL) {
				if ((ptr = strnchr(digitchars, base, p)) == NULL) {
					/* handle error */
					return TOKEN_ERROR;
				}
				c = ungetc(c, fp);
				if (c == EOF) {
					return TOKEN_EOF;
				}
				return TOKEN_NUMBER;
			}
			i = (int)((char *)ptr - digitchars);
			/* do something with digit */
			break;
		default:
			c = ungetc(c, fp);
			if (c == EOF) {
				return TOKEN_EOF;
			}
			return TOKEN_ERROR;
		}
	}
	return TOKEN_EOF;
}

#if 0
struct lexcontext {
	FILE *fp;
};

struct lexernode {
	char *sequence;
	int method;
	int token;

	struct lexernode *paths;
	unsigned int pathcount;
	unsigned int pathsize;
};

#define METHOD_CONTIGUOUS 0
#define METHOD_RANGE 1
{struct sequence {
	const char *seq;
	int method;
	int token;
};

static struct sequence commonseqs[] = {
	{ " \r\t\v\f", METHOD_RANGE, TOKEN_SKIP },
	{ "\n", METHOD_CONTIGUOUS, TOKEN_NEWLINE },
	{ ",", METHOD_CONTIGUOUS, TOKEN_COMMA },
	{ ":", METHOD_CONTIGUOUS, TOKEN_COLON },
	{ "[", METHOD_CONTIGUOUS, TOKEN_LSQBR },
	{ "]", METHOD_CONTIGUOUS, TOKEN_RSQBR },
	{ "(", METHOD_CONTIGUOUS, TOKEN_LPAREN },
	{ ")", METHOD_CONTIGUOUS, TOKEN_RPAREN },
	{ "-", METHOD_CONTIGUOUS, TOKEN_MINUS },
	{ "--", METHOD_CONTIGUOUS, TOKEN_MMINUS },
	{ "+", METHOD_CONTIGUOUS, TOKEN_PLUS },
	{ "++", METHOD_CONTIGUOUS, TOKEN_PPLUS },
	{ "*", METHOD_CONTIGUOUS, TOKEN_MUL },
	{ "/", METHOD_CONTIGUOUS, TOKEN_DIV },
	{ "^", METHOD_CONTIGUOUS, TOKEN_XOR },
	{ "&", METHOD_CONTIGUOUS, TOKEN_AND },
	{ "|", METHOD_CONTIGUOUS, TOKEN_OR }
};
static size_t commoncount = sizeof(commonseqs) / sizeof(struct sequence);

static struct lexernode *lexer = NULL;

#define traverse() traverse2(lexer)
void
traverse2(struct lexernode *node)
{
}

static int
lexdeinit(void)
{
	return 0;
}

static struct lexernode *
lexmknod(const char *seq, int nchar)
{
	struct lexernode *node = NULL;
	int method = METHOD_RANGE;

	if (nchar == 0) {
		nchar = strlen(seq);
		method = METHOD_CONTIGUOUS;
	}
	node = malloc(sizeof(struct lexernode));
	if (!node) {
		perror(NULL);
		return NULL;
	}

	node->sequence = calloc(nchar + 1, sizeof(char));
	strncpy(node->sequence, seq, nchar);
	node->method = method;
	node->token = token;

	node->paths = NULL;
	node->pathcount = -1;
	node->pathsize = -1;

	return node;
}

static int
lexaddsequence(const char *seq, int method, int token)
{
	int i = 0;
	char *c;
	struct lexernode *preceding = NULL, *current = lexer;

	for (c = seq; *c != '\0'; c++) {
		if (current == NULL) {
			current = lexmknod(c, method, preceding);
			if (lexer == NULL) {
				lexer = current;
			}
			continue;
		}

		if (lexer->seq[i] == '\0' || lexer->seq[i] != *c) {
			break;
		}
	}

	/* add new node */

	return 0;
}

static int
lexinit(void)
{
	int i;
	for (i = 0; i < commoncount; i++) {
		lexaddsequence(commonseqs[i].seq, commonseqs[i].method, commonseqs[i].token);
	}

	// add remaining sequences

	return 0;
}

int
tokget(struct lexcontext *context, struct token *tok)
{
	if (lexer == NULL && lexinit()) {
		die("Failed to initialize lexer\n");
	}

	return -1;
}
#endif

