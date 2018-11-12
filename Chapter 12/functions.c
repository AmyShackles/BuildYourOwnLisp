/* Functions should consist of three parts:
1) List of formal arguments which need to be bound before evaluation
2) A Q-Expression that represents the body of the function
3) The location to store the values assigned to the formal arguments (environment)

-- We need to store builtin functions and user-defined functions under the same type, 
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

/* Need to create a constructor for user defined lval functions */

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

/* Need to update functions for deletion, copying, and printing lvals because lvals changed */

// For deleting:
case LVAL_FUN:
	if (!v->builtin) {
		lenv_del(v->env);
		lval_del(v->formals);
		lval_del(v->body);
	}
	break;

// For copying:
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
		
// For printing:
case LVAL_FUN:
	if (v->builtin) {
		printf("<builtin>");
	} else {
		printf("(\\ "); lval_print(v->formals);
		putchar(' '); lval_print(v->body); putchar(')');
	}
	break;
	}
	
/* Need to create a builtin for lambda function: 
Should take as input a list of symbols and a list that represents the code and then return a function lval.
Do error checking to make sure the count and type of arguments are correct and then if so:
Pop the first two arguments from the list and pass them to previously defined function lval_lambda */

lval* builtin_lambda(lenv* e, lval* a) {
	/* Check two arguments, each of which are Q-expressions */
	LASSERT_NUM("\\", a, 1);
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
	lval* body - lval_pop(a, 0);
	lval_del(a);
	
	return lval_lambda(formals, body);
}

/* Right now, functions have their own environment, but they don't have access to things
outside of their environment (the global environment).
We can give our functions access to the global environment by changing the environment struct
to contain a reference to a parent environment.
That way, when lenv_get is called on the environment, and a symbol isn't found,
it can then search any parent environment for the symbol. */

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

lval* lenv_get(lenv* e, lval* k) {
	for (int i = 0; i < e->count; i++) {
		if (strcmp(e->syms[i], k->sym) == 0) {
			return lval_copy(e->vals[i]);
		}
	}
	
	if (e->par) {
		return lenv_get(e->par, k);
	} else {
		return lval_err("Unbound symbol '%s'", k->sym);
	}
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

/* Should add another builtin for local assignments - put in C, but = in Lisp */

lenv_add_builtin(e, "def", builtin_def);
lenv_add_builtin(e, "=", builtin_put);

lval* builtin_def(lenv* e, lval* a) {
	return builtin_var(e, a, "def");
}

lval* builtin_put(lenv* e, lval* a) {
	return builtin_var(e, a, "=");
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
			lval_del(a); return lval_err(
				"Function passed too many arguments. "
				"Got %i, Expected %i.", given, total);
		}
		
		/* Pop the first symbol from the formals */
		lval* sym = lval_pop(f->formals, 0);
		
		/* Pop the next argument from the list */
		lval* val = lval_pop(a, 0);
		
		/* Bind a copy into the function's environment */
		lenv_put(f->env, sym, val);
		
		/* Delete symbol and value */
		lval_del(sym); lval_del(val);
	}
	
	/* Argument list is now bound so can be cleaned up */
	lval_del(a);
	
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