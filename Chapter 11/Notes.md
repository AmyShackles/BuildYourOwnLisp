**Variables**

Variables are a way that we can assign names to values and copy them for later use.

In order for us to name values, we need to create a structure that stores the name and value of everything named in our program. We'll call it an `envrionment`. When we start a new prompt, we want to create a new environment to go with it in which each bit of input is evaluated and then store and recall variables as we program.

We can re-assign a name, but it mean deleting the old association and creating a new one.

**Symbol Syntax**

In order to allow for user-defined variables, we need to update the grammar for symbols to be more flexible.

`/[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/`

Because it's part of a C string, need to use two backslashes in order to escape. This rule lets symobls be any of the normal C identifier characters (a-zA-Z0-9\_), the arithmetic operators '+', '-', '\*', '/', the backslash character, and comparison operators '=', '<', '>', and '!' or ampersands ('&').

```
mpca_lang(MPCA_LANG_DEFAULT,
    "                                                       \
        number: /-?[0-9]+/;                                 \
        symbol: /[a-zA-Z0-9_+\\-*\\/\\\\=<>!&]+/;           \
        sexpr:  '(' <expr>* ')';                            \
        qexpr:  '{' <expr>* '}';                            \
        expr:   <number> | <symbol> | <sexpr> | <qexpr>;    \
        lisp:   /^/ <expr>* /$/;                            \
    ",
    Number, Symbol, Sexpr, Qexpr, Expr, Lisp);
```

Now that variables exist, symbols can no longer represent functions in our language (because symbols can represent either functions or values). Symbols are now names for us to look up in the environment and return a value for. In order to differentiate functions from non-function values, we'll need a new type of `lval` that uses a function pointer.

Function pointers are a way to store and pass around functions and are a way to call the function they poiunt to as if oit were a normal function.

The type associated with a function pointer specifies the type of the function it points to, not the type of the data pointed to.

Previously, our builtin functions took in an `lval*` as input and returned an `lval*` as output, but now we want to add a pointer for the environment `lenv*` as input.

```
typedef lval*(*lbuiltin)(lenv*, lval*)
```

`typedef` in this situation is used to name a variable, declaring it a new type, matching what would be the inferred type of the variable.

"Pointer types in C are actually meant to be written with the star `*` on the left hand side of the variable name, not the right hand side of the type - this is because C type syntax works by a kind of inference. Instead of reading '_Create a new `int` pointer `x`_', it is meant to read '_Create a new variable `x` where to dereference `x` results in an `int`._' Therefore, x is inferred to be a pointer to an int. This idea is extended to function pointers. We can read the above declaration as follows: '_To get an lval* we dereference `lbuiltin` and call it with a `lenv*`and a`lval\*`._' Therefore, lbuiltin must be a function pointer that takes an `lenv*` and a `lval*` and returns a `lval*`"

Because `lbuiltin` references `lval` and `lenv` types and we want to add an `lbuiltin` field in our `lval` struct, we need to use forward declarations as we have a cyclic type dependency.

In order to do forward declarations of types, you create struct types without bodies and then typedef them to the names we will use for them:

```
struct lval;
struct lenv;
typedef struct lval lval;
typedef struct lenv lenv;

enum { LVAL_ERR, LVAL_NUM, LVAL_SYM,
       LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };

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
```

Now that we have a new `lval` type for function, we need to change other functions in our program to work with the new `lval` type, which also means creating a new constructor for this new type.

```
lval* lval_fun(lbuiltin func) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_FUN;
    v->fun = func;
    return v;
}
```

We don't need to do anything special with deletion for function pointers, so the case of LVAL_FUN can just be to break.

On printing, we can print out a nominal string:

```
case LVAL_FUN: printf("<function>"); break;
```

We do, however, also need a function that can copy an lval in order to put things into and take them out of the environment. For numbers and functions, we can copy the fields directly, for strings we need to copy using malloc and strcpy. To copy lists, we need to allocate the right amount of space and copy each element individually.

```
lval* lval_copy(lval* v) {
    lval* x = malloc(sizeof(lval));
    x->type = v->type;

    switch(v->type) {
        case LVAL_FUN: x->fun = v->fun; break;
        case LVAL_NUM: x->num = v->num; break;
        case LVAL_ERR:
            x->err = malloc(strlen(v->err) + 1);
            strcpy(x->err, v->err); break;
        case LVAL_SYM:
            x->sym = malloc(strlen(v->sym) + 1);
            strcpy(x->sym, v->sym); break;
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
```

