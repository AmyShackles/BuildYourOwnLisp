### What are the four typical steps for adding new language features?

1) Syntax - Add a new rule to the language grammar
2) Representation - add new data type variations to represent the feature
3) Parsing - add new functions for reading the feature from the abstract syntax tree
4) Semantics - add new functions for evaluating and manipulating the feature

### Create a Macro specifically for testing for the incorrect number of arguments 
	#define INCORRECTARGNUM(args, count, err) \
		if (count != 1) { lval_del(args); return lval_err(err); }
		
### Create a Macro specifically for testing for being called with an empty list.
	#define EMPTYLIST(args, count, err) \
		if (count == 0) { lval_del(args); return lval_err(err); }
		
### Add a builtin function cons that takes a value and a Q-Expression and appends it to the front.
	- Could not figure this one out either.

### Add a builtin function len that returns the number of elements in a Q-expression.
	- Could not figure out for the life of me how to implement this.


### Add a builtin function init that returns all of a Q-expression except the final element.
	lval* builtin_init(lval* a) {
		INCORRECTARGNUM(a, a->count, "Function 'init' passed too many arguments!");
		LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type!");
	
		lval* x = lval_take(a, 0);
		lval_del(lval_pop(x, x->count - 1));
		return x;
	}

-- Accidentally added a 'last' function that returns the last element in a Q-expression.
	lval* builtin_last(lval* a) {
		INCORRECTARGNUM(a, a->count, "Function 'last' passed too many arguments!");
		LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'init' passed incorrect type!");
	
		lval* x = lval_take(a, a->count - 1);
		while (x->count > 1) {
			lval_del(lval_pop(x, 0));
		}
	
		return x;
	}