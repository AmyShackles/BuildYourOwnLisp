## Types in C

### Void

### Char

#### Char
	Type for character representation
	Equivalent to either signed char or unsigned char
#### Signed Char
	Type for signed character representation
#### Unsigned Char
	Type for unsigned character representation
	Also used to inspect object representations (raw memory)

### Int

#### Short
	Accessible also as short
#### Int
	Most optimal integer type, guaranteed 16 bits, most systems use 32
	Accessible also as signed int
#### Long
	Accessible also as long
#### Long Long Int
	Accessible also as long long 
#### Unsigned Short Int
	Accessible also as unsigned short
#### Unsigned Int
	Accessible also as unsigned
#### Unsigned Long Int
	Accessible also as unsigned long
#### Unsigned Long Long
	Accessible also as unsigned long long

### Float

#### Float
	Single precision floating point
#### Double
	Double precision floating point
#### Long Double
	Extended precision floating point
	
### Complex Floats - Float _Complex, Double _Complex, Long Double _Complex
	Modeling sum of real number and a real number multiplied by imaginary unit
	
### Imaginary Float - Float _Imaginary, Double _Imaginary
	Real number multiplied by the imaginary unit

### Enumerated Types
	Type whose value is the value of its underlying type

### Derived Types

#### Array Types
	Contiguously allocated nonempty sequence of objects of a particular type
#### Structure Types
	Used to declare new types
	Several variables bundled together
	Storage allocated in an ordered sequence
#### Union Types
	Like a struct but storage overlaps
	Value of at most one of the members can be stored in a union at once
#### Function Types
	Associates a compound statement with an identifier
#### Pointer Types
	Refers to a function or object of another type	

## Type Groups:
object - all types that aren't function types
character - char, signed char, unsigned char
integer - char, signed integer types, unsigned integer types, enumerated types
real types - integer types and real floating types
arithmetic - integer types and floating types
scalar - arithmetic types and pointer types
aggregate types - array types and structure types
derived declarator - array types, function types, pointer types

# What other conditional operators are there other than > and < ?
	? - ternary condition operators
	>= - greater than or equal to
	<= - less than or equal to
	== - equal to
	!= - not equal to
	
# What other mathematical operators are there other than add and subtract?
	Negation (-)
	Multiplication(*)
	Division(/)
	Modular division(%)
	
# What is the += operator and how does it work?
	+= is an assignment operator and it works by adding two operands together
	... and then assigning the value to the operand on the left
	
# What is the do loop and how does it work?
	The do loop is like the while loop except that it executes the code
	... and then checks to see if the test passes
	
# What is the switch statement and how does it work?
	A statement to compare one expression with others and then execute sub-statements
	It's like a key/value pair - in a given scenario, look for the value that matches
	... if a match is found, carry out the code associated with that match
	
# What is the break keyword and what does it do?
	Break is used to end a while/do/for loop/switch statement
	
# What is the continue keyword and what does it do?
	Continue is used to stop the current iteration of the loop and move on to the next one
	
# What does the typedef keyword do?
	Typedef's used to give a name to new data Type
	
