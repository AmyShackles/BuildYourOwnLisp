
/* Forward Declarations */

struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

/* Lisp Value */

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR }

// To get an lval*, we dereference lbuiltin and call it with a lenv* and a lval*
// lbuiltin must be a function pointer that takes an lenv* and a lval* and returns a lval*		
typedef lval*(*lbuiltin)(lenv*, lval*);

struct lval {
	int type;
	long num;
	char* err;
	char* sym;
	lbuiltin fun;
	int count;
	lval** cell;
};

lval* lval_fun(lbuiltin func) {
	lval* v = malloc(sizeof(lval));
	v->type = LVAL_FUN;
	v->fun = func;
	return v;
}

lval* lval_copy(lval* v) {
	lval* x = malloc(sizeof(lval));
	x->type = v->type;
	
	switch (v->type) {
	
		/* Copy functions and numbers directly */
		case LVAL_FUN: x->fun = v->fun; break; 
		case LVAL_NUM: x->num = v->num; break;
		
		/* Copy strings using malloc and strcpy */
		case LVAL_ERR:
			x->err = malloc(strlen(v->err) + 1);
			strcpy(x->err, v->err); break;
			
		case LVAL_SYM:
			x->sym = malloc(strlen(v->sym)+1);
			strcpy(x->sym, v->sym); break;
			
		/* Copy lists by copying each sub-expression */
		
	}
}


	mpca_lang(MPCA_LANG_DEFAULT,
		"														\
			number : /-?[0-9]+/ ;								\
			symbol : /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/ ;			\
			sexpr  : '(' <expr>* ')' ;							\
			qexpr  : '{' <expr>* '}' ;							\
			expr   : <number> | <symbol> | <sexpr> | <qexpr> ;  \
			lisp   : /^/ <expr>* /$/ ;							\
		",
		Number, Symbol, Sexpr, Qexpr, Expr, Lisp);

