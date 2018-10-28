#include "../mpc.h"

#include <stdio.h>
#include <stdlib.h>

/* If we are compiling on Windows, compile these functions */
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
	fputs(prompt, stdout);
	fgets(buffer, 2048, stdin);
	char* cpy = malloc(strlen(buffer)+1);
	strcpy(cpy, buffer);
	cpy[strlen(cpy)-1] = '\0';
	return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

/* Otherwise include the editline headers */
#else
#include <editline/readline.h>
#endif

int main(int argc, char** argv) {

	/* Create some parsers */
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Operator = mpc_new("operator");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");
	
	/* Define them with the following Language */
	mpca_lang(MPCA_LANG_DEFAULT,
		"															\
			number		: /-?[0-9]+\.?[0-9]+/ ;						\
			operator	: '+' | '-' | '*' | '/' | '%'				\
						| \"add\" | \"sub\" | \"mul\"				\
						| \"div\" | \"modulo\";						\
			expr		: <number> | '(' <operator> <expr>+ ')' ;	\
			lisp		: /^/ <operator> <expr>+ /$/ ;				\
		",
		Number, Operator, Expr, Lisp);
					
	/* Print Version and Exit Information */
	puts("Lisp Version 0.0.0.1");
	puts("Press Ctrl+c to Exit");
	
	/* In a never-ending loop ... */
	while (1) {
		/* Now in either case readline will be defined */
		/* Output our prompt and get input */
		char* input = readline("Lisp> ");
		
		/* Add input to history */
		add_history(input);
		
		/* Attempt to parse the user input */
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lisp, &r)) {
			/* On success, print the AST */
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			/* Otherwise print the error */
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		/* Free retrieved input */
		free(input);
			
	}
	
	/* Undefine and delete our parsers */
	mpc_cleanup(4, Number, Operator, Expr, Lisp);
	
	return 0;
}