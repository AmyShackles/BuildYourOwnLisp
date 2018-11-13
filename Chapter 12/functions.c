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

/* Functions should consist of three parts:
1) List of formal arguments which need to be bound before evaluation
2) A Q-Expression that represents the body of the function
3) The location to store the values assigned to the formal arguments (environment) */


/* Forward declarations */
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Lisp Value */

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

// To get an lval* we dereference lbuiltin nad call it with a lenv* and a lval*
// lbuiltin must be a function pointer that takes a lenv* and a lval* and returns an lval*

typedef lval*(*lbuiltin)(lenv*, lval*);


/* We need to store builtin functions and user-defined functions under the same type, 
so we need a way to internally differentiate between them.  
To this end, we check to see if the lbuiltin function pointer is NULL or not.
If the lbuiltin function pointer is NULL, it's a user-defined function - else, a builtin. */

struct lval {
	int type;
	
	/* Basic */
	long num;
	char* err;
	char* sym;
	
	/* Function */
	lbuiltin builtin;
	lenv* env;
	lval* formals;
	lval* body;
	
	/* Expression */
	int count;
	lval** cell;
};

lval* lval_num(long x) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_NUM;
	v->num = x;
	return v;
}

lval* lval_err(char* fmt, ...) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_ERR;
	
	/* Create a va list and initialize it */
	va_list va;
	va_start(va, fmt);
	
	/* Allocate 512 bytes of space */
	v->err = malloc(512);
	
	/* Print the error string with a maximum of 511 characters */
	vsnprintf(v->err, 511, fmt, va);
	
	/* Reallocate to number of bytes actually used */
	v->err = realloc(v->err, strlen(v->err)+1);
	
	/* Clean up our va list */
	va_end(va);
	
	return v;
}

lval* lval_sym(char* s) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SYM;
	v->sym = malloc(strlen(s) + 1);
	strcpy(v->sym, s);
	return v;
}

lval* lval_builtin(lbuiltin func) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->builtin = func;
	return v;
}

/* Need to create a constructor for user defined lval functions */

lenv* lenv_new(void);

lval* lval_lambda(lval* formals, lval* body) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	
	/* Set Builtin to Null */
	v->builtin = NULL;
	
	/* Build new environment */
	v->env = lenv_new();
	
	/* Set formals and body */
	v->formals = formals;
	v->body = body;
	return v;
}

lval* lval_sexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_SEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

lval* lval_qexpr(void) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_QEXPR;
	v->count = 0;
	v->cell = NULL;
	return v;
}

void lenv_del(lenv* e);

void lval_del(lval* v) {
	switch(v->type) {
		case LVAL_NUM: break;
		case LVAL_FUN:
			if (!v->builtin) {
				lenv_del(v->env);
				lval_del(v->formals);
				lval_del(v->body);
			}
		break;
		case LVAL_ERR: free(v->err); break;
		case LVAL_SYM: free(v->sym); break;
		case LVAL_QEXPR:
		case LVAL_SEXPR:
			for (int i = 0; i < v->count; i++) {
				lval_del(v->cell[i]);
			}
			free(v->cell);
		break;
	}
	free(v);
}

lenv* lenv_copy(lenv* e);

lval* lval_copy(lval* v) {
	lval* x = malloc(sizeof(lval));
	x->type = v->type;
	
	switch(v->type) {
		case LVAL_NUM: x->num = v->num; break;
		case LVAL_FUN:
			if (v->builtin) {
				x->builtin = v->builtin;
			} else {
				x->builtin = NULL;
				x->env = lenv_copy(v->env);
				x->formals = lval_copy(v->formals);
				x->body = lval_copy(v->body);
			}
		break;
		case LVAL_ERR:
			x->err = malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err); 
		break;	
		case LVAL_SYM:
			x->sym = malloc(strlen(v->sym) + 1);
			strcpy(x->sym, v->sym); 
		break;	
		case LVAL_SEXPR:
		case LVAL_QEXPR:
			x->count = v->count;
			x->cell = malloc(sizeof(lval*) * x->count);
			for (int i = 0; i < x->count; i++) {
				x->cell[i] = lval_copy(v->cell[i]);
			}
		break;
	}
	return x;
}

lval* lval_add(lval* v, lval* x) {
	v->count++;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	v->cell[v->count-1] = x;
	return v;
}