**Environment**

The environment structure needs to keep track of the relationship between names and values. In order to do this in our Lisp, we're going to have two different lists of equal length that will have corresponding entries to each other at the same position.

```
struct lenv {
    int count;
    char** syms;
    lval** vals;
};
```

We also need functions to create and delete this structure:

```
lenv* lenv_new(void) {
    lenv* e = malloc(sizeof(lenv));
    e->count = 0;
    e->syms = NULL;
    e->vals = NULL;
    return e;
}
```

```
void lenv_del(lenv* e) {
    for (int i = 0; i < e->count; i++) {
        free(e->syms[i]);
        lval_del(e->vals[i]);
    }
    free(e->syms);
    free(e->vals);
    free(e);
}
```

In order to get a value from the environment, we're going to loop through the items in the environment and check if the symbol matches anything stored. If we find a match, we'll return a copy of the stored value. Otherwise, we'll return an error.

```
lval* lenv_get(lenv* e, lval* k) {
    for (int i = 0; i < e->count; i++) {
        if (strcmp(e->syms[i], k->sym) == 0) {
            return lval_copy(e->vals[i]);
        }
    }
    return lval_err("unbound symbol");
}
```

In order to add a variable to the environment, we're going to loop through all the items in the environment and if there is a name match, we'll delete the value stored at that location and store a copy of the input value. If no existing value is found with the name, we'll allocate space using realloc and store a copy of the lval and its name at the newly allocated locations.

```
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
    e->sym = realloc(e->syms, sizeof(char*) * e->count);

    e->vals[e->count-1] = lval_copy(v);
    e->syms[e->count-1] = malloc(strlen(k->sym)+1);
    strcpy(e->syms[e->count-1], k->sym);
}
```

**Variable Evaluation**

Evaluation now depends on the environment, so we need to pass the environment as an argument and use it to get a value if there is a symbol type.

```
lval* lval_eval(lenv* e, lval* v) {
    if (v->type == LVAL_SYM) {
        lval* x = lenv_get(e, v);
        lval_del(v);
        return x;
    }
    if (v->type == LVAL_SEXPR) { return lval_eval_sexpr(e, v); }
    return v;
}
```

Now that we have a function type, our evaluation of S-Expressions has to change so that it checks for a function type rather than a symbol type. If it is a function type, we can call the fun field of the `lval` like we would a standard function call.

```
lval* lval_eval_sexpr(lenv* e, lval* v) {
    for (int i = 0; i < v->count; i++) {
        v->cell[i] = lval_eval(e, v->cell[i]);
    }
    for (int i = 0; i < v->count; i++) {
        if (v->cell[i]->type == LVAL_ERR) {
            return lval_take(v, i);
        }
    }

    if (v->count == 0) { return v; }
    if (v->count == 1) { return lval_take(v, 0); }

    /* Ensure first element is a function after evaluation */
    lval* f = lval_pop(v, 0);
    if (f->type != LVAL_FUN) {
        lval_del(v); lval_del(f);
        return lval_err("First element is not a function");
    }

    /* If it is, call function to get result */
    lval* result = f->fun(e, v);
    lval_del(f);
    return result;
}
```

We need to modify our builtin functions so that they can take in an environment as well as an `lval` in order to return an `lval`.

For example:

```
lval* builtin_add(lenv* e, lval* a) {
    return builtin_op(e, a, '+');
}

lval* builtin_sub(lenv* e, lval* a) {
    return builtin_op(e, a, '-');
}

lval* builtin_mul(lenv* e, lval* a) {
    return builtin_op(e, a, '*');
}

lval* builtin_div(lenv* e, lval* a) {
    return builtin_op(e,a, '/');
}
```

We now need to create a function `lval` and a symbol `lval` with the name of each builtin and register them with the environment with `lenv_put`.

```
void lenv_add_builtin(lenv* e, char* name, lbuiltin func) {
    lval* k = lval_sym(name);
    lval* v = lval_fun(func);
    lenv_put(e, k, v);
    lval_del(k); lval_del(v);
}

void lenv_add_builtins(lenv* e) {
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
```

To make this all work, we need to call the function that adds the builtins to the environment before creating the interactive prompt and we need to delete the environment after we're finished using it.

```
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
```

**Define Function**

