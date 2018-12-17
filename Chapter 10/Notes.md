**Steps to adding features**:

- Syntax - add new rule to the language grammar

- Representation - add new data type variation to represent this feature

- Parsing - add new functions for reading this feature from the abstract syntax tree

- Semantics - add new functions for evaluating and manipulating this feature

**Q-Expressions (Quoted Expressions)** - a type of Lisp expression that isn't evaluated by the standard Lisp mechanics and is instead left exactly as it is. They can be used to store and manipulate other Lisp values (numbers, symbols, S-Expressions...).

Syntax for Q-Expressions are similar to S-Expressions, the only difference is that they utilize curly brackets `{}` rather than parentheses `()`.

Q-Expressions don't exists in other Lisps because other Lisps use Macros to stop evaluation (look like functions but don't eval arguments). A special Macro called quote `'` exists which can be used to stop evaluation of almost anything and is the inspiration for Q-Expressions in this Lisp.

Because Q-Expressions are similar to S-Expressions, the internal behavior is going to be much the same - the data fields on `lval_qexpr`, for example, can be reused, with the exception of type.

Printing and deleting of Q-Expressions follows the pattern of S-Expressions.

Need to add a special case for constructing an empty Q-Expression to `lval_read` below where we create an empty S-Expression.

Need to update `lval_read` to recognize curly bracket characters

Because Q-Expressions do not have special evaluation methods, no editing of evaluation functions needs to take place in order to implement Q-Expressions.

**Builtin Functions to Manipulate Q-Expressions**

`list` = takes one or more arguments and returns a new Q-Expression containing the arguments

`head` - takes a Q-Expression and returns a Q-Expression with only the first element

`tail` - takes a Q-Expression and returns a Q-Expression with the first element removed

`join` - takes one or more Q-Expressions and returns a Q-Expression of them joined together

`eval` - takes a Q-Expression and evaluates it as though it were an S-Expression

Our builtin functions should have the same interface as builtin_op -> the arguments should be bundled into an S-Expression which the function must use and then delete. Should return a new `lval` as the result of an evaluation.

For `head` and `tail`, we can utilize lval_take and lval_pop, but we need to check that the input is valid first (that they are only passed a single argument and that that argument is a Q-Expression and that the Q-Expression isn't empty).

`head` function can repeatedly pop and delete the item at index `i` until there is nothing in the list

`tail` can pop and delete the item at index `0`, leaving the tail remaining

```
lval* builtin_head(lval* a) {
    if (a->count != 1) {
        lval_del(a);
        return lval_err("Function 'head' passed too many arguments!");
    }

    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'head' passed incorrect types!");
    }

    if (a->cell[0]->count == 0) {
        lval_del(a);
        return lval_err("Function 'head passed {}!");
    }

    lval* v = lval_take(a, 0);

    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}
```

```
lval* builtin_tail(lval* a) {
    if (a->count != 1) {
        lval_del(a);
        return lval_err("Function 'tail' passed too many arguments!");
    }

    if (a->cell[0]->type != LVAL_QEXPR) {
        lval_del(a);
        return lval_err("Function 'tail' passed incorrect types!");
    }

    if (a->cell[0]->count == 0) {
        lval_del(a);
        return lval_err("Function 'tail' passed {}!");
    }

    lval* v = lval_take(a, 0);

    lval_del(lval_pop(v, 0));
    return v;
}
```

In order to make these functions cleaner, we might want to include the use of Macros in order to do the validation outside of the functions.

**Macros** are preprocessor statements for creating function-like things that are evaluated before the program is compiled. They work by taking some arguments and pasting them into some pattern. Macros are defined using the preprocessor directive `#define` followed by the name of the macro followed by the argument names in parentheses followed by the pattern for the macro.

Macros are usually given capitalized names to help distinguish them from regular functions. Our error checking macro might look like this:

```
#define LASSERT(args, cond, err) \
    if (!(cond)) { lval_del(args); return lval_err(err); }
```

Modified `head` and `tail` after LASSERT Macro has been created:

```
lval* builtin_head(lval* a) {
    LASSERT(a, a->count == 1, "Function 'head' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'head' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0, "Function 'head' passed {}!");

    lval* v = lval_take(a, 0);
    while (v->count > 1) { lval_del(lval_pop(v, 1)); }
    return v;
}
```

```
lval* builtin_tail(lval* a) {
    LASSERT(a, a->count == 1, "Function 'tail' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'tail' passed incorrect type!");
    LASSERT(a, a->cell[0]->count != 0, "Function 'tail' passed {}");

    lval* v = lval_take(a, 0);
    lval_del(lval_pop(v, 0));
    return v;
}
```

`list` converts an input S-Expression into a Q-Expression and returns it

```
lval* builtin_list(lval* a) {
    a->type = LVAL_QEXPR;
    return a;
}
```

`eval` acts kind of the opposite - takes a Q-Expression, converts it to an S-Expression and evaluates it using `lval_eval`

```
lval* builtin_eval(lval* a) {
    LASSERT(a, a->count == 1, "Function 'eval' passed too many arguments!");
    LASSERT(a, a->cell[0]->type == LVAL_QEXPR, "Function 'eval' passed incorrect type!");

    lval* x = lval_take(a, 0);
    x->type = LVAL_SEXPR;
    return lval_eval(x);
}
```

For `join`, we're going to utilize two different functions - `builtin_join` that iterates through the argument list, passing them to `lval_join`, along with the next element in the argument lsit, which combines and returns the two elements so that `builtin_joijn` has an updated `lval` with the two elements joined together and can add more onto it.

**Builtin lookup**
In order to tell the program which function to call, we define a function `builtin` which takes in an `lval` and the name of the function, performs a string compare on the function name, and returns a call to the appropriate builtin function, passing the `lval` as an argument. We can use this to also work on previously created functions resolved with `builtin_op`

Now that all of the functions navigated to using `builtin`, we can change the line in `lval_eval_sexpr` to call `builtin` rather than `builtin_op`
