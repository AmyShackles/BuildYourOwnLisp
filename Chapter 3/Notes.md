A program in C consists of only function definitions and structure definitions -> functions and types

To create variables: declare type, name, [value]

Declarations are statements and must end with `;`

Example types:
| Name | Type |
|-------- |------------------------------------- |
| void | empty type |
| char | single character/byte |
| int | integer |
| long | integer that can hold larger values |
| float | decimal number |
| double | decimal number with more precision |

**Functions**

Function -> manipulates variables, optionally changes state of program, takes as input some variables and returns some single variable as output

To declare:

type of return variable name of function (comma separated input list) {

&nbsp;&nbsp;&nbsp;&nbsp;list of statements to execute;

&nbsp;&nbsp;&nbsp;&nbsp;return statement to have function finish output

}

For example:

```
int digitValue(char digitChar) {
    if (digitChar > '9') {
        digitChar = (digitChar - '0') - 7;
    } else {
        digitChar = digitChar - '0';
    }
    return digitChar;
}
```

`int` is the type of return variable
`digitValue` is the name of the function
`char digitChar` is the input
The instructions between the first open bracket and the second to last closing bracket are the list of statements to execute
`return digitChar` is the return statement to end the function

You call functions by writing their name and supplying arguments in parentheses, separated by commas.

**Structures**

- Used to declare new types
- Made up of several variables bundles together

To declare a structure:

```
typedef struct {
    type name
} name of struct
```

You access fields on the struct with dot notation

Example:

```

typedef struct {
float x;
float y;
} point;

point p;
p.x = 0.1;
p.y = 10.0;

float length = sqrt(p.x * p.x + p.y * p.y);

```

**Pointers**

int\* -> pointer to integer<br/>
char\*\* -> pointer to pointer to characters<br/>
char\* -> string (array/list) of characters ending in a null terminator

**Conditionals** -> used to execute code if certain conditions are met.

- inside a conditional statement's parantheses, any value that is not 0 will evaluate to true

**Loops** -> repeat code until condition false or counter elapses

- Two main types: for and while
  - While loop -> while (condition) {}
  - For loop -> for (initializer, condition, incrementer/decrementer)
    - Condition checked before each run of the loop. If false, exit
    - Initializer performed before loop starts
    - Often used for counting as it's more compact than a while loop