Now we have to allow for users to be able to define their own variables. In order to allow for a user to pass in a symbol without it being evaluated, symbols must be wrapped in {} (as Q-Expressions aren't evaluated).

Our define function will take in a list of numbers and number of other values and assign each of the values to each of the symbols. It will check that the arguments are the correct types, then iterate over each symbol and value and put them in the environment. If there is an error, it will be returned, but if there is no error, an empty expression `()` will be returned.

```
lval* builtin_def(lenv* e, lval* a) {
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'def' passed incorrect type!");

    lval* syms = a->cell[0];

    for (int i = 0; i < syms->count; i++) {
        LASSERT(a, syms->cell[i]->type == LVAL_SYM, "Function 'def' cannot define non-symbol");
    }

    LASSERT(a, syms->count == a->count - 1, "Function 'def' cannot define incorrect number of values to symbols");

    for (int i = 0; i < syms->count; i++) {
        lenv_put(e, syms->cell[i], a->cell[i+1]);
    }

    lval_del(a);
    return lval_sexpr();
}
```

**Error Reporting**

We should be able to tell the user what symbol they typed that threw an error in order to help them track down errors, etypos, etc. We can do this by creating `variable argument` functions, which can take a variable number of arguments.

We will change `lval_err` to act like `printf`, taking a format string and a list of arguments to match into the string.

In order to declare that a function takes variable arguments, you use the special syntax of ellipses `...` which represent the rest of the arguments.

```
lval* lval_err(char* fmt, ...)
```

Now we need to examine what's been passed in!

(Fun fact, functions that can take a variable type or number of arguments can be sometimes called `variadic` functions)

We're going to be using some functions that can be found in `stdarg.h`, but we don't have to include stdarg.h to our program because `mph.h` already has them included.

Our function needs to declare an object of type `va_list` that the macros `va_start()`, `va_arg()` and `va_end()` use.

`va_start` initializes the va_list for use by `va_arg` and `va_end`. `va_start` should be passed the `va_list` and the name of the last argument before the varialbe argument list (last argument where type is known).

It's possible then to examine each variable using `va_arg`, but for our Lisp interpreter, we're going to pass the entire argument list to `vsnprintf`, which takes in a `va_list` and writes it to a string. Since `vsnprintf` outputs to a string, we need to first allocate space for it. Because we don't know the size of the string until after the function is run, we allocate a large buffer and then reallocate to a smaller buffer once we know the size of the error message.

```
lval* lval_err(char* fmt, ...) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_ERR;

    va_list va;
    va_start(va, fmt);

    v->err = malloc(512);

    vsnprintf(v->err, 511, fmt, va);

    v->err = realloc(v->err, strlen(v->err)+1);

    va_end(va);

    return v;
}

```

Now we can add better error messages to our functions, for example, in `lenv_get`, we can use:

```
return lval_err("Unbound symbol '%s'", k->sym);
```

We can also change LASSERT to take variable arguments. Because it's a macro and not a standard function, the syntax is slightly different than the one we used for `lval_err`. We use ellipses in the argument list again, but in the body of the function, we use `##__VA_ARGS__` to paste the contents of the arguments. The hash signs remove the leading comma if the macro is passed no extra arguments. Because we might use args in the making of the error message, we have to remember not to delete it until we've created the value for error.

```
#define LASSERT(args, cond, fmt, ...) \
    if (!(cond)) { \
        lval* err = lval_err(fmt, ##__VA_ARGS__); \
        lval_del (args); \
        return err; \
    }
```

Now we can update error messages!:

```
LASSERT(a, a->count == 1,
    "Function 'head' passed too many arguments. "
    "Got %i, expected %i.",
    a->count, 1);
```

We can also add a function that will return the name of the type of LVAL was passed in in order to improve error messages!

```
char* ltype_name(int t) {
    switch(t) {
        case LVAL_FUN: return "Function";
        case LVAL_NUM: return "Number";
        case LVAL_ERR: return "Error";
        case LVAL_SYM: return "Symbol";
        case LVAL_SEXPR: return "S-Expression";
        case LVAL_QEXPR: return "Q-Expression";
        default: return "Unknown type";
    }
}
```

```
LASSERT(a, a->cell[0]->type == LVAL_QEXPR,
    "Function 'head' passed incorrect type for argument 0. "
    "Got %s, expected %s",
    ltype_name(a->cell[0]->type), ltype_name(LVAL_QEXPR));
```
