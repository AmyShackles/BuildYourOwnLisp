### Define a Lisp function that returns the first element from a list

```
fun {firstElement f & xs} {f}
```

### Define a Lisp function that returns the second element from a list

```
fun {secondElemnt a b & cd} {b}
```

### Define a Lisp function that calls a function with two arguments in reverse order

```
fun {compare a b} {> b a}
```

```
fun {reverse a b c d} {eval (head {d c b a})}
```

### Define a Lisp function that calls a function with arguments, then passes the result to another function

```
fun {multiplyByTwo x} {* x (add-Together 7 5)}
```

### Define a `builtin_fun` C function that is equivalent to the Lisp `fun` function

### Change variable arguments so at least one extra argument must be supplied before it is evaluated

```

```
