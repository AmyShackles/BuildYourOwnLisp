### Write a recursive function to compute the number of leaves of a tree.
	int number_of_leaves(mpc_ast_t* t) {
		int branches = number_of_branches(r.output)
		if (branches->children_num == 0) { return 1; )
		if (branches->children_num >= 1) {
			int total = 1;
			for (int i = 0; i < branches->children_num; i++) {
			total = total + number_of_leaves(branches->children[i]);
			}
			return total;
		)
		return 0;
	}


### Write a recursive function to compute the number of branches of a tree.
	int number_of_branches(mpc_ast_t* t) {
		if (t->children_num == 0) { return 1; )
		if (t->children_num >= 1) {
			int total = 1;
			for (int i = 0; i < t->children_num; i++) {
			total = total + number_of_branches(t->children[i]);
			}
			return total;
		)
		return 0;
	}

### How would you use strstr to see if a node was tagged as an expr?
	strstr(t-> tag, "expr")


### How would you use strcmp to see if a node had the contents '(' or ')'?
	strcmp(op, "(") == 0)
	strcmp(op, ")") == 0)

### Add the operator modulo, which returns the remainder of division.
	- Added!

### Add the operator ^, which raises one number to another.  For example ^ 4 2 is 16
	- Added!
	
### Add the function min which returns the smallest number. For example min 1 5 3 is 1
	- Added!
	
### Add the function max which returns the biggest number.  For example max 1 5 3 is 5
	- Added!
	
### Change the minus operator so that when it receives one argument it negates it
	- Added a 'neg' operator that negates all numbers following it ...