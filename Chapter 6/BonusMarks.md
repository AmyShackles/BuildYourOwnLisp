### Write a regular expression matching strings of all a or b - 
such as aababa or bbaa
- /a*b*/

### Write a regular expression matching strings of consecutive a or b - 
such as ababab or aba
- /ab+ || ba+/

### Write a regular expression matching pit, put, and respite but not peat, spit, or part
- /(res)?p[iu]te?/

### Change the grammar to add a new operator such as %
- added % to operators list

### Change the grammar to recognize operators written in textual format
- added \"add\", \"sub\", \"mul\", \"div\", and \"modulo\"

### Change the grammar to recognize decimal numbers such as 0.01, 5.21, or 10.2
- Changed number regex to /-?[0-9]+\.[0-9]+/

### Change the grammar to make the operators written conventionally between two expressions
	expr		: <number> | '(' <expr> <operator> <expr>+ ')' ;	\
	lisp		: /^/ <expr> <operator> <expr>+ /$/ ;				\
	
### Use the grammar from the previous chapter to parse Doge.
	- You must add start and end of input