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
	/* Print Version and Exit Information */
	puts("Lisp Version 0.0.0.1");
	puts("Press Ctrl+c to Exit");
	puts("Note that Lisp has not been taught any commands.");
	puts("Lisp has a thirst for knowledge but the brain of a sieve.\n");
	
	/* In a never-ending loop ... */
	while (1) {
		/* Now in either case readline will be defined */
		/* Output our prompt and get input */
		char* input = readline("Lisp> ");
		
		/* Add input to history */
		add_history(input);
		
		/* Echo input back to user */
		printf("%s is an invalid command.\n", input);
		
		/* Free retrieved input */
		free(input);
	}
	return 0;
}