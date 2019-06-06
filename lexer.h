#pragma once

// tokens
enum
{
  TOKEN_EOF = 0,
  TOKEN_SPACE = 32,
  TOKEN_NOT = 33,	   //	!
  TOKEN_QUTA = 34,	 //	"
  TOKEN_HASH = 35,	 //	#
  TOKEN_DOLLER = 36,	 //	$
  TOKEN_PERCENT = 37,	 //	%
  TOKEN_AND = 38, 	 //	&
  TOKEN_PRIME = 39,	 //	'
  TOKEN_LPAREN = 40,	 //	(
  TOKEN_RPAREN = 41,	 //	)
  TOKEN_STAR = 42,	 //	*
  TOKEN_PLUS = 43,	 //	+
  TOKEN_COMMA = 44,	 //	,
  TOKEN_MINUS = 45,	 //	-
  TOKEN_DOT = 46,	   //	.
  TOKEN_SLASH = 47,	 //	/
  TOKEN_COLON = 58,	 //	:
  TOKEN_SEMICOLON = 59,//	;
  TOKEN_LT = 60,	   //	<
  TOKEN_EQ = 61,	   //	=
  TOKEN_GT = 62,	   //	>
  TOKEN_QUESTION = 63, //	?
  TOKEN_AT = 64,  	 //	@
  TOKEN_LBRACKET = 91, //	[
  TOKEN_BACKSLASH = 92,//
  TOKEN_RBRACKET = 93, //	]
  TOKEN_CARET = 94,	 //	^
  TOKEN_UNDERLINE = 95,//	_
  TOKEN_UPRIME = 96,	 //	`
  TOKEN_LBRACE = 123,	 //	{
  TOKEN_OR = 124,	 //	|
  TOKEN_RBRACE = 125,	 //	}
  TOKEN_TILDE = 126,	 //	~

  TOKEN_MINUSGT = 127, // ->
  TOKEN_LTLT,      // <<
  TOKEN_GTGT,      // >>
  TOKEN_LOR,       // ||
  TOKEN_LAND,      // &&
  TOKEN_LTEQ,      // <=
  TOKEN_GTEQ,      // >=
  TOKEN_EQEQ,      // ==
  TOKEN_NEQ,       // !=
  TOKEN_INC,       // ++
  TOKEN_DEC,       // --

  TOKEN_CONTINUE,
  TOKEN_BREAK,
  TOKEN_ELSE,
  TOKEN_IF,
  TOKEN_IMPORT,
  TOKEN_RETURN,
  TOKEN_SUPER,
  TOKEN_THIS,
  TOKEN_ID, // class, struct, function, enum, variable
  TOKEN_WHILE,
  TOKEN_DELETE,
  TOKEN_SIZEOF,
  TOKEN_TYPEOF,
  TOKEN_COUNT,
  TOKEN_NULL,
  TOKEN_REF,
  TOKEN_INSERT,

  // system function
  TOKEN_FORMAT,
  TOKEN_PRINT,

  TOKEN_CHAR,
  TOKEN_DATA,
  TOKEN_NUMBER,

  TOKEN_LINE
};

/* token type */
typedef struct token {
  long64_t key;
  long64_t value;
  long64_t pos;
  long64_t row;
  long64_t col;
} token_t;

token_t *
token_create(long64_t key, long64_t value, long64_t pos, long64_t row, long64_t col);

long64_t
token_destroy(itable_t *it);

void
lexer(table_t *ls, const char *source);
