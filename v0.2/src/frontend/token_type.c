#include "token_type.h"

// symbols
const struct TokenType TOKEN_TYPE_LPAREN = { 1, "LPAREN" };	// "("
const struct TokenType TOKEN_TYPE_RPAREN = { 2, "RPAREN" };	// ")"

const struct TokenType TOKEN_TYPE_LCURLY_BRACE = { 3, "LCURLY_BRACE" };	// "{"
const struct TokenType TOKEN_TYPE_RCURLY_BRACE = { 4, "RCURLY_BRACE" };	// "}"

const struct TokenType TOKEN_TYPE_LSQUARE_BRACE = { 5, "LSQUARE_BRACE" };	// "["
const struct TokenType TOKEN_TYPE_RSQUARE_BRACE = { 6, "RSQUARE_BRACE" };	// "]"

const struct TokenType TOKEN_TYPE_PLUS			= {  7, "PLUS" };			// "+"
const struct TokenType TOKEN_TYPE_MINUS			= {  8, "MINUS" };			// "-"
const struct TokenType TOKEN_TYPE_STAR			= {  9, "STAR" };			// "*"
const struct TokenType TOKEN_TYPE_FORWARD_SLASH	= { 10, "FORWARD_SLASH" };	// "/"
const struct TokenType TOKEN_TYPE_PERCENT		= { 11, "PERCENT" }; 		// "%"

const struct TokenType TOKEN_TYPE_EQUAL			= { 12, "EQUAL" };			// "="

// const struct TokenType TOKEN_TYPE_PLUS_EQUAL		= { 12, "PLUS_EQUAL" };		// "+="
// const struct TokenType TOKEN_TYPE_MINUS_EQUAL	= { 12, "MINUS_EQUAL" };	// "-="
// const struct TokenType TOKEN_TYPE_STAR_EQUAL		= { 12, "STAR_EQUAL" };		// "*="
// const struct TokenType TOKEN_TYPE_DIVIDE_EQUAL	= { 12, "DIVIDE_EQUAL" };	// "/="
// const struct TokenType TOKEN_TYPE_MOD_EQUAL		= { 12, "MOD_EQUAL" };		// "%="

const struct TokenType TOKEN_TYPE_NOT		= { 13, "NOT" };		// "!"
const struct TokenType TOKEN_TYPE_DOT		= { 14, "DOT" };		// "."
const struct TokenType TOKEN_TYPE_COMMA		= { 15, "COMMA" };		// ","
const struct TokenType TOKEN_TYPE_COLON		= { 16, "COLON" };		// ":"
const struct TokenType TOKEN_TYPE_SEMICOLON = { 17, "SEMICOLON" };	// ";"

const struct TokenType TOKEN_TYPE_PLUS_PLUS	  = { 18, "PLUS_PLUS" };	// "++"
const struct TokenType TOKEN_TYPE_MINUS_MINUS = { 19, "MINUS_MINUS" };	// "--"

const struct TokenType TOKEN_TYPE_EQUAL_EQUAL			= { 20, "EQUAL_EQUAL" };			// "=="
const struct TokenType TOKEN_TYPE_NOT_EQUAL				= { 21, "NOT_EQUAL" };				// "!="
const struct TokenType TOKEN_TYPE_LESS_THAN				= { 22, "LESS_THAN" };				// "<"
const struct TokenType TOKEN_TYPE_LESS_THAN_EQUALS		= { 23, "LESS_THAN_EQUALS" };		// "<="
const struct TokenType TOKEN_TYPE_GREATER_THAN 			= { 24, "GREATER_THAN" };			// ">"
const struct TokenType TOKEN_TYPE_GREATER_THAN_EQUALS	= { 25, "GREATER_THAN_EQUALS" };	// ">="

const struct TokenType TOKEN_TYPE_LOGICAL_AND = { 26, "LOGICAL_AND" };	// "&&"
const struct TokenType TOKEN_TYPE_LOGICAL_OR  = { 27, "LOGICAL_OR" };	// "||"

const struct TokenType TOKEN_TYPE_DASH_ARROW 	= { 28, "DASH_ARROW" };		// "->"
const struct TokenType TOKEN_TYPE_EQUAL_ARROW 	= { 29, "EQUAL_ARROW" }; 	// "=>"
// If want to change ordinals of start and end symbols token type
// then also modify the print_token function accordingly


// keywords
const struct TokenType TOKEN_TYPE_VAR	= { 30, "VAR" };
const struct TokenType TOKEN_TYPE_VAL	= { 31, "VAL" };

const struct TokenType TOKEN_TYPE_TRUE	= { 32, "TRUE" };
const struct TokenType TOKEN_TYPE_FALSE	= { 33, "FALSE" };

const struct TokenType TOKEN_TYPE_IF	= { 34, "IF" };
const struct TokenType TOKEN_TYPE_ELSE	= { 35, "ELSE" };
const struct TokenType TOKEN_TYPE_WHEN	= { 36, "WHEN" };

const struct TokenType TOKEN_TYPE_LOOP	 = { 37, "LOOP" };
const struct TokenType TOKEN_TYPE_REPEAT = { 38, "REPEAT" };

const struct TokenType TOKEN_TYPE_BREAK 	= { 56, "BREAK" };
const struct TokenType TOKEN_TYPE_CONTINUE 	= { 57, "CONTINUE" };

const struct TokenType TOKEN_TYPE_CLASS = { 39, "CLASS" };
// const struct TokenType TOKEN_TYPE_SUPER = { 40, "SUPER" };
// const struct TokenType TOKEN_TYPE_THIS	= { 41, "THIS" };

const struct TokenType TOKEN_TYPE_FUN	= { 42, "FUN" };
const struct TokenType TOKEN_TYPE_RET	= { 43, "RET" };



/*
class
else
false
fun
if
loop
print
repeat
ret
super
this
true
var
when

fun add(int a, int b) {
	ret (a + b);
}

*/

const struct TokenType TOKEN_TYPE_IDENTIFIER = { 44, "IDENTIFIER" };


// literals
const struct TokenType TOKEN_TYPE_INTEGER 	= { 45, "INT" };
const struct TokenType TOKEN_TYPE_DOUBLE	= { 46, "DOUBLE" };
const struct TokenType TOKEN_TYPE_STRING	= { 47, "STRING" };


// miscellaneous
const struct TokenType TOKEN_TYPE_ERROR = { 48, "ERROR" };
const struct TokenType TOKEN_TYPE_EOF   = { 49, "EOF" };


// temporary identifier tokentype
const struct TokenType TOKEN_TYPE_PRINT	= { 50, "PRINT" };




const struct TokenType TOKEN_TYPE_PLUS_EQUAL		= { 51, "PLUS_EQUAL" };		// "+="
const struct TokenType TOKEN_TYPE_MINUS_EQUAL		= { 52, "MINUS_EQUAL" };	// "-="
const struct TokenType TOKEN_TYPE_STAR_EQUAL		= { 53, "STAR_EQUAL" };		// "*="
const struct TokenType TOKEN_TYPE_SLASH_EQUAL		= { 54, "SLASH_EQUAL" };	// "/="
const struct TokenType TOKEN_TYPE_PERCENT_EQUAL		= { 55, "PERCENT_EQUAL" };	// "%="


