/* NATURAL GRAMMARS

mpc lets us write grammars in a more natural form by letting us specify everything in one long string
When using this method, we don't have to worry about how to join or discard inputs, that's all done for us!
*/
#include "../mpc.h"

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


int main(int argc, char** argv) {
	/* Create and name rules */

	mpc_parser_t* Adjective = mpc_new("adjective");
	mpc_parser_t* Noun = mpc_new("noun");
	mpc_parser_t* Phrase = mpc_new("phrase");
	mpc_parser_t* Doge = mpc_new("doge");

	/* Start defining those rules!

	First argument are options flags (we use the defaults)
	Second argument is a multiline string that is the grammar specification, 
	consisting of rewrite rules.
	Name of rule, colon, definition, semicolon. */
	mpca_lang(MPCA_LANG_DEFAULT,
		"												\
			adjective	: \"wow\" | \"many\"			\
					  	| \"so\" | \"such\";			\
			noun		: \"lisp\" | \"language\"		\
						| \"book\" | \"build\" | \"c\";	\
			phrase		: <adjective>+ <noun>+ ;			\
			doge		: /^/ <phrase>* /$/ ;			\
		",
		Adjective, Noun, Phrase, Doge);
		
	puts("Doge Version 0.0.0.1");
	puts("Press Ctrl+c to Exit\n");
	
	while (1) {
	
		char* input = readline("doge> ");
		add_history(input);
		
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Doge, &r)) {
			mpc_ast_print(r.output);
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		free(input);
	}
		
	/* Special symbol breakdown!

	"ab"			The string ab is required
	'a'				The character 'a' is required
	'a' | 'b'		First 'a' is required, then 'b' is required
	'a'*			Zero or more 'a' are required
	'a'+			One or more 'a' are required
	<abba>			The rule called abba is required */	
	/* Do some parsing here .... */

	mpc_cleanup(4, Adjective, Noun, Phrase, Doge);
	
	return 0;
}

/* Description of what the input string to mpca_lang should look like is specifying grammar
	mpc uses itself internally to parse the input you give it to mpca_laang by specifying
	a grammar in code using the previous method */