lval* lval_join(lval* x, lval* y) {
	for (int i = 0; i < y->count; i++) {
		x = lval_add(x, y->cell[i]);
	}
	free(y->cell);
	free(y);
	return x;
}

lval* lval_pop(lval* v, int i) {
	lval* x = v->cell[i];
	memmove(&v->cell[i], &v->cell[i+1], sizeof(lval*) * (v->count-i-1));
	v->count--;
	v->cell = realloc(v->cell, sizeof(lval*) * v->count);
	return x;
}

lval* lval_take(lval* v, int i) {
	lval* x = lval_pop(v, i);
	lval_del(v);
	return x;
}

void lval_print(lval* v);

void lval_print_expr(lval* v, char open, char close) {
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
		case LVAL_FUN:
			if (v->builtin) {
				printf("<builtin>");
			} else {
				printf("(\\ "); lval_print(v->formals);
				putchar(' '); lval_print(v->body); putchar(')');
			}
		break;
		case LVAL_NUM: printf("%li", v->num); break;
		case LVAL_ERR: printf("Error: %s", v->err); break;
		case LVAL_SYM: printf("%s", v->sym); break;
		case LVAL_SEXPR: lval_print_expr(v, '(', ')'); break;
		case LVAL_QEXPR: lval_print_expr(v, '{', '}'); break;
	}
}

void lval_println(lval* v) { lval_print(v); putchar('\n'); }

char* ltype_name(int t) {
	switch(t) {
		case LVAL_FUN: return "Function";
		case LVAL_NUM: return "Number";
		case LVAL_ERR: return "Error";
		case LVAL_SYM: return "Symbol";
		case LVAL_SEXPR: return "S-Expression";
		case LVAL_QEXPR: return "Q-Expression";
		default: return "Unknown";	
	}
}

struct lenv {
	lenv* par;
	int count;
	char** syms;
	lval** vals;
};

lenv* lenv_new(void) {
	lenv* e = malloc(sizeof(lenv));
	e->par = NULL;
	e->count = 0;
	e->syms = NULL;
	e->vals = NULL;
	return e;
}

void lenv_del(lenv* e) {
	for (int i = 0; i < e->count; i++) {
		free(e->syms[i]);
		lval_del(e->vals[i]);
	}
	free(e->syms);
	free(e->vals);
	free(e);
}

/* Because we have a new lval type that has its own environment, we need a way to copy 
the environments for when we copy lval structs */

lenv* lenv_copy(lenv* e) {
	lenv* n = malloc(sizeof(lenv));
	n->par = e->par;
	n->count = e->count;
	n->syms = malloc(sizeof(char*) * n->count);
	n->vals = malloc(sizeof(lval*) * n->count);
	for (int i = 0; i < e->count; i++) {
		n->syms[i] = malloc(strlen(e->syms[i]) + 1);
		strcpy(n->syms[i], e->syms[i]);
		n->vals[i] = lval_copy(e->vals[i]);
	}
	return n;
}

lval* lenv_get(lenv* e, lval* k) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			return lval_copy(e->vals[i]);
		}
		if (strcmp(k->sym, "exit") == 0) {
			printf("Exiting the program... \n\n");
			exit(0);
		}
	}
	
	if (e->par) {
		return lenv_get(e->par, k);
	} else {
		return lval_err("Unbound symbol '%s'", k->sym);
	}
}



void lenv_put(lenv* e, lval* k, lval* v) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			lval_del(e->vals[i]);
			e->vals[i] = lval_copy(v);
			return;
		}
	}
	
	e->count++;
	e->vals = realloc(e->vals, sizeof(lval*) * e->count);
	e->syms = realloc(e->syms, sizeof(char*) * e->count);
	
	e->vals[e->count-1] = lval_copy(v);
	e->syms[e->count-1] = malloc(strlen(k->sym)+1);
	strcpy(e->syms[e->count-1], k->sym);
}

/* Now there are two ways of defining a variable - either locally or globally.
We can keep lenv_put the same as it can be used for definition in the local environment.
But we need a new function for definition in the global environment that will follow the parent 
chain up before using lenv_put to define locally */

void lenv_def(lenv* e, lval* k, lval* v) {
	while (e->par) {
		e = e->par;
	}
	lenv_put(e, k, v);
}


/* Builtins */

