#if !defined(lexer_h)
	#define lexer_h

	#include <stdbool.h>
	#include "token.h"

	struct Lexer {
		const void *type;

		const char *const *lines;

		int cur_line_index;

		int cur_char_index;

		// the current capacity of the the tokens dynamic array
		int tokens_size;

		// array of pointer to tokens
		const void **tokens;

		bool had_error;
	};

	extern const void *Lexer;

	#define IS_LEXER(obj) ((obj) && (((struct Lexer *) obj)->type == Lexer))

	#define LEXER_GET_LINES(lexer) (((struct Lexer *) lexer)->lines)
	#define LEXER_GET_LINE(lexer, line_index) (((struct Lexer *) lexer)->lines[line_index])

	#define LEXER_GET_TOKENS(lexer) (((struct Lexer *) lexer)->tokens)

	#define LEXER_HAD_ERROR(lexer) (((struct Lexer *) lexer)->had_error)

	void scan(void *lexer);

#endif // lexer_h
