**Chapter 14 - Strings**

We're going to start adding code to allow our Lisp interpreter to load files. For that to happen, we need a user to be able to provide a file name. While our interpreter supports symbols, it doesn't yet support strings, so we need to add that as another `lval` type!

```
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_STR, LVAL_FUN, LVAL_SEXPR, LVAL_QEXPR };
```

```
struct lval {
  int type;
  long num;
  char* err;
  char* sym;
  char* str;
  lbuiltin builtin;
  lenv* env;
  lval* formals;
  lval* body;
  int count;
  lval** cell;
};
```

Constructor for our new string type:

```
lval* lval_str(char* s) {
    lval* v = malloc(sizeof(lval));
    v->type = LVAL_STR;
    v->str = malloc(strlen(s) + 1);
    strcpy(v->str, s);
    return v;
}
```

Need to add conditions for the new string type on the relevant `lval` functions:

In `lval_del`:

```
case LVAL_STR:
    free(v->str);
    break;
```

In `lval_copy`:

```
case LVAL_STR:
    x->str = malloc(strlen(v->str) + 1);
    strcpy(x->str, v->str);
    break;
```

In `lval_eq`:

```
case LVAL_STR:
    return (strcmp(x->str, y->str) == 0);
```

In `ltype_name`:

```
case LVAL_STR:
    return "String";
```

In order to print strings, we need to add a bit of code because the way that we're currently storing strings are not in a way that a user would want to see them displayed (i.e., there are no escape characters to output it in an easily readable format). We're going to use some of `mpc`'s functionality to help with this:

In lval_print:

```
case LVAL_STR:
    lval_print_str(v);
    break;
```

```
void lval_print_str(lval* v) {
    /* Make a copy of the string */
    char* escaped = malloc(strlen(v->str) + 1);
    strcpy(escaped, v->str);

    /* Pass it through the escape function */
    escaped = mpcf_escape(escaped);

    /* Print it between double quotation mark characters */
    printf("\"%s\"", escaped);

    /* Free the copied string */
    free(escaped);
}
```

We'll define a string as a `"` character, followed by zero or more of either a backslash `\\` followed by any other character `.` or anything that isn't a `"` character, ending with a `"` character.

```
string : /\"(\\\\.|[^\"])*\"/ ;
```

Adding a case in `lval_read`:

```
if (strstr(t->tag, "string")) {
    return lval_read_str(t);
}
```

This is where things get tricky. In order to read the string, we need to take the quotation marks out, unescape it, convert characters to their actual encoded characters, create a new `lval`, and clean everything up at the end:

```
lval* lval_read_str(mpc_ast_t* t) {
    /* Cut off the final quote character */
    t->contents[strlen(t->contents)-1] = '\0';

    /* Copy the string missing out the first quote character */
    char* unescaped = malloc(strlen(t->contents+1)+1);
    strcpy(unescaped, t->contents+1);

    /* Pass through the unescape function */
    unescaped = mpcf_unescape(unescaped);

    /* Construct a new lval using the string */
    lval* str = lval_str(unescaped);

    /* Free the string and return */
    free(unescaped);

    return str;
}
```
Note:  Don't forget to change expr to include `<string>`

In Lisp, comments are defined by a semicolon followed by any number of characters that are not newline characters represented by either \r or \n.

```
comment     : /;[^\\r\\n]*/ ;
```

Because comments are only for programmers reading the code, the internal function for reading them should just ignore them, which is why we add this to `lval_read`:

```
if (strstr(t->children[i]->tag, "comment")) { continue; }
```

Comments won't be much use in the prompt, but they will be helpful for adding into files of code to annotate them.  Which brings us to the load function!

This load function should load and evaluate a file when passed a string of its name.  Because we want to use our grammar in order to read in the file contents, parse, and evaluate them, our load function will rely on `mpc_parser* Lisp`, which will require all our parser pointers to be forward declared.

The load function will check that the input argument is a single string, then use `mpc_parse_contents` to read in the contents of a file using a grammar, parsing the contents of a file into an `mpc_result` that will either be an _abstract syntax tree_ or an _error_.

On successfully parsing a file, we don't treat it like one expression = we let users list multiple expressions and evaluate all of them individually by looping over each expression in the contents of the file and evaluating them one at a time.  If there are errors, we should print them and continue.  If there's a parse error, we're going to extract the message and put it into an error `lval` which we return.  If there are no errors, the return value for the builtin can just be the empty expression.

Full code should look like this:

```
lval* builtin_load(lenv* e, lval* a) {
    LASSERT_NUM("load", a, 1);
    LASSERT_TYPE("load", a, 0, LVAL_STR);

    /* Parse file given by string name */
    mpc_result_t r;
    if (mpc_parse_contents(a->cell[0]->str, Lisp, &r)) {
        /* Read contents */
        lval* expr = lval_read(r.output);
        mpc_ast_delete(r.output);

        /* Evaluate each expression */
        while (expr->count) {
            lval* x = lval_eval(e, lval_pop(expr, 0));
            /* If evaluation loads to error, print it */
            if (x->type == LVAL_ERR) { lval_println(x); }
            lval_del(x);
        }

        /* Delete expressions and arguments */
        lval_del(expr);
        lval_del(a);

        /* Return empty list */
        return lval_sexpr();
    } else {
        /* Get parse error as string */
        char* err_msg = mpc_err_string(r.error);
        mpc_err_delete(r.error);

        /* Create new error message using it */
        lval* err = lval_err("Could not load library %s", err_msg);
        free(err_msg);
        lval_del(a);

        /* Cleanup and return error */
        return err;
    }
}
```

We're going to add some functionality to take command line arguments.  If argc (argument count) is set to 1, we'll invoke the interpreter.  Otherwise, we can run each of the arguments through the builtin_load function.

```
/* Supplied with list of files */

if (argc >= 2) {
    /* Loop over each supplied filename (starting from 1) */
    for (int i = 1; i < argc; i++) {
        /* Argument list with a single argument, the filename */
        lval* args = lval_add(lval_sexpr(), lval_str(argv[i]));

        /* Pass to builtin load and get the result */
        lval* x = builtin_load(e, args);

        /* If the result is an error, print it */
        if (x->type == LVAL_ERR) { lval_println(x); }
        lval_del(x);
    }
}
```

If we're running programs from the command line, we might want them to output data rather than just define functions and their values.  This function prints each argument separated by a space and then prints a newline character to finish.  It returns the empty expression:

```
lval* builtin_print(lenv* e, lval* a) {
    /* Print each argument followed by a space */
    for (int i = 0; i < a->count; i++) {
        lval_print(a->cell[i]); putchar(' ');
    }
    /* Print a newline and delete arguments */
    putchar('\n');
    lval_del(a);

    return lval_sexpr();
}
```

Now that we have strings, we can add in an error reporting function that can take as input a user supplied string and provide it an error message for `lval_err`.

```
lval* builtin_error(lenv* e, lval* a) {
    LASSERT_NUM("error", a, 1);
    LASSERT_TYPE("error", a, 0, LVAL_STR);

    /* Construct error from first argument */
    lval* err = lval_err(a->cell[0]->str);

    /* Delete arguments and return */
    lval_del(a);
    return err;
}
```

Now we just have to register all the new fangled functions!

```
lenv_add_builtin(e, "load", builtin_load);
lenv_add_builtin(e, "error", builtin_error);
lenv_add_builtin(e, "print", builtin_print);
```

