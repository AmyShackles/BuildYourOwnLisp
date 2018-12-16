**Abstract Syntax Tree** - represents structure of the program based on the input entered by the user. - at leaves, number and operators - at branches, rules to produce that part of the tree

The structure for mpc_ast_t is as follows:

```
typedef struct mpc_ast_t {
    char* tag;
    char* contents;
    mpc_state_t state;
    int children_num;
    struct mpc_ast_t** children;
} mpc_ast_t;
```

tag: String containing the rules used to parse the item. Example: exp|number|regex

contents: Actual contents of a node

state: stater parser was in when it found the node -> line and column number (won't use in program)

children num: How many children a node has

children: Array of children - Access children with array/bracket notation.

Because `mpc_ast_t is a pointer to a struct, you need to use arrow syntax instead of dot syntax (field access of pointer types use an arrow)

\*\*Observations on syntax tree:

- Node tagged with number is always a number and has no children - base case

- If tagged with Exp and not Number, need to look at second child (first is always open paren) to see what operator it is and apply it to eval of remaining children excluding the last which is always close paren - recursive case

Need to accumulate result, can do so with lont (long int)

**String functions**:

atoi: converts a char\* to an int

strcmp: takes as input two char\* and if they are equal it returns 0

strstr: takes as input two char\* and returns a pointer to the location of the second in the first, or 0 if the second is not a substring of the first.

Use strcmp to check which operator to use.

Use strstr to check if a tag contains some substring

`eval_op` takes in a number, operator string, and another number. It tests for which operator is passed in and performs the corresponding operation on those inputs

You can pass the syntax tree into the eval function and print the result using printf and %li
