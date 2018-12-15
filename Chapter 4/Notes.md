`fputs` - like `puts` but without newline

`editline` -> `readline` and `add_history` -> lets you record history of input, accessible with up and down arrows

`readline` -> read from prompt while allowing editing

`#include <stdlib.h>` -> adds the header file for stdlib, which includes things like `free`

`#include <editline/readline.h>` -> adds header file to include readline

`#include <editline/history>` -> adds header file to include add_history

- not necessary on Mac, just need to include `- ledit` `- ledit` allows the compiler to embed editline calls in program

**Portability is a problem in C**
Not all operating systems respond the same. The preprocessor is a way to get around this as it can detect what operating system the person is using and define rules based on operating system using `#ifdef`, `#else` and `#endif`

The part of our code that handles the differences in operating system is at the beginning of every program from here on out

```
#ifdef _WIN32
#include <string.h>

static char buffer[2048];

/* Fake readline function */
char* readline(char* prompt) {
    fputs(prompt, stdout);
    fgets(buffer, 2048, stdin);
    char* cpy = malloc(strlen(buffer)+1);
    strcpy(cpy, buffer);
    cpy[strlen(cpy)-1] = '\0';
    return cpy;
}

/* Fake add_history function */
void add_history(char* unused) {}

#else
#include <editline/readline.h>
#include <editline/history.h>
// Note, I don't include <editline/history.h>
// because my Mac is not a fan of it.
// I should really just add an elseif for Mac handling.
// I might do that later.

#endif
```
