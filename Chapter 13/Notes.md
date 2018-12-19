### Chapter 13 - Conditionals

We're going to reuse our Number data type for the result of comparisons -> keeping C's rules of 0 representing a false and any other number representing true.

Our ordering functions will only take two arguments and will only work on numbers, returning an `lval` number 0 or 1 depending on the equality comparison between the two `lval`.

We can use one function to do the comparisons:

Steps:

- Check for errors
- Compare the numbers in each of the arguments to get a result
- Return the result as a number

```
lval* builtin_gt(lenv* e, lval* a) {
    return builtin_ord(e, a, ">");
}

lval* builtin_ge(lenv* e, lval* a) {
    return builtin_ord(e, a, ">=");
}

lval* builtin_lt(lenv* e, lval* a) {
    return builtin_ord(e, a, "<");
}

lval* builtin_le(lenv* e, lval* a) {
    return builtin_ord(e, a, "<=");
}

lval* builtin_ord(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    LASSERT_TYPE(op, a, 0, LVAL_NUM);
    LASSERT_TYPE(op, a, 1, LVAL_NUM);

    int r;
    if (strcmp(op, ">") == 0) {
        r = (a->cell[0]->num > a->cell[1]->num);
    }
    if (strcmp(op, "<") == 0) {
        r = (a->cell[0]->num < a->cell[1]->num);
    }
    if (strcmp(op, ">=") == 0) {
        r = (a->cell[0]->num >= a->cell[1]->num);
    }
    if (strcmp(op, "<=") == 0) {
        r = (a->cell[0]->num <= a->cell[1]->num);
    }
    lval_del(a);
    return lval_num(r);
}
```

Now we're going to write a function that checks for equality and should be able to tell if two functions are equivalent, so it should check all fields that make up the data type.

```
int lval_eq(lval* x, lval* y) {
    /* Different types are always unequal */
    if (x->type != y->type) { return 0; }

    /* Compare based on type */
    switch(x->type) {
        /* Compare number values */
        case LVAL_NUM: return (x->num == y->num);
        /* Compare string values */
        case LVAL_ERR: return (strcmp(x->err, y->err) == 0);
        case LVAL_SYM: return (strcmp(x->sym, y->sym) == 0);

        /* If builtin, compare.  Otherwise, compare formals and body */
        case LVAL_FUN:
            if (x->builtin || y->builtin) {
                return x->builtin == y->builtin;
            } else {
                return lval_eq(x->formals, y->formals) && lval_eq(x->body, y->body);
            }

        case LVAL_QEXPR:
        case LVAL_SEXPR:
            if (x->count != y->count) { return 0; }
            for (int i = 0; i < x->count; i++) {
                /* If any element not equal, whole list not equal */
                if (!lval_eq(x->cell[i], y->cell[i])) { return 0; }
            }
            /* Otherwise, lists must be equal */
            return 1;
        break;
    }
    return 0;
}
```

Using this, the new function for equality comparisons is simple to add -> we ensure two arguments are input and equal, store the result into an `lval`, and return it.

```
lval* builtin_cmp(lenv* e, lval* a, char* op) {
    LASSERT_NUM(op, a, 2);
    int r;
    if (strcmp(op, "==") == 0) {
        r = lval_eq(a->cell[0], a->cell[1]);
    }
    if (strcmp(op, "!=") == 0) {
        r = !lval_eq(a->cell[0], a->cell[1]);
    }
    lval_del(a);
    return lval_num(r);
}

lval* builtin_eq(lenv* e, lval* a) {
    return builtin_cmp(e, a, "==");
}

lval* builtin_ne(lenv* e, lval* a) {
    return builtin_cmp(e, a, "!=");
}
```

Now we're going to make an `if` statement that can act like a ternary, where the user can pass in the result of a comparison and two Q-Expressions -> one being the code to execute if the comparison is true and one to execute if the comparison is false.

```
lval* builtin_if(lenv* e, lval* a) {
    LASSERT_NUM("if", a, 3);
    LASSERT_TYPE("if", a, 0, LVAL_NUM);
    LASSERT_TYPE("if", a, 1, LVAL_QEXPR);
    LASSERT_TYPE("if", a, 2, LVAL_QEXPR);

    lval* x;
    a->cell[1]->type = LVAL_SEXPR;
    a->cell[2]->type = LVAL_SEXPR;

    if (a->cell[0]->num) {
        /* If the conditon is true, evaluate first expression */
        x = lval_eval(e, lval_pop(a, 1));
    } else {
        /* Otherwise, evaluate second expression */
        x = lval_eval(e, lval_pop(a, 2));
    }
    /* Delete argument list and return */
    lval_del(a);
    return x;
}
```

REGISTER THE BUILTINS!

```
lenv_add_builtin(e, "if", builtin_if);
lenv_add_builtin(e, "==", builtin_eq);
lenv_add_builtin(e, "!=", builtin_ne);
lenv_add_builtin(e, ">", builtin_lt);
lenv_add_builtin(e, ">=", builtin_le);
lenv_add_builtin(e, "<", builtin_gt);
lenv_add_builtin(e, "<=", builtin_ge);
```
