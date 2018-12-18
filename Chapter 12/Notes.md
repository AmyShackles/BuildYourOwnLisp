**Functions**

One way to think about functions is that they're a description of computation to be used later on and defining a function is like saying that you want `x` set of actions to take place when you use the name `y`. They're defined once and can be called on however many times you'd like to call them after that.

Another way to think about them is a black box that takes input and produces output.

A third way of thinking about functions is as partial computations - that they take a given set of input and that it can't be run until provided those values. These inputs are called `unbound variables` and in order to finish the computation, you supply those variables to the function. Output from a partial computation is itself a variable with an unknown value that can be placed asz input to another function.

`Lambda calculus` is the study of functions and is a field that combines logic, maths, and computer science. Lambda is a Greek letter used to represent the binding of variables.

Four our Lisp, we'll have user defined functions take two arguments, the first being the list of formal arguments and the second being a list of operations. We'll use the '\' symbol as the name for this kind of function as it's a bit like a Lambda and easier to type.

When running the function, it's going to be evaluated with the builtin `eval` function.

Example input:

```
\ {x y} {+ x y}
```

You can then call it by using it as the first argument in an S-Expression:

```
(\ {x y} {+ x y}) 10 20
```

To name the user-defined function, you can pass it to the existing builtin `def`:

```
def {add-together} (\ {x y} {+ x y})
```

And then call it by referring to its name:

```
add-together 10 20
```

To store a function as an `lval`, we have to think of what it consists of (3 parts): - List of formal arguments which have to be bound before evaluation - A Q-Expression representing the body of the funciton - A location to store the values assigned to the formal arguments (we have the structure to store these values in the environment)

We'll store builtin and user-defined functions in type LVAL_FUN, so we need a way to differentiate them. We can do this by checking f the lbuiltin function pointer is NULL. If it's not NULL, we know it's a builtin function.

```
struct lval {
    int type;
    long num;
    char* err;
    char* sym;

    lbuiltin builtin;
    lenv* env;
    lval* formals;
    lval* body;

    int count;
    lval** cell;
};
```

We've changed the name of lbuiltin from fun to builtin, so we'll need to change the portions of our code that reference `fun`. We also need to create a constructor for user-defined functions where we build a new environment and assign the `formals` and `body` values to those passed in.

```
lval* lval_lambda(lval* formals, lval* body) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->builtin = NULL;
    v->env = lenv_new();
    v->formals = formals;
    v->body = body;
    return v;
}
```

Because we've changed `lval`, we need to change the functions that delete, copy, and print `lval`s in order to deal with the change.

For deletion:

```
case LVAL_FUN:
    if (!v->builtin) {
        lenv_del(v->env);
        lval_del(v->formals);
        lval_del(v->body);
    }
break;
```

For copying:

```
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
```

For printing:

```
case LVAL_FUN:
    if (v->builtin) {
        printf("<builtin>");
    } else {
        printf("\\"); lval_print(v->formals);
        putchar(' '); lval_print(v->body); putchar(')');
    }
break;
```

Adding the Lambda function:

```
lval* builtin_lambda(lenv* e, lval* a) {
    LASSERT_NUM("\\", a, 2);
    LASSERT_TYPE("\\", a, 0, LVAL_QEXPR);
    LASSERT_TYPE("\\", a, 1, LVAL_QEXPR);

    for (int i = 0; i < a->cell[0]->count; i++) {
        LASSERT(a, (a->cell[0]->cell[i]->type == LVAL_SYM),
        "Cannot define non-symbol.  Got %s, expected %s.",
        ltype_name(a->cell[0]->cell[i]->type), ltype_name(LVAL_SYM));
    }

    lval* formals = lval_pop(a, 0);
    lval* body = lval_pop(a, 0);
    lval_del(a);

    return lval_lambda(formals, body);
}
```

Register with other builtins:

```
lenv_add_builtin(e, "\\", builtin_lambda);
```

