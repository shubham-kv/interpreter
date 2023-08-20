#if !defined(token_type_h)
	#define token_type_h

	#include "stdint.h"

	struct TokenType {
		// an interger just for differentiation
		uint8_t ordinal;

		// string representation of the token type
		const char *text;
	};

	// symbols
	extern const struct TokenType TOKEN_TYPE_LPAREN;
	extern const struct TokenType TOKEN_TYPE_RPAREN;

	extern const struct TokenType TOKEN_TYPE_LCURLY_BRACE;
	extern const struct TokenType TOKEN_TYPE_RCURLY_BRACE;

	extern const struct TokenType TOKEN_TYPE_LSQUARE_BRACE;
	extern const struct TokenType TOKEN_TYPE_RSQUARE_BRACE;

	extern const struct TokenType TOKEN_TYPE_PLUS;
	extern const struct TokenType TOKEN_TYPE_MINUS;
	extern const struct TokenType TOKEN_TYPE_STAR;
	extern const struct TokenType TOKEN_TYPE_FORWARD_SLASH;
	extern const struct TokenType TOKEN_TYPE_PERCENT;

	extern const struct TokenType TOKEN_TYPE_EQUAL;
	extern const struct TokenType TOKEN_TYPE_NOT;

	extern const struct TokenType TOKEN_TYPE_DOT;
	extern const struct TokenType TOKEN_TYPE_COMMA;
	extern const struct TokenType TOKEN_TYPE_COLON;
	extern const struct TokenType TOKEN_TYPE_SEMICOLON;

	extern const struct TokenType TOKEN_TYPE_PLUS_PLUS;
	extern const struct TokenType TOKEN_TYPE_MINUS_MINUS;

	extern const struct TokenType TOKEN_TYPE_EQUAL_EQUAL;
	extern const struct TokenType TOKEN_TYPE_NOT_EQUAL;
	extern const struct TokenType TOKEN_TYPE_LESS_THAN;
	extern const struct TokenType TOKEN_TYPE_LESS_THAN_EQUALS;
	extern const struct TokenType TOKEN_TYPE_GREATER_THAN;
	extern const struct TokenType TOKEN_TYPE_GREATER_THAN_EQUALS;

	extern const struct TokenType TOKEN_TYPE_LOGICAL_AND;
	extern const struct TokenType TOKEN_TYPE_LOGICAL_OR;

	extern const struct TokenType TOKEN_TYPE_DASH_ARROW;
	extern const struct TokenType TOKEN_TYPE_EQUAL_ARROW;


	// keywords
	extern const struct TokenType TOKEN_TYPE_VAR;
	extern const struct TokenType TOKEN_TYPE_VAL;

	extern const struct TokenType TOKEN_TYPE_TRUE;
	extern const struct TokenType TOKEN_TYPE_FALSE;

	extern const struct TokenType TOKEN_TYPE_IF;
	extern const struct TokenType TOKEN_TYPE_ELSE;
	extern const struct TokenType TOKEN_TYPE_WHEN;

	extern const struct TokenType TOKEN_TYPE_LOOP;
	extern const struct TokenType TOKEN_TYPE_REPEAT;

	extern const struct TokenType TOKEN_TYPE_BREAK;
	extern const struct TokenType TOKEN_TYPE_CONTINUE;

	extern const struct TokenType TOKEN_TYPE_CLASS;
	// extern const struct TokenType TOKEN_TYPE_SUPER;
	// extern const struct TokenType TOKEN_TYPE_THIS;

	extern const struct TokenType TOKEN_TYPE_FUN;
	extern const struct TokenType TOKEN_TYPE_RET;

	extern const struct TokenType TOKEN_TYPE_IDENTIFIER;

	// literals
	extern const struct TokenType TOKEN_TYPE_INTEGER;
	extern const struct TokenType TOKEN_TYPE_DOUBLE;
	extern const struct TokenType TOKEN_TYPE_STRING;

	// miscellaneous
	extern const struct TokenType TOKEN_TYPE_ERROR;
	extern const struct TokenType TOKEN_TYPE_EOF;

	// temporary
	extern const struct TokenType TOKEN_TYPE_PRINT;


	extern const struct TokenType TOKEN_TYPE_PLUS_EQUAL;
	extern const struct TokenType TOKEN_TYPE_MINUS_EQUAL;
	extern const struct TokenType TOKEN_TYPE_STAR_EQUAL;
	extern const struct TokenType TOKEN_TYPE_SLASH_EQUAL;
	extern const struct TokenType TOKEN_TYPE_PERCENT_EQUAL;

	extern const struct TokenType TOKEN_TYPE_CHAR;
	extern const struct TokenType TOKEN_TYPE_LONG;

#endif // token_type_h

