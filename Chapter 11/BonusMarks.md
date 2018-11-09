### Create a Macro to aid specifically with reporting type errors
	#define LASSERT_TYPE(func, args, index, expect) \
		LASSERT(args, args->cell[index]->type == expect, \
			"Function '%s' passed incorrect type for argument %i.  Got %s, Expected %s.", \
			func, index, ltype_name(args->cell[index]->type), ltype_name(expect))
	

### Create a Macro to aid specifically with reporting count errors
	#define LASSERT_NUM(func, args, num) \
		LASSERT(args, args->count == num, \
			"Function '%s' passed incorrect number of arguments.  Got %i, Expected %i.", \
			func, args->count, num)

### Change printing a builtin function so that it prints its name
	- Can't figure out how to do this at the moment

### Write a function for printing out all the named values in an environment
	- Will come back to this later!

### Redefine one of the builtin variables to something different
	- Unsure what this is asking - will come back to later

### Change redefinition of one of the builtin variables to something different an error
	- Don't understand what this is asking me to do.

### Create an exit function for stopping the prompt and exiting
	- Typing 'exit' now prompts an exit condition