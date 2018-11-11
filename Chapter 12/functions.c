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