In order to create better error handling, we introduce a new struct with fields that can represent either a number or an error, the `lval` or Lisp Value struct:

```
typedef struct {
    int type;
    long num;
    int err;
} lval;
```

Though we use `int` for type, we're actually going to be employing an enum in order to keep the code more readable.

An **enum** is a declaration of variables which under the hood are automatically assigned integer constant values.

Enum for lval types:

```
enum { LVAL_NUM, LVAL_ERR};
```

Enum for possible error types:

```
enum { LERR_DIV_ZERO, LERR_BAD_OP, LERR_BAD_NUM };
```

Now we need to create functions that can create the two different lval types:

```
lval lval_num(long x) {
    lval v;
    v.type = LVAL_NUM;
    v.num = x;
    return v;
}

lval lval_err(int x) {
    lval v;
    v.type = LVAL_ERR;
    v.err = x;
    return v;
}
```

As the lval can be either a number or an error, we can't use a simple printf statement. Instead, we must construct a function that will determine which needs to be printed and perform the appropriate action.

```
void lval_print(lval v) {
    switch(v.type) {
        case LVAL_NUM: printf("%li, v.num); break;
        case LVAL_ERR:
            if (v.err == LERR_DIV_ZERO) {
                printf("Error: Division by zero");
            }
            if (v.err == LERR_BAD_OP) {
                printf("Error: Invalid operator");
            }
            if (v.err == LERR_BAD_NUM) {
                printf("Error: Invalid number");
            }
            break;
    }
}

void lval_println(lval v) { lval_print(v); putchar('\n'); }
```

lval_println calls lval_print and then adds a newline to the end of the printing of lval.

We then need to modify our `eval_op` code to be operating on an lval rather than a long and to return the calling of `lval_err` on error condition.

**Ternary**

\<condition> ? \<then> : \<else>

We then need to change the `eval` function to check errno to see if the conversion has gone the way that we'd like it to.
