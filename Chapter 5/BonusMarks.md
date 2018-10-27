### Write down some more examples the Doge language contains
		"												\
			adjective	: \"very\" | \"much\";			\
			noun		: \"art\" | \"dignity\";		\
			verb		: \"amaze\" | \"eat\"			\
						: \"fall\" | \"trick\"			\
						: \"feels\" | \"excite\"		\
			phrase		: <adjective> <noun>			\
						| <adjective> <verb>;			\
			doge		: <phrase>*;					\
		",
		
### Why are there back slashes in front of quote marks in the grammar?
Because we're escaping them!

### Why are there backslashes at the end of the line in the grammar?
This indicates a continued line.  The backslash in this case in called a backslash-newline.
This tells the preprocessor to join the current line with the one following it.


### Describe textually a grammar for decimal numbers such as 0.01 or 52.221
- Zero or more numbers followed by a decimal followed by one or more numbers.
	mpc_parser_t* Tens = mpc_new("tens");
	mpc_parser_t* Ones = mpc_new("ones");
	mpc_parser_t* Decmimal = mpc_new("decimal");
	mpc_parser_t* Tenths = mpc_new("tenths");
	mpc_parser_t* Hundredths = mpc_new("hundredths");
	mpc_parser_t* Thousandths = mpc_new("thousandths");
	mpc_parser_t* Number = mpc_new("number");
		
	mpca_lang(MPCA_LANG_DEFAULT,
	"
		tens		:	/[0-9]/;	\
		ones		:	/[0-9]/;	\
		decimal		:	/./;		\
		tenths		:	/[0-9]/;	\
		hundredths	:	/[0-9]/;	\
		thousandths	:	/[0-9]/;	\
		number		:	<tens> <ones> <decimal> <tenths> <hundredths> <thousandths>;
	",
	Tens, Ones, Decimal, Tenths, Hundredths, Thousandths, Number);
	


### Describe textually a grammar for web URLs such as http://www.buildyourownlisp.com
- A web URL is a protocol followed by a domain followed by a toplevel domain.
	mpc_parser_t* Protocol = mpc_new("protocol");
	mpc_parser_t* Domain = mpc_new("domain");
	mpc_parser_t* TopLevelDomain = mpc_new("topleveldomain");
	mpc_parser_t* URI = mpc_new("uri");
	mpc_parser_t* URL = mpc_new("url");
	
	mpca_lang(MPCA_LANG_DEFAULT,
	"
		protocol		:	\"http\"				| \"https\";		\
		domain			:	\"buildyourownlisp\"	| \"google\"		\
						|	\"facebook\"			| \"lambdaschool\";	\
		topleveldomain	:	\".com\"				| \".net\"			\
						|	\"org\"					| \".biz\";			\
		uri				:	<protocol> <domain> <topleveldomain>;
		url				:	<uri>;
	",
	Protocol, Domain, TopLevelDomain, URI, URL);
	
### Describe textually a grammar for simple English sentences such as 'the cat sat on the mat'
- A simple English sentence is an article followed by a noun followed by a verb 
followed by a preposition followed by another noun.

	mpc_parser_t* Article = mpc_new("article");
	mpc_parser_t* Animal = mpc_new("animal");
	mpc_parser_t* Verb = mpc_new("verb");
	mpc_parser_t* Preposition = mpc_new("preposition");
	mpc_parser_t* Noun = mpc_new("noun");
	mpc_parser_t* Sentence = mpc_new("sentence");
	
	mpca_lang(MPCA_LANG_DEFAULT,
	"
		article			:	\"the\"	| \"a\"				\
						|	\"an\";						\
		animal			:	\"cat\"	| \"dog\"			\
						|	\"bird\" | \"mouse\";		\
		verb			:	\"sat\" | \"sits\"			\
						|	\"lies\" | \"sleeps\"		\
						|	\"slept\" | \"lays\"		\
		preposition		:	\"on\" | \"against\"		\
						|	\"by\" | \"near\";			\
		noun			:	\"shelf\"	| \"mat\"		\
						|	\"carpet\"	| \"table\";	\
		sentence		:	<article> <animal> <verb> <preposition> <article> <noun>;
	",
	Article, Animal, Verb, Preposition, Noun, Sentence);
	
### Textually describe a grammar for JSON
- JSON is a { followed by zero or more numbers and/or strings, and/or booleans -
can include an opening array followed by zero or more numbers and/or strings and/or booleans
followed by an end array followed by }
	mpc_parser_t* Number = mpc_new("number");
	mpc_parser_t* String = mpc_new("string");
	mpc_parser_t* Boolean = mpc_new("boolean");
	mpc_parser_t* BeginArray = mpc_new("beginarray");
	mpc_parser_t* EndArray = mpc_new("endarray");
	mpc_parser_t* BeginObject = mpc_new("beginobject");
	mpc_parser_t* EndObject = mpc_new("endobject");
	mpc_parser_t* NameSeparator = mpc_new("nameseparator");
	mpc_parser_t* ValueSeparator = mpc_new("valueseparator");
	
	mpca_lang(MPCA_LANG_DEFAULT,
	"
		number				:	\[0-9]*\;				\
		string				:	\[a-zA-Z]*\;			\
		boolean				:	\true\ | \false\;		\
		beginarray			:	\[\;					\
		endarray			:	\]\;					\
		beginobject			:	\{\;					\
		endobject			:	\}\;					\
		nameseparator		:	\;\	| \:\;				\
		valueseparator		:	\;\ | \,\;				\
	",
	Number, String, Boolean, BeginArray, EndArray, BeginObject, EndObject, NameSeparator, ValueSeparator);
	