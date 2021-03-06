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

/* Add SYM and SEXPR as possible lval types */
/* I added LVAL_DEC in order to handle decimal numbers */
enum { LVAL_ERR, LVAL_NUM, LVAL_DEC, LVAL_SYM, LVAL_SEXPR };

typedef struct lval {
	int type;
	long num;
	/* double dec is going to be used to implement decimals */
	double dec;
	/* Error and Symbol types have some string data */
	char* err;
	char* sym;
	/* Count and Pointer to a list of "lval*" */
	int count;
	struct lval** cell;
} lval;

/* Construct a pointer to a new Number lval */
lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	/* Was a little tricky here and added a field on lval_num types that are the equivalent of x cast as a double
	My understanding is this is what allows values to convert to float/double when necessary (when one argument is a decimal) */
	v->dec = (double) x;
	return v;
}

/* Construct a pointer to a new Decimal lval */
lval* lval_dec(double y) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_DEC;
	v->dec = y;
	return v;
}

/* Construct a pointer to a new Error lval */
lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

/* Construct a pointer to a new Symbol lval */
lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

/* A pointer to a new empty Sexpr lval */
lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lval_del(lval* v) {
	switch (v->type) {
		/* Do nothing special for number types */
		case LVAL_NUM: break;
		/* LVAL_DEC is for decimal numbers, so also should not have anything special done */
		case LVAL_DEC: break;
		/* For Err or Sym free the string data */
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		
		/* If Sexpr delete all elements inside */
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			
			/* Also free the memory allocated to contain the pointers */
			free(v->cell);
		break;
		}
		
	/* Free the memory allocated for the "lval" struct itself */
	free(v);
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_pop(lval* v, int i) {
	/* Find the item at 'i' */
	lval* x = v->cell[i];
	
	/* Shift memory after the item at 'i' over the top */
	memmove(&v->cell[i], &v->cell[i+1],
	sizeof(lval*) * (v->count-i-1));
	
	/* Decrease the count of items in the list */
	v->count--;
	
	/* Reallocate the memory used */
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;	
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
}

void lval_print(lval* v);

void lval_expr_print(lval* v, char open, char close) {
	putchar(open);
	for (int i = 0; i < v->count; i++) {
		
		/* Print value contained within */
		lval_print(v->cell[i]);
		
		/* Don't print trailing space if last element */
		if (i != (v->count-1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_print(lval* v) {
	switch(v->type) {
		case LVAL_NUM: printf("%li", v->num); break;
		/* In C, %f is naturally converted to a double */
		case LVAL_DEC: printf("%f", v->dec); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
	}
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }


lval* builtin_op(lval* a, char* op) {
	
	/* Ensure all arguments are numbers */
	for (int i = 0; i < a->count; i++) {
		if (a->cell[i]->type != LVAL_NUM && a->cell[i]->type != LVAL_DEC) {
			lval_del(a);
			return lval_err("Cannot operate on non-number!");
		}
	}
	
	/* Pop the first element */
	lval* x = lval_pop(a, 0);
	
	/* If no arguments and sub then perform unary negation */
	if ((strcmp(op, "-") == 0) && a->count == 0) {
		x->num = -x->num;
		x->dec = -x->dec;
	}
	
	/* While there are still elements remaining */
	while (a->count > 0) {
	
	/* Pop the next element */
	lval* y = lval_pop(a, 0);
	
	/* In order to account for the fact that numbers being parsed may be decimals and not integers,
		you need to have the logic available for both possible outputs */
	if (strcmp(op, "+") == 0) { x->num += y->num; x->dec += y->dec; }
	if (strcmp(op, "add") == 0) { x->num += y->num; x->dec += y->dec; }
	if (strcmp(op, "-") == 0) { x->num -= y->num; x->dec -= y->dec; }
	if (strcmp(op, "subtract") == 0) { x->num -= y->num; x->dec -= y->dec; }
	if (strcmp(op, "*") == 0) { x->num *= y->num; x->dec *= y->dec; }
	if (strcmp(op, "multiply") == 0) { x->num *= y->num; x->dec *= y-> dec; }
	if (strcmp(op, "%") == 0) { x->num %= y->num; }
	if (strcmp(op, "^") == 0) {
	/* If x->num doesn't equal x->dec, we know it's a decimal, so we should use lval_dec to make the pointer */
		if (x->num != x->dec) {
			x = lval_dec(pow(x->dec, y->dec)); 
		} else {
			x = lval_num(pow(x->num, y->num));
		}
	}
	if (strcmp(op, "modulo") == 0) { x->num %= y->num; }
	if (strcmp(op, "divide") == 0) {
		if (y->num == 0) {
			lval_del(x); lval_del(y);
			x = lval_err("Division by zero!"); break;
		}
		x->num /= y->num;
		if (y->dec == 0.00) {
			lval_del(x); lval_del(y);
			x = lval_err("Division by zero!"); break;
		}
		x->dec /= y->dec;
	}
	if (strcmp(op, "/") == 0) {
		if (y->num == 0) {
			lval_del(x); lval_del(y);
			x = lval_err("Division by zero!"); break;
			}
			x->num /= y->num;
		if (y->dec == 0.00) {
			lval_del(x); lval_del(y);
			x = lval_err("Division by zero!"); break;
			}
			x->dec /= y->dec;
		}
		lval_del(y);
	}
	lval_del(a); return x;
}

lval* lval_eval(lval* v);

lval* lval_eval_sexpr(lval* v) {
	
	/* Evaluate children */
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}
	
	/* Error checking */
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
	}
	
	/* Empty expression */
	if (v->count == 0) { return v; }
	
	/* Single expression */
	if (v->count == 1) { return lval_take(v, 0); }
	
	/* Ensure first element is symbol */
	lval* f = lval_pop(v, 0);
	if (f->type != LVAL_SYM) {
		lval_del(f); lval_del(v);
		return lval_err("S-expression does not start with symbol!");
	}
	
	/* Call builtin with operator */
	lval* result = builtin_op(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v) {
	/* Evaluate Sexpressions */
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
	
	/* All other lval types remain the same */
	return v;
}

lval* lval_read_dec(mpc_ast_t* t) {
	errno = 0;
	double x = strtod(t->contents, NULL);
	return errno != ERANGE ? lval_dec(x) : lval_err("invalid number");
}
lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
	/* If Symbol or Number return conversion to that type */
	if (strstr(t->tag, "number")) { 
	/* Bit cheating, but if the contents contain a period, it's a decimal and should be read with lval_read_dec */
		if (strstr(t->contents, ".")) { 
			return lval_read_dec(t); 
		}
		else { 
			return lval_read_num(t); 
		}
	}
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
	
	/* If root (>) or sexpr, create empty list */
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
	
	/* Fill this list with any valid expression contained within */
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	
	return x;
}


int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	/* For number, we have to add the option of a period in order to accept decimal numbers */
	mpca_lang(MPCA_LANG_DEFAULT,
		"														\
			number	: /-?[0-9.]+/ ;								\
			symbol	: '+' | '-' | '*' | '/' | '%' | '^'			\
					| \"add\" | \"subtract\" | \"multiply\"		\
					| \"divide\" | \"modulo\" ;					\
			sexpr	: '(' <expr>* ')' ;							\
			expr	: <number> | <symbol> | <sexpr> ;			\
			lisp	: /^/ <expr>* /$/ ;							\
		",
		Number, Symbol, Sexpr, Expr, Lisp);
	
	puts("Lisp Version 0.0.0.5");
	puts("Press Ctrl+C to exit\n");
	
	while(1) {
		char* input = readline("Lisp> ");
		add_history(input);
		
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lisp, &r)) {
			lval* x = lval_eval(lval_read(r.output));
			lval_println(x);
			lval_del(x);
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		
		free(input);
	}
	
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lisp);
	
	return 0;
}