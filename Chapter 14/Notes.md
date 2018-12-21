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
