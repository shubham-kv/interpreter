#if !defined(lexer_h)
	#define lexer_h

	#include <stdbool.h>
	#include "token.h"

	struct Lexer {
		const void *type;

		// a pointer to the Source
		const void *source;

		uint64_t lines_count;
		const char *const *lines;

		// index of the line being scanned
		int line_index;

		// index of the character being scanned
		int char_index;

		// the total no. of tokens scanned
		uint64_t tokens_count;

		// the tokens dynamic array
		const void **tokens;

		bool had_error;
	};

	extern const void *Lexer;

	#define IS_LEXER(ptr) ((ptr) && (((const struct Lexer *) ptr)->type == Lexer))
	#define LEXER_GET_TOKENS(lexer) (((const struct Lexer *) lexer)->tokens)
	#define LEXER_TOKENS_COUNT(lexer) (((const struct Lexer *) lexer)->tokens_count)
	#define LEXER_HAD_ERROR(lexer) 	(((const struct Lexer *) lexer)->had_error)

	void scan(void *lexer);

#endif // lexer_h
