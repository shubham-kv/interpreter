#if !defined(token_h)
	#define token_h

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

	#define IS_TOKEN(obj) ((obj) && (((struct Token *) obj)->type == Token))
	#define TOKEN_GET_TYPE(token) ((struct TokenType)(((struct Token *) token)->token_type))
	#define TOKEN_GET_START(token) (((struct Token *) token)->start)
	#define TOKEN_GET_LENGTH(token) (((struct Token *) token)->length)

	void print_token(const void *token);

#endif // token_h
