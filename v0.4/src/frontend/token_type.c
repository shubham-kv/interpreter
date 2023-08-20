#include "token_type.h"

// symbols
const struct TokenType TOKEN_TYPE_LPAREN = { 1, "LPAREN" };	// "("
const struct TokenType TOKEN_TYPE_RPAREN = { 2, "RPAREN" };	// ")"

const struct TokenType TOKEN_TYPE_LCURLY_BRACE 	= { 3, "LCURLY_BRACE" };	// "{"
const struct TokenType TOKEN_TYPE_RCURLY_BRACE 	= { 4, "RCURLY_BRACE" };	// "}"

const struct TokenType TOKEN_TYPE_LSQUARE_BRACE = { 5, "LSQUARE_BRACE" };	// "["
const struct TokenType TOKEN_TYPE_RSQUARE_BRACE = { 6, "RSQUARE_BRACE" };	// "]"

const struct TokenType TOKEN_TYPE_PLUS			= {  7, "PLUS" };			// "+"
const struct TokenType TOKEN_TYPE_MINUS			= {  8, "MINUS" };			// "-"
const struct TokenType TOKEN_TYPE_STAR			= {  9, "STAR" };			// "*"
const struct TokenType TOKEN_TYPE_FORWARD_SLASH	= { 10, "FORWARD_SLASH" };	// "/"
const struct TokenType TOKEN_TYPE_PERCENT		= { 11, "PERCENT" }; 		// "%"

const struct TokenType TOKEN_TYPE_EQUAL			= { 12, "EQUAL" };			// "="
const struct TokenType TOKEN_TYPE_LESS_THAN		= { 13, "LESS_THAN" };		// "<"
const struct TokenType TOKEN_TYPE_GREATER_THAN 	= { 14, "GREATER_THAN" };	// ">"

const struct TokenType TOKEN_TYPE_NOT			= { 15, "NOT" };			// "!"
const struct TokenType TOKEN_TYPE_DOT			= { 16, "DOT" };			// "."
const struct TokenType TOKEN_TYPE_COMMA			= { 17, "COMMA" };			// ","
const struct TokenType TOKEN_TYPE_COLON			= { 18, "COLON" };			// ":"
const struct TokenType TOKEN_TYPE_SEMICOLON 	= { 19, "SEMICOLON" };		// ";"

const struct TokenType TOKEN_TYPE_PLUS_PLUS	  	= { 20, "PLUS_PLUS" };		// "++"
const struct TokenType TOKEN_TYPE_MINUS_MINUS 	= { 21, "MINUS_MINUS" };	// "--"

const struct TokenType TOKEN_TYPE_EQUAL_EQUAL			= { 22, "EQUAL_EQUAL" };			// "=="
const struct TokenType TOKEN_TYPE_NOT_EQUAL				= { 23, "NOT_EQUAL" };				// "!="
const struct TokenType TOKEN_TYPE_LESS_THAN_EQUALS		= { 24, "LESS_THAN_EQUALS" };		// "<="
const struct TokenType TOKEN_TYPE_GREATER_THAN_EQUALS	= { 25, "GREATER_THAN_EQUALS" };	// ">="

const struct TokenType TOKEN_TYPE_PLUS_EQUAL	= { 26, "PLUS_EQUAL" };		// "+="
const struct TokenType TOKEN_TYPE_MINUS_EQUAL	= { 27, "MINUS_EQUAL" };	// "-="
const struct TokenType TOKEN_TYPE_STAR_EQUAL	= { 28, "STAR_EQUAL" };		// "*="
const struct TokenType TOKEN_TYPE_SLASH_EQUAL	= { 29, "SLASH_EQUAL" };	// "/="
const struct TokenType TOKEN_TYPE_PERCENT_EQUAL	= { 30, "PERCENT_EQUAL" };	// "%="

const struct TokenType TOKEN_TYPE_LOGICAL_AND 	= { 31, "LOGICAL_AND" };	// "&&"
const struct TokenType TOKEN_TYPE_LOGICAL_OR  	= { 32, "LOGICAL_OR" };		// "||"

const struct TokenType TOKEN_TYPE_DASH_ARROW 	= { 33, "DASH_ARROW" };		// "->"
const struct TokenType TOKEN_TYPE_EQUAL_ARROW 	= { 34, "EQUAL_ARROW" }; 	// "=>"


// keywords
const struct TokenType TOKEN_TYPE_IMPORT 	= { 35, "IMPORT" };

const struct TokenType TOKEN_TYPE_VAR		= { 36, "VAR" };
const struct TokenType TOKEN_TYPE_VAL		= { 37, "VAL" };

const struct TokenType TOKEN_TYPE_TRUE		= { 38, "TRUE" };
const struct TokenType TOKEN_TYPE_FALSE		= { 39, "FALSE" };

const struct TokenType TOKEN_TYPE_IF		= { 40, "IF" };
const struct TokenType TOKEN_TYPE_ELSE		= { 41, "ELSE" };
const struct TokenType TOKEN_TYPE_WHEN		= { 42, "WHEN" };

const struct TokenType TOKEN_TYPE_LOOP	 	= { 43, "LOOP" };
const struct TokenType TOKEN_TYPE_REPEAT 	= { 44, "REPEAT" };

const struct TokenType TOKEN_TYPE_BREAK 	= { 45, "BREAK" };
const struct TokenType TOKEN_TYPE_CONTINUE 	= { 46, "CONTINUE" };

const struct TokenType TOKEN_TYPE_CLASS 	= { 47, "CLASS" };
const struct TokenType TOKEN_TYPE_FUN		= { 48, "FUN" };
const struct TokenType TOKEN_TYPE_RET		= { 49, "RET" };


/*
class
else
false
fun
if
import
loop
print
repeat
ret
super
this
true
var
when
*/

const struct TokenType TOKEN_TYPE_IDENTIFIER = { 50, "IDENTIFIER" };


// literals
const struct TokenType TOKEN_TYPE_INTEGER 	= { 51, "INT" };
const struct TokenType TOKEN_TYPE_DOUBLE	= { 52, "DOUBLE" };
const struct TokenType TOKEN_TYPE_STRING	= { 53, "STRING" };


// miscellaneous
const struct TokenType TOKEN_TYPE_ERROR = { 54, "ERROR" };
const struct TokenType TOKEN_TYPE_EOF   = { 55, "EOF" };



const struct TokenType TOKEN_TYPE_BITWISE_AND	= { 57, "&" }; 		// "&"
const struct TokenType TOKEN_TYPE_BITWISE_OR	= { 56, "|" }; 		// "|"
const struct TokenType TOKEN_TYPE_BITWISE_XOR	= { 58, "^" }; 		// "^"
const struct TokenType TOKEN_TYPE_BITWISE_NOT	= { 59, "~" }; 		// "~"






