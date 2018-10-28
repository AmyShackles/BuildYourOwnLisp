### Run the previous chapter's code through gdb and crash it.  See what happens
- Raincheck!

### How do you give an enum a name?
	- You just put a name after enum and before you declare the options
	
### What are union data types and how do they work?
	- Good question!  Unions are like structures with some critical differences -
	you can access anything in a struct at any point, whereas you can only access
	one element of a union at a time. 
	Unions only take up as much space as the largest element in the union because
	only one element is accessible at a time and the other elements are filled with
	garbage values.
	Structs are allocated the amount of space it takes for each element of the struct.
	
### What are the advantages of using a union instead of a struct?
	- They take up less memory
	
### Can you use a union in the definition of lval?
	- No, because you have to access different elements
	
### Extend parsing and evaluation to support the remainder operator
 - Did that a while ago
 
### Extend parsing and evaluation to support decimal types using a double field.
- Raincheck!