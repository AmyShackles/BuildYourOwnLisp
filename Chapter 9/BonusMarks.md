### Give an example of a variable in our program that lives on The Stack:
	mpc_parser_t* instances live on the stack
	
### Give an example of a variable in our program that lives on The Heap:

	lval* v instances live on the heap - they need to be given space with malloc and freed at a later point.
	
### What does the strcpy function do?
	Strcpy copies a string - it takes as arguments a pointer to a destination array where the string will be copied
	and the string to be copied.
	
	So in char* readline, strcpy(cpy, buffer) is copying the contents of buffer into the empty array cpy.
	
### What does the realloc function do?
	- It tries to resize the memory block pointed to by the first argument to the size of the second argument.
	- If successful, it returns the pointer to the new space in memory
	- If unsuccessful, it returns NULL
	
### What does the memmove function do?
	- Copies a block of memory from one location to another.

### How does memmove differ from memcpy?
	- Memcpy just copies from one location to another.
	- Memmove copies the data to an intermediate buffer and then from buffer to destination.
	- Memcpy has issues when strings overlap.

### Extend parsing and evaluation to support the remainder operator %:
	- Already done!
	
### Extend parsing and evaluation to support decimal types using a double field:
	- It's probably not the best way to go about implementing it, but it works.