Lisps are famous for having no distinction between data and code, using the same structures to represent both. We have to separate the reading of the input from the evaluating of the input.

**S-Expression** - symbolic expression -> structure built up recursively of numbers, symbols and other lists.

The evaluation behavior of S-Expressions is to look at the first item in the list, take this to be the oeprator, and take other items as operands.

When you call a funcgtion in C, the arguments are passed by value -> a copy of them is passed to the function call.

By using addresses instead of actual data, we allow functions to access andc modify some location of memory without having to copy any data.

A **pointer** is just a number representing the starting index of the data in memory. The type of pointer tells us what might be stored there.

'\*' means pointer
'&' is how you get the address of some data
'\*' can also be used to deference a pointer to get data at an address. `->` is used to do this for a field of a pointer to a struct.

**Stack** - memory where your program lives

**Heap** - section of memory for storage of objects with longer lifespan. Have to manually allocate space with malloc/calloc and manually free it past use.

**Memory leak** - when program keeps allocating more space without freeing previously used space

S-Expressions are variable length lists of other values.

**strlen** does not include the null terminator in its calculation, so 1 must be added to get the size of the array.
