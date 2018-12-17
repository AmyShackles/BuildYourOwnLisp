Lisps are famous for having no distinction between data and code, using the same structures to represent both. We have to separate the reading of the input from the evaluating of the input.

**S-Expression** - symbolic expression -> structure built up recursively of numbers, symbols and other lists.

The evaluation behavior of S-Expressions is to look at the first item in the list, take this to be the oeprator, and take other items as operands.

When you call a funcgtion in C, the arguments are passed by value -> a copy of them is passed to the function call.

By using addresses instead of actual data, we allow functions to access andc modify some location of memory without having to copy any data.

A **pointer** is just a number representing the starting index of the data in memory. The type of pointer tells us what might be stored there.

'\*' means pointer
'&' is how you get the address of some data
'\*' can also be used to deference a pointer to get data at an address. `->` is used to do this for a field of a pointer to a struct.

**Stack** - memory where your program lives

**Heap** - section of memory for storage of objects with longer lifespan. Have to manually allocate space with malloc/calloc and manually free it past use.

**Memory leak** - when program keeps allocating more space without freeing previously used space

Since S-Expressions are a collection of symbols and numbers, we need to add symbol and number `lval` types. `LVAL_SYM` will be used to represent operations such as `*`

```
enum { LVAL_ERR, LVAL_NUM, LVAL_SYM, LVAL_SEXPR }
```

S-Expressions are variable length lists of other values. Because we can't create variable length structs, we need a field that points to the location where we store a list of `lval*`. This field, then, is a pointer to a pointer (`lval**`). We also need to keep track of how many elements are in the list of structs. The definition for lval then updates to:

```
typedef struct lval {
    int type;
    long num;
    char* err;
    char* sym;
    int count; // number of elements in cell
    struct lval** cell; // list of lval pointers
} lval;
```

Because `lval` needs to contain a reference to itself, we need to name it before the opening bracket so that the compiler understands what type `cell` is.

We then change the functions that construct `lval` to return pointers rather than the structs themselves in order to make the tracking of variables more simple. In order to allocate enough space for the lval struct, we need to malloc with sizeof and then fill its fields with the arrow operator. And because the fields for `lval` may contain pointers to other things that have been allocated on the heap, we need to delete the things it points to on the heap when we are done with the `lval`.

When we initialize an S-Expression, we set the `cell` field to NULL to indicate that it is a pointer that doesn't yet point to anything.

**strlen** does not include the null terminator in its calculation, so 1 must be added to get the size of the array.

We then need a function that deletes `lval` and calls free on all of the things inside the `lval` type that would need to be deallocated. Because what is freed depends on the type of `lval1`, a switch case can be implemented to check for the type and perform the appropriate deallocation.

**Way the Lisp interpreter works**

Reads in the program, constructs an `lval*` to represent it, and then evalutes it to get the result.

We covert the abstract syntax tree to an S-Expression by recursively checking each node of the tree and constructing different `lval*` types depending on the tag and contents of the node. If the node is tagged as a number or symbol, it can return an `lval*` directly whereas if the node is a root or sexpr, we have to create an empty S-Expression `lval` and add each valid sub-expression to it.

`lval_add` will add elements to an S-Expression and increase the count of the list in the `cell` field, reallocating the amount of space required by v->cell. The new space can be used to store the extra `lval*` required and it sets the final value of the list with `v->cell[v->count-1]` to the value `lval* x` passed in.

We then need to change the print statement to be able to print S-Expressions by looping over all the sub-expressions of an expression and printing them

**Forward declaration** - when you type a function declaration without a body followed by a semicolon in order to signal to the compiler that that function will be defined later.

**Evaluation for S-Expressions**

1 - Evaluate children - If any are errors, return the first error using `lval_take`

2 - If no children, return directly

3 - If one child, return the single expression contained within parentheses

4 - If There are no errors and the expression has more than one child, we separate the first element of the expression with `lval_pop`, check if it's a symbol and if it is, check what symbol it is and then pass it and the arguments to `builtin_op` to perform calculations. If the first element is not a symbol, we delete it and the values passed into the evaluation function, returning an error.

To evaluate other types, we just return them.

`lval_pop` extracts one element from an S-Expression at index 'i' and shifts the rest of the list back so that it no longer contains that pointer and then returns the extracted value. It doesn't delete the input list or the element removed from the list, so both need to be deleted with `lval_del` later on.

`lval_take` is similar to `lval_pop` but deletes the list after extraction, which means that only the element extracted in `lval_take` needs to be deleted later.

We create the function `builtin_op` in order to evaluate the input. If any input is a non-number `lval*`, it should return an error. It pops the first element and if there are no arguments and the element is a '-', it performs unary negation on the first number. It then pops elements one by one and performs arithmetic based on what operator is currently in operation. If a zero is encountered on division, `x`, `y` and the argument list `a` are destroyed and an error is returned. If no errros are encountered, the argument list is deleted and the new expression is returned.
