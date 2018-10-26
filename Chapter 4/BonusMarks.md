# Chapter 4

### What does the \n mean in strings?
\n is the newline character, is an end-of-line/line-break character and inserts a new line in the text.

### What other patterns can be used with printf?

- %c writes a single character
- %d or %i converts a signed integer to a decimal
- %o converts an unsigned integer into an octal
- %x or %X converts an unsigned integer into hexadecimal
- %u converts an unsigned integer into a decimal
- %f or %F converts a floating point number to a decimal
- %e or %E converts a floating point number to decimal exponent notation
- %a or %A converts a floating point number to hexadecimal exponent notation
- %g or %G converts a floating point number to decimal or decimal exponent notation
depending on the value and the precision
- %n - returns the number of characters written so far by this call to the function
- %p - writes an implementation defined character sequence defining a pointer

### What happens when you pass printf a variable that does not match the pattern?
'Undefined behavior' 

### What does the preprocessor command #ifndef do?
The directive #ifndef is used to test if something is not defined as a macro name.
This allows you to check if there's a definition and if there isn't, to provide one.
The ensures that you're not overwriting a definition.

### What does the preprocessor command #define do?
It allows you to create macro definitions, which are like constants.
Unlike variables, they can't be changed by your program.

### If _WIN32 is defined on Windows, what is defined for Linux or Mac?
__APPLE__ is the identifier for Mac and __linux__ is the identifier for Linux
