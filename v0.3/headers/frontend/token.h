#if !defined(token_h)
	#define token_h

	#include <stdio.h>
	#include <stdlib.h>

	#include "token_type.h"

	struct Token {
		const void *type;

		// The type of the token
		struct TokenType token_type;

		// A pointer to the start of the lexeme of the token
		const char *start;

		// index of the first char in the line
		int start_index;

		// length of the lexeme
		int length;

		// the index of the line at which the token was found
		int line_index;
	};

	extern const void *Token;

	#define IS_TOKEN(ptr) ((ptr) && (((const struct Token *) ptr)->type == Token))

	#define TOKEN_GET_TYPE(token) 	((const struct TokenType)(((struct Token *) token)->token_type))
	#define TOKEN_GET_START(token) 	(((const struct Token *) token)->start)
	#define TOKEN_GET_LENGTH(token) (((const struct Token *) token)->length)

	void token_printer(const void *self, FILE *stream);

#endif // token_h