#define LASSERT(args, cond, fmt, ...) \
	if (!(cond)) { \
		lval* err = lval_err(fmt, ##__VA_ARGS__); lval_del(args); return err; }

#define LASSERT_TYPE(func, args, index, expect) \
	LASSERT(args, args->cell[index]->type == expect, \
		"Function '%s' passed incorrect type for argument %i.  \
		Got %s, Expected %s.", \
		func, index, ltype_name(args->cell[index]->type), ltype_name(expect))
		
#define LASSERT_NUM(func, args, num) \
	LASSERT(args, args->count == num, \
		"Function '%s' passed incorrect number of arguments.  \
		Got %i, Expected %i.", \
		func, args->count, num)
		
#define LASSERT_NOT_EMPTY(func, args, index) \
	LASSERT(args, args->cell[index]->count != 0, \
		"Function '%s' passed {} for argument %i.", func, index);
		
lval* lval_eval(lenv* e, lval* v);

/* Need to create a builtin for lambda function: 
Should take as input a list of symbols and a list that represents the code and then return a function lval.
Do error checking to make sure the count and type of arguments are correct and then if so:
Pop the first two arguments from the list and pass them to previously defined function lval_lambda */

lval* builtin_lambda(lenv* e, lval* a) {
	/* Check two arguments, each of which are Q-expressions */
	LASSERT_NUM("\\", a, 2);
	LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
	LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);
	
	/* Check that the first QEXPR contains only symbols */
	for (int i = 0; i < a->cell[0]->count; i++) {
		LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
			"Cannot define non-symbol.  Got %s, Expected %s.",
			ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
	}
	
	/* Pop first two arguments and pass them to lval_lambda */
	lval* formals = lval_pop(a, 0);
	lval* body = lval_pop(a, 0);
	lval_del(a);
	
	return lval_lambda(formals, body);
}

lval* builtin_list(lenv* e, lval* a) {
	a->type = LVAL_QEXPR;
	return a;
}

lval* builtin_head(lenv* e, lval* a) {
	LASSERT_NUM("head", a, 1);
	LASSERT_TYPE("head", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("head", a, 0);
	
	lval* v = lval_take(a, 0);
	while (v->count > 1) { lval_del(lval_pop(v, 1)); }
	return v;
}

lval* builtin_tail(lenv* e, lval* a) {
	LASSERT_NUM("tail", a, 1);
	LASSERT_TYPE("tail", a, 0, LVAL_QEXPR);
	LASSERT_NOT_EMPTY("tail", a, 0);
	
	lval* v = lval_take(a, 0);
	lval_del(lval_pop(v, 0));
	return v;
}

lval* builtin_eval(lenv* e, lval* a) {
	LASSERT_NUM("eval", a, 1);
	LASSERT_TYPE("eval", a, 0, LVAL_QEXPR);
	
	lval* x = lval_take(a, 0);
	x->type = LVAL_SEXPR;
	return lval_eval(e, x);
}

lval* builtin_join(lenv* e, lval* a) {
	for (int i = 0; i < a->count; i++) {
		LASSERT_TYPE("join", a, i, LVAL_QEXPR);
	}
	lval* x = lval_pop(a, 0);
	
	while(a->count) {
		lval* y = lval_pop(a, 0);
		x = lval_join(x, y);
	}
	
	lval_del(a);
	return x;
}

lval* builtin_op(lenv* e, lval* a, char* op) {
	for (int i = 0; i < a->count; i++) {
		LASSERT_TYPE(op, a, i, LVAL_NUM);
	}
	lval* x = lval_pop(a, 0);
	
	if ((strcmp(op, "-") == 0) && a->count == 0) {
		x->num = -x->num;
	}
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

lval* builtin_add(lenv* e, lval* a) {
	return builtin_op(e, a, "+");
}

lval* builtin_sub(lenv* e, lval* a) {
	return builtin_op(e, a, "-");
}

lval* builtin_mul(lenv* e, lval* a) {
	return builtin_op(e, a, "*");
}

lval* builtin_div(lenv* e, lval* a) {
	return builtin_op(e, a, "/");
}

lval* builtin_var(lenv* e, lval* a, char* func) {
	LASSERT_TYPE(func, a, 0, LVAL_QEXPR);
	
	lval* syms = a->cell[0];
	for (int i = 0; i < syms->count; i++) {
		LASSERT(a, (syms->cell[i]->type == LVAL_SYM),
			"Function '%s' cannot define non-symbol. "
			"Got %s, Expected %s.", func,
			ltype_name(syms->cell[i]->type),
			ltype_name(LVAL_SYM));
	}
	
	LASSERT(a, (syms->count == a->count-1),
	"Function '%s' passed too many arguments for symbols. "
	"Got %i, Expected %i.", func, syms->count, a->count-1);
	
	for (int i = 0; i < syms->count; i++) {
		/* If 'def' define globally, if 'put' define locally */
		if (strcmp(func, "def") == 0) {
			lenv_def(e, syms->cell[i], a->cell[i+1]);
		}
		
		if (strcmp(func, "=") == 0) {
			lenv_put(e, syms->cell[i], a->cell[i+1]);
		}
	}
	
	lval_del(a);
	return lval_sexpr();
}

/* Should add another builtin for local assignments - put in C, but = in Lisp */


lval* builtin_def(lenv* e, lval* a) {
	return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
	return builtin_var(e, a, "=");
}


void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
	lval* k = lval_sym(name);
	lval* v = lval_builtin(func);
	lenv_put(e, k, v);
	lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
	lenv_add_builtin(e, "\\", builtin_lambda);	
	lenv_add_builtin(e, "def", builtin_def);
	lenv_add_builtin(e, "=", builtin_put);
	
	lenv_add_builtin(e, "list", builtin_list);
	lenv_add_builtin(e, "head", builtin_head);
	lenv_add_builtin(e, "tail", builtin_tail);
	lenv_add_builtin(e, "eval", builtin_eval);
	lenv_add_builtin(e, "join", builtin_join);
	
	lenv_add_builtin(e, "+", builtin_add);
	lenv_add_builtin(e, "-", builtin_sub);
	lenv_add_builtin(e, "*", builtin_mul);
	lenv_add_builtin(e, "/", builtin_div);

}



/* Need to write code for when expression gets evaluated and lval is called.
For builtins, process stays the same.
For user-defined functions, we need to bind each argument passed in to each of the symbols
in the forms field and then evaluate the body using the env field as an environment
and the calling environment as a parent. */

/* This is replaced by an lval_call function that forms partially evaluated functions
in the event that fewer arguments are supplied than the number of symbols in the 
user-defined function

lval* lval_call(lenv* e, lval* f, lval* a) {
	// If builtin, call it
	if (f->builtin) { return f->builtin(e, a); }
	
	// Assign each argument to each formal in order
	for (int i = 0; i < a->count; i++) {
		lenv_put(f->env, f->formals->cell[i], a->cell[i]);
	}
	
	lval_del(a);
	
	// Set the parent environment
	f->env->par = e;
	
	// Evaluate the body 
	return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
} // Note, this will crash if number of arguments and number of formals differ */

/* Two ways of approaching this problem - 
1) Throwing an error whenever the number of arguments is incorrect
2) Allowing for the partial evaluation of functions - binding what arguments are provided 
and replacing the function with a more complete one containing some bound variables */

lval* lval_call(lenv* e, lval* f, lval* a) {
	/* If builtin, apply that */
	if (f->builtin) { return f->builtin(e, a); }
	
	/* Record argument counts */
	int given = a->count;
	int total = f->formals->count;
	
	/* While arguments still remain to be processed */
	while (a->count) {
		
		/* If we've run out of formal arguments to bind */
		if (f->formals->count == 0) {
			lval_del(a); 
			return lval_err(
				"Function passed too many arguments. "
				"Got %i, Expected %i.", given, total);
		}
		
		/* Pop the first symbol from the formals */
		lval* sym = lval_pop(f->formals, 0);
		
		/* Special case to deal with '&' */
		if (strcmp(sym->sym, "&") == 0) {
			/* Ensure '&' is followed by another symbol */
			if (f->formals->count != 1) {
				lval_del(a);
				return lval_err("Function format invalid. "
					"Symbol '&' not followed by single symbol.");
			}
			
			/* Next formal should be bound to remaining arguments */
			lval* nsym = lval_pop(f->formals, 0);
			lenv_put(f->env, nsym, builtin_list(e, a));
			lval_del(sym); lval_del(nsym);
			break;
		}
		
		/* Pop the next argument from the list */
		lval* val = lval_pop(a, 0);
		
		/* Bind a copy into the function's environment */
		lenv_put(f->env, sym, val);
		
		/* Delete symbol and value */
		lval_del(sym); lval_del(val);
	}
	
	/* Argument list is now bound so can be cleaned up */
	lval_del(a);
	
	/* If '&' remains in formal list bind to empty list */
	if (f->formals->count > 0 && strcmp(f->formals->cell[0]->sym, "&") == 0) {
		
		/* Check to ensure &* is not passed invalidly */
		if (f->formals->count != 2) {
			return lval_err("Function format invalid. "
				"Symbol '&' not followed by single symbol.");
		}
		
		/* Pop and delete '&' symbol */
		lval_del(lval_pop(f->formals, 0));
		
		/* Pop next symbol and create empty list */
		lval* sym = lval_pop(f->formals, 0);
		lval* val = lval_qexpr();
		
		/*Bind to environment and delete */
		lenv_put(f->env, sym, val);
		lval_del(sym); lval_del(val);
	}
	
	/* If all formals have been bound, evaluate */
	if (f->formals->count == 0) {
		
		/* Set the environment parent to evaluation environment */
		f->env->par = e;
		
		/* Evaluate and return */
		return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
	} else {
		/* Otherwise return partially evaluated function */
		return lval_copy(f);
	}
}

lval* lval_eval_sexpr(lenv* e, lval* v) {
	for(int i = 0; i < v->count; i++) {
		v->cell[i] = lval_eval(e, v->cell[i]);
	}
	
	for (int i = 0; i < v->count; i++) {
		if (v->cell[i]->type == LVAL_ERR) { return lval_take(v, i); }
	}
	
	if (v->count == 0) { return v; }
	if (v->count == 1) { return lval_eval(e, lval_take(v, 0)); }
	
	lval* f = lval_pop(v, 0);
	if (f->type != LVAL_FUN) {
		lval* err = lval_err(
			"S-Expression starts with incorrect type. "
			"Got %s, Expected %s.",
			ltype_name(f->type), ltype_name(LVAL_FUN));
		lval_del(f); lval_del(v);
		return err;
	}
	
	lval* result = lval_call(e, f, v);
	lval_del(f);
	return result;
}

lval* lval_eval(lenv* e, lval* v) {
	if (v->type == LVAL_SYM) {
		lval* x = lenv_get(e, v);
		lval_del(v);
		return x;
	}
	if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
	return v;
}

lval* lval_read_num(mpc_ast_t* t) {
	errno = 0;
	long x = strtol(t->contents, NULL, 10);
	return errno != ERANGE ? lval_num(x) : lval_err("Invalid number.");
}

lval* lval_read(mpc_ast_t* t) {
	if (strstr(t->tag, "number")) { return lval_read_num(t); }
	if (strstr(t->tag, "symbol")) { return lval_sym(t->contents); }
	
	lval* x = NULL;
	if (strcmp(t->tag, ">") == 0) { x = lval_sexpr(); }
	if (strstr(t->tag, "sexpr")) { x = lval_sexpr(); }
	if (strstr(t->tag, "qexpr")) { x = lval_qexpr(); }
	
	for (int i = 0; i < t->children_num; i++) {
		if (strcmp(t->children[i]->contents, "{") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "}") == 0) { continue; }
		if (strcmp(t->children[i]->contents, ")") == 0) { continue; }
		if (strcmp(t->children[i]->contents, "(") == 0) { continue; }
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
		"																	\
			number	:	/-?[0-9]+/ ;										\
			symbol	:	/[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;					\
			sexpr	:	'(' <expr>* ')' ;									\
			qexpr	:	'{' <expr>* '}' ;									\
			expr	:	<number> | <symbol> | <sexpr> | <qexpr> ;			\
			lisp	:	/^/ <expr>* /$/ ;									\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lisp);
		
	puts("Lisp Version 0.0.0.8");
	puts("Press Ctrl+c to Exit\n");
	
	lenv* e = lenv_new();
	lenv_add_builtins(e);
	
	while (1) {
		char* input = readline("Lisp> ");
		add_history(input);
		
		mpc_result_t r;
		if (mpc_parse("<stdin>", input, Lisp, &r)) {
			lval* x = lval_eval(e, lval_read(r.output));
			lval_println(x);
			lval_del(x);
			
			mpc_ast_delete(r.output);
		} else {
			mpc_err_print(r.error);
			mpc_err_delete(r.error);
		}
		free(input);
	}
	
	lenv_del(e);
	mpc_cleanup(6, Number, Symbol, Sexpr, Qexpr, Expr, Lisp);
	
	return 0;
}