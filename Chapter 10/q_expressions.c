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

/* Add QEXP as possible lval type */
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR, LVAL_QEXPR };

typedef struct lval {
	int type;
	long num;
	char* err;
	char* sym;
	int count;
	struct lval** cell;
} lval;

lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* m) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	v->err = malloc(strlen(m) + 1);
	strcpy(v->err, m);
	return v;
}

lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

/* A pointer to a new empty Qexpr lval */
lval* lval_qexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lval_del(lval* v) {
	switch(v->type) {
		case LVAL_NUM: break;
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		
		/* If Qexpr or Sexpr then delete all elements inside */
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			/* Also free the memory allocated to contain the pointers */
			free(v->cell);
		break;		
	}
	
	free(v);
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_join(lval* x, lval* y) {
	/* For each cell in 'y' add it to 'x' */
	while (y->count) {
		x = lval_add(x, lval_pop(y, 0));
	}
	
	/* Delete the empty 'y' and return 'x' */
	lval_del(y);
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
		lval_print(v->cell[i]);
		if (i != (v->count-1)) {
			putchar(' ');
		}
	}
	putchar(close);
}

void lval_print(lval* v) {
	switch(v->type) {
		case LVAL_NUM: printf("%li", v->num); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_expr_print(v, '(', ')'); break;
		case LVAL_QEXPR: lval_expr_print(v, '{', '}'); break;
	}
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

#define LASSERT(args, cond, err) \
	if (!(cond)) { lval_del(args); return lval_err(err); }
	
#define INCORRECTARGNUM(args, count, err) \
	if (count > 1) { lval_del(args); return lval_err(err); }
	
#define EMPTYLIST(args, count, err) \
	if (count == 0) { lval_del(args); return lval_err(err); }
	
lval* lval_eval(lval* v);

lval* builtin_list(lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_head(lval* a) {
	// LASSERT(a, a->count == 1, "Function 'head' passed too many arguments!");
	INCORRECTARGNUM(a, a->count, "Function 'head' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type!");
	EMPTYLIST(a, a->cell[0]->count, "Function 'head' passed {}, which is an empty list!");
	//LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");
	
	lval* v = lval_take(a, 0);
	while (v->count > 1) { lval_del(lval_pop(v, 1)); }
	return v;
}
/* Original code for builtin_head sans macro

lval* builtin_head(lval* a) {
	// Check error conditions
	if (a->count != 1) {
		lval_del(a);
		return lval_err("Function 'head' passed too many arguments!");
	}
	
	if (a->cell[0]->type != LVAL_QEXPR) {
		lval_del(a);
		return lval_err("Function 'head' passed incorrect types!");
	}
	
	if (a->cell[0]->count == 0) {
		lval_del(a);
		return lval_err("Function 'head' passed {}!");
	}
	
	// Otherwise take first argument
	lval* v = lval_take(a, 0);
	
	// Delete all elements that are not head and return
	while (v->count > 1) { lval_del(lval_pop(v, 1)); }
	return v;
}
*/

lval* builtin_tail(lval* a) {
	// LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
	INCORRECTARGNUM(a, a->count, "Function 'tail' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type!");
	EMPTYLIST(a, a->cell[0]->count, "Function 'tail' passed {}, which is an empty list!");
	//LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}!");
	
	lval* v = lval_take(a, 0);
	lval_del(lval_pop(v, 0));
	return v;
}
/* Original code for builtin_tail sans macro
lval* builtin_tail(lval* a) {
	// Check error conditions 
	if (a->count != 1) {
		lval_del(a);
		return lval_err("Function 'tail' passed too many arguments!");
	}
	
	if (a->cell[0]->type != LVAL_QEXPR) {
		lval_del(a);
		return lval_err("Function 'tail' passed incorrect types!");
	}
	
	if (a->cell[0]->count == 0) {
		lval_del(a);
		return lval_err("Function 'tail' passed {}!");
	}
	
	// Take first argument 
	lval* v = lval_take(a, 0);
	
	// Delete first element and return 
	lval_del(lval_pop(v, 0));
	return v;
} 
*/

lval* builtin_last(lval* a) {
	INCORRECTARGNUM(a, a->count, "Function 'last' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type!");
	
	lval* x = lval_take(a, a->count - 1);
	while (x->count > 1) {
		lval_del(lval_pop(x, 0));
	}
	
	return x;
}

lval* builtin_init(lval* a) {
	INCORRECTARGNUM(a, a->count, "Function 'init' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type!");
	
	lval* x = lval_take(a, 0);
	lval_del(lval_pop(x, x->count - 1));
	return x;
}

// lval* builtin_cons(lval* a) {
// 	if (a->count != 2) {
// 		lval_del(a);
// 		return lval_err("Function 'cond' needs two arguments"); }
// 	
// 	lval* x = lval_take(a, 0);
// 	while (a->count) {
// 		x = lval_join(x, lval_take(a, 0));
// 	}
// 	
// 	lval_del(a);
// 	
// 	return x;
// }

lval* builtin_eval(lval* a) {
	// LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
	INCORRECTARGNUM(a, a->count, "Function 'eval' passed too many arguments!");
	LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type!");
	
	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(x);
}

lval* builtin_join(lval* a) {
	for (int i = 0; i < a->count; i++) {
		LASSERT(a, a->cell[i]->type == LVAL_QEXPR, "Function 'join' passed incorrect type.");
	}
	lval* x = lval_pop(a, 0);
	
	while (a->count) {
		x = lval_join(x, lval_pop(a, 0));
	}
	
	lval_del(a);
	return x;
}

lval* builtin_op(lval* a, char* op) {
	for (int i = 0; i < a->count; i++) {
		if(a->cell[i]->type != LVAL_NUM) {
			lval_del(a);
			return lval_err("Cannot operate on non-number!");
		}
	}
	lval* x = lval_pop(a, 0);
	if ((strcmp(op, "-") == 0) && a->count == 0) { x->num = -x->num; }
	
	while (a->count > 0) {
		lval* y = lval_pop(a, 0);
		
		if (strcmp(op, "+") == 0) { x->num += y->num; }
		if (strcmp(op, "-") == 0) { x->num -= y->num; }
		if (strcmp(op, "*") == 0) { x->num *= y->num; }
		if (strcmp(op, "/") == 0) {
			if (y->num == 0) {
				lval_del(x); lval_del(y);
				x = lval_err("Division by zero!");
				break;
				}
				x->num /= y->num;
		}
		lval_del(y);
	}
	lval_del(a);
	return x;
}

lval* builtin(lval* a, char* func) {
	if (strcmp("list", func) == 0) { return builtin_list(a); }
	if (strcmp("head", func) == 0) { return builtin_head(a); }
	if (strcmp("tail", func) == 0) { return builtin_tail(a); }
	if (strcmp("join", func) == 0) { return builtin_join(a); }
	if (strcmp("last", func) == 0) { return builtin_last(a); }
	if (strcmp("eval", func) == 0) { return builtin_eval(a); }
	if (strcmp("init", func) == 0) { return builtin_init(a); }
// 	if (strcmp("cons", func) == 0) { return builtin_cons(a); }
	if (strstr("+-/*", func)) { return builtin_op(a, func); }
	lval_del(a);
	return lval_err("Unknown function!");
}

lval* lval_eval_sexpr(lval* v) {
	for (int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(v->cell[i]);
	}
	
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
	}
	
	if (v->count == 0) { return v; }
	
	if (v->count == 1) { return lval_take(v, 0); }
	
	lval* f = lval_pop(v, 0);
	if (f->type != LVAL_SYM) {
		lval_del(f); lval_del(v);
		return lval_err("S-expression does not start with symbol.");
	}
	
	/* Call builtin with operator */
	lval* result = builtin(v, f->sym);
	lval_del(f);
	return result;
}

lval* lval_eval(lval* v) {
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(v); }
	return v;
}

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("invalid number");
}

