#include "../mpc.h"
#include <math.h>

#ifdef _WIN32

static char buffer[2048];

char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}

void add_history(char* unused) {}

#else
#include <editline/readline.h>
#endif

/* Use operator string to see which operation to perform */
long eval_op(long x, char* op, long y) {
	if (strcmp(op, "+") == 0) { return x + y; }
	if (strcmp(op, "-") == 0) { return x - y; }
	if (strcmp(op, "neg") == 0) { return -(x) -(y); }
	if (strcmp(op, "*") == 0) { return x * y; }
	if (strcmp(op, "/") == 0) { return x / y; }
	if (strcmp(op, "%") == 0) { return x % y; }
	if (strcmp(op, "add") == 0) { return x + y; }
	if (strcmp(op, "^") == 0) { return pow(x, y); }
	if (strcmp(op, "subtract") == 0) { return x - y; }
	if (strcmp(op, "multiply") == 0) { return x * y; }
	if (strcmp(op, "divide") == 0) { return x / y; }
	if (strcmp(op, "modulo") == 0) { return x % y; }
	if (strcmp(op, "max") == 0) { return x > y ? x : y; }
	if (strcmp(op, "min") == 0) { return x < y ? x : y; }
	return 0;
}

long eval(mpc_ast_t* t) {
	/* If tagged as number return it directly. */
	if (strstr(t->tag, "number")) {
		return atoi(t->contents);
	}
	
	/* The operator is always the second child. */
	char* op = t->children[1]->contents;
	
	/* We store the third child in 'x' */
	long x = eval(t->children[2]);
	
	/* Iterate the remaining children and combining. */
	int i = 3;
	while (strstr(t->children[i]->tag, "expr")) {
		x = eval_op(x, op, eval(t->children[i]));
		i++;
	}
	
	return x;

}

int main(int argc, char** argv) {

	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");
	
	mpca_lang(MPCA_LANG_DEFAULT,
	"																		\
		number		: /-?[0-9]+/ ;											\
		operator	: '+' | '-' | '*' | '/' | '%' | '^'						\
					| \"add\" | \"subtract\" | \"multiply\" | \"neg\"		\
					| \"divide\" | \"modulo\" | \"max\" | \"min\" ;			\
		expr		: <number> | '(' <operator> <expr>+ ')' ;				\
		lisp		: /^/ <operator> <expr>+ /$/ ;							\
	",
	Number, Operator, Expr, Lisp);
	
	puts("Lisp Version 0.0.0.1");
	puts("Press Ctrl+c to Exit");
	
	while(1) {
	
	char* input = readline("Lisp> ");
	add_history(input);
  
	mpc_result_t r;
	if (mpc_parse("<stdin>", input, Lisp, &r)) {
		
		long result = eval(r.output);
		printf("%li\n", result);
		mpc_ast_delete(r.output);
		
	} else {
		mpc_err_print(r.error);
		mpc_err_delete(r.error);
	}
	
	free(input);
	}
	
	mpc_cleanup(4, Number, Operator, Expr, Lisp);
	
	return 0;
}