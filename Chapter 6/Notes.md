**Polish notation** -> operator before operands

Example:
| Regular notation | vs | Polish Notation |
|-------------------- |---- |-------------------- |
| 1 + 2 + 6 | is | + 1 2 6 |
| 6 + (2 \* 9) | is | + 6 (\* 29) |
| (10 \* 2) / (4 + 2) | is | / (\* 10 2) (+ 4 2) |

A program is an operator followed by one or more expressions where an expression is either a number or, in parentheses, an operator followed by one or more expressions.

### More formally:

**Program**: Start of input, an operator, one or more Expressions, end of input

**Expression**: Either a number or '(', an operator, one or more Expressions, and an ')'

**Operator**: '+', '-', '\*', '/'

**Number**: An optional - and one or more characters between 0 and 9.

### REGULAR EXPRESSION TIME

| RegEx    | Meaning                                           |
| -------- | ------------------------------------------------- |
| .        | Any character is required                         |
| a        | The character _a_ is required                     |
| [abcdef] | Any character in the set _abcdef_ is required     |
| [a-f]    | Any character in the range _a_ to _f_ is required |
| a?       | The character _a_ is optional                     |
| a\*      | Zero or more of the character _a_ is required     |
| a+       | One or more of the character _a_ is required      |
| ^        | The start of input is required                    |
| \$       | The end of input is required                      |

Example for number:<br/>
`/-?[0-9]+/` // Optional -, one or more numbers 0-9

**MPC LIBRARY**
https://github.com/orangeduck/mpc

**COMMAND TO COMPILE INTERPRETER**:

On Linux and Mac:

```
cc -std=c99 -Wall nameofprogram.c mpc.c -ledit -lm -o nameofprogram
```

On Windows:

```
cc -std=c99 -Wall nameofprogram.c mpc.c -o nameofprogram
```

We use `mpc_parser_t` as the type to represent our parsers and create them using `mpc_new`. We then define them using `mpca_lang` and need to remember to do cleanup just before main returns by implementing `mpc_cleanup`.

We use `mpc_parse` to attempt to parse user input using our parser. On success, an internal structure is copied into `r` in the field output which we can then print using `mpc_ast_print` and delete using `mpc_ast_delete`. If there has been an error, that error is copied into r in the field `error` and can be printed using `mpc_err_print` and deleted using `mpc_err_delete`.
