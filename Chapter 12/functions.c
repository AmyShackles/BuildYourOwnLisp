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