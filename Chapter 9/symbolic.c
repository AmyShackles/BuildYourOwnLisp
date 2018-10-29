enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR }

typedef struct lval {
	int type;
	long num;
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


mpc_parser_t* Number = mpc_new("number");
mpc_parser_t* Symbol = mpc_new("symbol");
mpc_parser_t* Sexpr = mpc_new("sexpr");
mpc_parser_t* Expr = mpc_new("expr");
mpc_parser_t* Lisp = mpc_new("lisp");

mpca_lang(MPCA_LANG_DEFAULT,
	"														\
		number	: /-?[0-9]+/ ;								\
		symbol	: '+' | '-' | '*' | '/' | '^' | '%' ;		\
		sexpr	: '(' <expr>* ')' ;							\
		expr	: <number> | <symbol> | <sexpr> ;			\
		lisp	: /^/ <expr>* /$/ ;							\
	",
	Number, Symbol, Sexpr, Expr, Lisp);
	
	
void lval_del(lval* v) {
	switch (v->type) {
		/* Do nothing special for number types */
		case LVAL_NUM: break;
		
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
	mpc_cleanup(5, Number, Symbol, Sexpr, Expr, Lisp);