In order for user-defined functions to have access to other builtins and globals, we need to update the struct for the environment to include a reference to the parent environment that doesn't get deleted when lenv gets deleted and doesn't get copied when lenv gets copied -> if you call lenv_get on the environment and the symbol isn't found, it will look to any parent environment to see if the symbol was defined elsewhere.

To signify an environment has no parent, use NULL.

```
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
```

In order for lenv_get to search the parent environment, we need to add to it:

```
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
```

Now that we have an lval type with its own environment, we need a way to copy environments for when we copy lval structs:

```
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
```

Now that we have parent environments, the way we define variables must also change - either we define within the local environment or we use the parent environment to define the variable in the global environment. `lenv_put` can remain the same and be used for locals, but we need a new function to go up the parent chain before using `lenv_put` to define locally.

```
void lenv_def(lenv* e, lval* k, lval* v) {
    while (e->par) { e = e->par; }
    lenv_put(e, k, v);
}
```

We will use this later on to write partial results of calculations to local variables inside a function. We should add a builtin for local assignment (we'll call this `put` in C, but `=` symbol in Lisp). Register these.

```
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
        "Got %s, expected %s.", func,
        ltype_name(syms->cell[i]->type),
        ltype_name(LVAL_SYM));
    }

    LASSERT(a, (syms->count == a->count-1),
    "Function '%s' passed too many arguments for symbols. "
    "Got %i, expected %i.", func, syms->count, a->count-1);

    for (int i = 0; i < syms->count; i++) {
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
```

Need to write code for when expression gets evaluated and lval is caled. For builtins, process stays more or less the same. For user-defined functions, we need to bind each of the arguments passed in to each of the symbols in the formals field and then evaluate the body using the env field as an environment and calling environment as a parent.

Might look like this:

```
lval* lval_call(lenv* e, lval* f, lval* a) {
    /* If a builtin, call it */
    if (f->builtin) { return f->builtin(e, a); }

    /* Assign each argument to each formal in order */
    for (int i = 0; i < a->count; i++) {
        lenv_put(f->env, f->formals->cell[i], a->cell[i]);
    }

    lval_del(a);

    /* Set the parent environment */
    f->env->par = e;

    /* Evaluate the body */
    return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
}
```

^^ If the number of arguments and the number of symbols supplied differ, this program will crash.

Could add error handling so that if the number of arguments is incorrect, an error message is returned. Instead, we'll do something cooler! If too few arguments are supplied, we'll bind the first few formal arguments and return it, leaving the rest unbound -> creates a partially evaluated function!

Function takes argument after consuming the input to its right. If it requires no further input, it evaluates and replaces itself with the new value. If it requires new/more input, it is replaced by a more complete function with one of its variables bound. This repeats until the final value for the program is created.

```
lval* lval_call(lenv* e, lval* f, lval* a) {
    /* If builtin, simply apply that */
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
                "Got %i, expected %i.", given, total);
        }

        /* Pop the first symbol from the formals */
        lval* sym = lval_pop(f->formals, 0);

        /* Pop the next argument from the list */
        lval* val = lval_pop(a, 0);

        /* Bind a copy into the function's environment */
        lenv_put(f->env, sym, val);

        /* Delete the symbol and value */
        lval_del(sym); lval_del(val);
    }

    /* Argument list is now bound so can be cleaned up */
    lval_del(a);

    /* If all formals have been bound, evaluate */
    if (f->formals->count == 0) {
        /* Set environment parent to evaluation environment */
        f->env->par = e;

        /* Evaluate and return */
        return builtin_eval(f->env, lval_add(lval_sexpr(), lval_copy(f->body)));
    } else {
        /* Otherwise, return partially evaluated function */
        return lval_copy(f);
    }
}
```

In the earlier version of our interpreter, `lval* result` was the result of calling the function on the current `lval` directly. Now that we have the `lval_call` function that checks whether or not the function is a builtin and either calls it directly if it is or pieces the function together based on user input before calling it if it isn't, `lval* result` is going to instead be a call to `lval_call`, passing in the environment, the formals list, and the argument list.

```
lval* f = lval_pop(v, 0);
if (f->type != LVAL_FUN) {
    lval* err = lval_err(
        "S-Expression starts with incorrect type. "
        "Got %s, expected %s.",
        ltype_name(f->type), ltype_name(LVAL_FUN));
    lval_del(f); lval_del(v);
    return err;
}

lval* result = lval_call(e, f, v);
```

Example for how to test Lisp:

```
Lisp> def {add-mul} (\ {x y} {+ x {* x y}})
()
Lisp> add-mul 10 20
210
Lisp> add-mul 10
(\ {y} {+ x {* x y }})
Lisp> def {add-mul-ten} (add-mul 10)
()
Lisp> add-mul-ten 50
510
Lisp>
```

In order to allow greater flexilibilty in user-defined function definitions, we're going to add functionality where a user can define variadic functions. There isn't really an easy way to accomplish this, so we'll hard-code a system into our language using the `&` symbol.

Users will then be able to define formal arguments that look like `{x & xs}`, which means that the function will take in a single argument `x` followed by zero or more arguments joined together into a list called `xs` -> a bit like the ellipses we used to declare variable arguments in C.

When assigning our formal arguments, then, we'll look for the `&` symbol and if we find it, take the next formal argument and assign it any arguments remaining that were passed in. We'll have to convert the argument list into a Q-Expression, so we'll need to check that the `&` is followed by a real symbol and throw an error if not -> we'll add this case after the first symbol is popped off the formals list in the while loop of `lval_call`.

```
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
    lval_del(sym); lval_del(nysm);
    break;
}
```

If the user doesn't supply any variable arguments after the first named ones, we'll set the symbol following the '&' to an empty list. We'll add code for this scenario after the code to delete the argument list and before the check to see if all formals have been evaluated.

```
/* If '&' remains in formal list, bind to empty list */
if (f->formals->count > 0 &&
    strcmp(f->formals->cell[0]->sym, "&") == 0) {
    /* Check to ensure that & is not passed invalidly */
    if (f->formals->count != 2) {
        return lval_err("Function format invalid. "
            "Symbol '&' not followed by single symbol.");
    }

    /* Pop and delete '&' symbol */
    lval_del(lval_pop(f->formals, 0));

    /* Pop next symbol and create an empty list */
    lval* sym = lval_pop(f->formals, 0);
    lval* val = lval_qexpr();

    /* Bind to environment and delete */
    lenv_put(f->env, sym, val);
    lval_del(sym); lval_del(val);
}
```

Lambdas are a simple and powerful way of defining functions, but the syntax leaves much to be desired. However, we can now write functions that define functions in much simpler syntax!

User can supply a name for the function and the formal arguments in one list and we can separate those out and use them in the definition.

Example:

```
\ {args body} {def (head args) {\ (tail args) body )}
```

We can name the function by passing it to `def` like normal:

```
def {fun} (\ {args body} {def (head args) (\ (tail args) body)})
```

With this in place, the function `add-together` we defined earlie could be defined like so:

```
fun {add-together x y} {+ x y}
```

**Currying**

At this point, we have some functions that can take a variable number of arguments, but we can't pass lists of arguments to them. We can create a function to solve this problem. In the situation of "+", we could append the function to the front of the list and perform evaluation.

`unpack` will take as input some function and some list and append the function to the front of the list before evaluating it.

```
fun {unpack f xs}{ eval (join (list f) xs)}
```

In other situations, we might have the opposite problem -> a function that takes a list as input, but we want to call it using variable arguments. We can make use of the fact that "&" puts variable arguments into a list for us:

```
fun {pack f & xs} {f xs}
```

In some languages, this is called `currying` and `uncurrying`, named after Haskell Curry.

```
Lisp> def {uncurry} pack
()
Lisp> def {curry} unpack
()
Lisp> curry + {5 6 7}
18
Lisp> uncurry head 5 6 7
(5)
```

Because of the way our partial evaluation works, we don't need to think of currying with a specific set of arguments -> we can think of functions themselves as being in either curried or uncurried form.