lval* lval_read(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
	
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
	if (strstr(t->tag, "qexpr")) { x = lval_qexpr(); }
	
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->tag, "regex") == 0) { continue; }
		x = lval_add(x, lval_read(t->children[i]));
	}
	
	return x;
}


int main(int argc, char** argv) {
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* Symbol = mpc_new("symbol");
	mpc_parser_t* Sexpr = mpc_new("sexpr");
	mpc_parser_t* Qexpr = mpc_new("qexpr");
	mpc_parser_t* Expr = mpc_new("expr");
	mpc_parser_t* Lisp = mpc_new("lisp");

	mpca_lang(MPCA_LANG_DEFAULT,
		"																\
			number		: /-?[0-9]+/ ;									\
			symbol		: \"list\" | \"head\" | \"tail\" | \"join\"		\
						| \"eval\" | \"last\" | '+' | '-' | '*' | '/'   \
						| \"init\" | \"cons\";									\
			sexpr		: '(' <expr>* ')' ;								\
			qexpr		: '{' <expr>* '}' ;								\
			expr		: <number> | <symbol> | <sexpr> | <qexpr> ;		\
			lisp		: /^/ <expr>* /$/ ;								\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lisp);
		
	puts("Lisp Version 0.0.0.6");
	puts("Press Ctrl+c to exit\n");
	
	while (1) {
		
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
	
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lisp);
	
	return 0;
	
}