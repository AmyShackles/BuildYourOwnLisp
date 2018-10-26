/* 
CODING GRAMMARS!

Example, write code for grammar that recognizes the language of Shiba Inu.

Definitions:
Adjective can be either 'wow', 'many', 'so', or 'such'
Noun can be 'lisp', 'language', 'c', 'book', or 'build'
Phrase is an Adjective followed by a Noun
A Doge is zero or more Phrases. 
*/

#include "mpc.h"

/* Build a parser 'Adjective' to recognize descriptions */

int main(int argc, char** argv) {
	mpc_parser_t* Adjective = mpc_or(4,
		mpc_sym("wow"), mpc_sym("many"),
		mpc_sym("so"), mpc_sym("such)
	);

	/* Build a parser 'Noun' to recognize things */
	mpc_parser_t* Noun = mpc_or(5,
		mpc_sym("lisp"), mpc_sym("language"),
		mpc_sym("book"), mpc_sym("build"),
		mpc_sym("c")
	);

	/* To define Phrase, we reference existing parsers.
		Need to use the function mpc_and - 
		which specifies one thing is required, then another
		As input, we pass it Adjective and Noun
		The function also takes the arguments mpc_strfold and free, 
		which say how to join or delete the results of these parsers.  
		(Ignore these arguments for now) 
	*/
	
	mpc_parser_t* Phrase = mpc_and(2, mpcf_strfold,
		Adjective, Noun, free);
	
	/* To define Doge, we must specify that zero or more of some parser is required.
		For this we need to use the function mpc_many.
		As before, requires mpcf_strfold to say how the results are joined together, 
		which we can ignore 
	*/
	
	mpc_parser_t* Doge = mpc_many(mpcf_strfold, Phrase);
	
	/* Do some parsing here ... */
	mpc_delete(Doge);
	
	return 0;
}

/* By creating a parser that looks for zero or more occurrences of another parser,
	our Doge parser accepts inputs of any length - making its language infinite! 
	
	Examples of possible strings Doge could accept:

		"wow book such language many lisp"
		"so c such build such language"
		"many build wow c"
		""
		"wow lisp wow c many language"
		"so c"

*/