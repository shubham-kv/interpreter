#include "my_lexer.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <windows.h>

#include "my_time.h"
#include "colors.h"
#include "log.h"
#include "type.h"
#include "object.h"
#include "my_memory.h"

#include "token_type.h"
#include "token.h"
#include "my_source.h"

#define TOKENS_STEP_SIZE 32


static void *lexer_constructor(void *self, va_list* ap);
static void *lexer_destructor(void *self);

static const struct Type _type_lexer = {
	.size = sizeof(struct Lexer),
	.constructor = lexer_constructor,
	.destructor = lexer_destructor
};

const void *Lexer = &_type_lexer;


// ... = new(Lexer, source);

static void *lexer_constructor(void *_self, va_list* ap) {
	#if defined(DEBUG)
		vi_log("lexer_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	self->source = va_arg(*ap, const void *);
	assert(IS_SOURCE(self->source));

	self->lines_count = SOURCE_LINES_COUNT(self->source);
	self->lines = (const char *const *) SOURCE_LINES(self->source);

	self->line_index = 0;
	self->char_index = 0;
	self->tokens = NULL;
	self->tokens_count = 0;
	self->had_error = false;

	#if defined(DEBUG)
		vi_log("ret lexer_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

static void *lexer_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("lexer_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	for(int i = 0; i < self->tokens_count; i++) {
		delete((void *) self->tokens[i]);
	}
	free(self->tokens);

	#if defined(DEBUG)
		vi_log("ret lexer_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


#define IS_ALPHA(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
#define IS_DIGIT(c) ((c >= '0' && c <= '9'))
#define IS_ALPHA_NUMERIC(c) ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || (c == '_'))
#define IS_EOF(c) (c == '\0')

#define CUR_LINE(lexer) \
	((lexer->line_index < lexer->lines_count) ? (lexer->lines[lexer->line_index]) : (NULL))

#define NEXT_LINE(lexer) \
	((lexer->line_index < (lexer->lines_count - 1)) ? (lexer->lines[lexer->line_index + 1]) : (NULL))

#define CUR_CHAR(lexer) \
	((lexer->line_index < lexer->lines_count) ? (lexer->lines[lexer->line_index] + lexer->char_index) : (NULL))

#define PEEK(lexer) \
	((lexer->line_index < lexer->lines_count) ? (lexer->lines[lexer->line_index][lexer->char_index]) : ('\0'))

#define MATCH(lexer, e) \
	(((lexer->line_index < lexer->lines_count) ? (lexer->lines[lexer->line_index][lexer->char_index]) : ('\0')) == (e))



static char l_peek_next(const struct Lexer *self) {
	assert(IS_LEXER(self));

	const char *line = CUR_LINE(self);

	if(NULL == line) {
		return '\0';
	}

	if(self->char_index < (strlen(line) + 1)) {
		return line[self->char_index + 1];;
	}
	else {
		line = NEXT_LINE(self);
		return (line) ? line[0] : '\0';
	}
}


static void l_advance(struct Lexer *self) {
	assert(IS_LEXER(self));

	if(self->line_index < self->lines_count) {
		const char *cur_line = self->lines[self->line_index];

		if(self->char_index < (strlen(cur_line) - 1)) {
			self->char_index++;
		}
		else {
			self->line_index++;
			self->char_index = 0;
		}
	}
}


static bool l_consume(struct Lexer *self, const char e) {
	if(MATCH(self, e)) {
		l_advance(self);
		return true;
	}

	return false;
}


void l_flag_error(
	const int line_number,
	const char *err_line, uint32_t err_pos, uint32_t err_length,
	const char *err_msg, ...
) {
	va_list ap;
	va_start(ap, err_msg);

	#if defined(_WIN32)
		HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
		GetConsoleScreenBufferInfo(consoleHandle, &consoleInfo);
		WORD savedAttr = consoleInfo.wAttributes;
		SetConsoleTextAttribute(consoleHandle, FOREGROUND_RED);
	#else
		fprintf(stderr, COLOR_RED);
	#endif

	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	fprintf(stderr, "-%03d-| ", line_number);

	for(int i = 0; err_line[i] != '\0'; i++) {
		char c = err_line[i];
		
		if('\t' == c)
			c = ' ';

		fprintf(stderr, "%c", c);
	}

	// err_pos is assumed to be index
	for(int i = 0; i < (err_pos + 7); i++)
		fprintf(stderr, " ");

	for(int i = 0; i < err_length; i++)
		fprintf(stderr, "^");
	
	fprintf(stderr, "\n\n");

	#if defined(_WIN32)
		SetConsoleTextAttribute(consoleHandle, savedAttr);
	#else
		fprintf(stderr, COLOR_RESET);
	#endif
}










// skips characters until the next space character
static void skip_chars(struct Lexer *self) {
	assert(IS_LEXER(self));

	while(1) {
		char c = PEEK(self);

		if((c == ' ') || (c == '\t') || (c == '\n')) {
			l_advance(self);
		} else {
			break;
		}
	}
}



static struct Token *make_number_token(struct Lexer *self) {
	assert(IS_LEXER(self));

	const char *start 	= CUR_CHAR(self);
 	int start_index 	= self->char_index;
	int line_index 		= self->line_index;

	struct TokenType token_type = TOKEN_TYPE_INTEGER;

	// bcuz first char is already checked and matched
	l_advance(self);

	// consume the digits
	while(1) {
		char c = PEEK(self);

		if(IS_DIGIT(c)) {
			l_advance(self);
		} else {
			break;
		}
	}

	int length = self->char_index - start_index;

	if(l_consume(self, '.')) {

		// consume the digits
		while(1) {
			char c = PEEK(self);

			if(IS_DIGIT(c)) {
				l_advance(self);
			} else {
				break;
			}
		}

		token_type = TOKEN_TYPE_DOUBLE;
		length = self->char_index - start_index;
	}

	return new(Token, token_type, start, start_index, length, line_index);
}


static struct Token *make_string_token(struct Lexer *self) {
	const char *start = CUR_CHAR(self);
 	int start_index = self->char_index;
	int line_index = self->line_index;

	// consume the starting quote char
	l_advance(self);

	while(1) {
		char c = PEEK(self);

		// found an escape sequence
		if(c == '\\') {
			// consume the first '\'
			l_advance(self);

			// consume the next escape sequence char
			l_advance(self);
			continue;
		}

		if((c == *start) || (c == '\n') || (c == '\0')) {
			break;
		}
		else {
			l_advance(self);
		}
	}

	if(l_consume(self, *start)) {
		return new(Token, TOKEN_TYPE_STRING, start, start_index, (self->char_index - start_index), line_index);
	}

	// raise error otherwise
	l_flag_error(
		(line_index + 1),
		self->lines[line_index],
		start_index,
		1,
		"SyntaxError: Unterminated string at line %d.", (line_index + 1)
	);

	self->had_error = true;

	void *err_token = new(Token, TOKEN_TYPE_ERROR, start, start_index, 1, line_index);
	skip_chars(self);

	return err_token;
}


static struct Token *make_word_token(struct Lexer *self) {
	const char *start = CUR_CHAR(self);
 	int start_index = self->char_index;
	int line_index = self->line_index;

	bool is_keyword = true;

	// bcuz first char is already checked and matched
	l_advance(self);

	while(1) {
		char c = PEEK(self);

		if(IS_ALPHA(c) || (c == '_')) {
			l_advance(self);
		}
		else if(IS_DIGIT(c)) {
			is_keyword = false;
			l_advance(self);
		}
		else {
			break;
		}
	}

	int token_len = self->char_index - start_index;

	if(!is_keyword) {
		return new(Token, TOKEN_TYPE_IDENTIFIER, start, start_index, token_len, line_index);
	}

	switch(start[0]) {
		case 'b': {
			if(token_len == 5 && (strncmp(start + 1, "reak", 4) == 0)) {
				return new(Token, TOKEN_TYPE_BREAK, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'c': {
			if((token_len == 5) && (strncmp(start + 1, "lass", 4) == 0)) {
				return new(Token, TOKEN_TYPE_CLASS, start, start_index, token_len, line_index);
			}
			else if((token_len == 8) && (strncmp(start + 1, "ontinue", 7) == 0)) {
				return new(Token, TOKEN_TYPE_CONTINUE, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'e': {
			if((token_len == 4) && (strncmp(start + 1, "lse", 3) == 0)) {
				return new(Token, TOKEN_TYPE_ELSE, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'f': {
			if((token_len == 3) || (token_len == 5)) {
				switch(start[1]) {
					case 'a': {
						if((token_len == 5) && (strncmp(start + 2, "lse", 3) == 0))	{
							return new(Token, TOKEN_TYPE_FALSE, start, start_index, token_len, line_index);
						}
						break;
					}

					case 'u': {
						if((token_len == 3) && (start[2] == 'n')) {
							return new(Token, TOKEN_TYPE_FUN, start, start_index, token_len, line_index);
						}
						break;
					}
				}
			}
			break;
		}

		case 'i': {
			if((token_len == 2) && (start[1] == 'f')) {
				return new(Token, TOKEN_TYPE_IF, start, start_index, token_len, line_index);
			}
			if((token_len == 6) && (strncmp(start + 1, "mport", 5) == 0)) {
				return new(Token, TOKEN_TYPE_IMPORT, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'l': {
			if((token_len == 4) && (strncmp(start + 1, "oop", 3) == 0)) {
				return new(Token, TOKEN_TYPE_LOOP, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'r': {
			if((start[1] == 'e') && (token_len == 6 || token_len == 3)) {
				switch(start[2]) {
					case 't': {
						if(token_len == 3) {
							return new(Token, TOKEN_TYPE_RET, start, start_index, token_len, line_index);
						}
						break;
					}

					case 'p': {
						if((token_len == 6) && (strncmp(start + 3, "eat", 3) == 0)) {
							return new(Token, TOKEN_TYPE_REPEAT, start, start_index, token_len, line_index);
						}
						break;
					}
				}
			}
			break;
		}

		// case 's': {
		// 	if((token_len == 5) && (strncmp(start + 1, "uper", 4) == 0)) {
		// 		return new(Token, TOKEN_TYPE_SUPER, start, start_index, token_len, line_index);
		// 	}
		// 	break;
		// }

		case 't': {
			if(token_len == 4) {
				switch(start[1]) {
					// case 'h': {
					// 	if(strncmp(start + 2, "is", 2) == 0) {
					// 		return new(Token, TOKEN_TYPE_THIS, start, start_index, token_len, line_index);
					// 	}
					// 	break;
					// }

					case 'r': {
						if(strncmp(start + 2, "ue", 2) == 0) {
							return new(Token, TOKEN_TYPE_TRUE, start, start_index, token_len, line_index);
						}
						break;
					}
				}
			}
			break;
		}

		case 'v': {
			if((token_len == 3) && (strncmp(start + 1, "ar", 2) == 0)) {
				return new(Token, TOKEN_TYPE_VAR, start, start_index, token_len, line_index);
			}
			if((token_len == 3) && (strncmp(start + 1, "al", 2) == 0)) {
				return new(Token, TOKEN_TYPE_VAL, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'w': {
			if((token_len == 4) && (strncmp(start + 1, "hen", 3) == 0)) {
				return new(Token, TOKEN_TYPE_WHEN, start, start_index, token_len, line_index);
			}
			break;
		}
	}

	return new(Token, TOKEN_TYPE_IDENTIFIER, start, start_index, token_len, line_index);
}



// returns boolean based on whether the comment was skipped successfully
static bool skip_multi_line_comments(struct Lexer *self) {
	assert(IS_LEXER(self));

	while(1) {
		char c = PEEK(self);

		if(c == '*') {
			l_advance(self);

			c = PEEK(self);
			if(c == '/') {
				l_advance(self);
				return true;
			}
		}
		else if(c == '\0') {
			return false;
		}
		else {
			l_advance(self);
		}
	}

	// if(l_consume(self, '*')) {
	// 	if(l_consume(self, '/')) {
	// 		return true;
	// 	}
	// 	return skip_multi_line_comments(self);
	// }

	return false;
}


static void skip_spaces(struct Lexer *self) {
	assert(IS_LEXER(self));

	while(1) {
		char c = PEEK(self);

		switch(c) {
			case ' ':
			case '\r':
			case '\t': {
				l_advance(self);
				break;
			}

			case '\n': {
				self->line_index++;
				self->char_index = 0;
				break;
			}

			case '/': {
				char next_char = l_peek_next(self);

				if('/' == next_char) {
					l_advance(self);	// consumes the 1st '/'
					l_advance(self);	// consumes the 2nd '/'

					while(!MATCH(self, '\n')) {
						l_advance(self);
					}
				}
				else if('*' == next_char) {
					int start_index = self->char_index;
					int line_index = self->line_index;

					l_advance(self);	// consumes the 1st '/'
					l_advance(self);	// consumes the 2nd '*'

					if (!skip_multi_line_comments(self)) {
						l_flag_error(
							line_index + 1,
							self->lines[line_index],
							start_index,
							2,
							"SyntaxError: Unterminated multi-line comment.\n"
						);
						self->had_error = true;
					}
				}
				else {
					// we can't skip that '/', so return
					return;
				}
				break;
			}

			default: {
				// we got to scan other characters, so return
				return;
			}
		}
	}
}


static struct Token *make_token(struct Lexer *self, struct TokenType type, int token_len) {
	const char *start = CUR_CHAR(self);
	int start_index = self->char_index;
	int line_index = self->line_index;

	void *token = new(Token, type, start, start_index, token_len, line_index);

	for(uint32_t i = 0; i < token_len; i++) {
		l_advance(self);
	}

	return token;
}



static void *next_token(void *_self) {
	struct Lexer *self = (struct Lexer *)_self;
	assert(IS_LEXER(self));

	skip_spaces(self);

	char c = PEEK(self);

	if(IS_EOF(c)) {
		return new(Token, TOKEN_TYPE_EOF, NULL, 0, 0, self->line_index);
	}
	if(IS_DIGIT(c)) {
		return make_number_token(self);
	}
	if(IS_ALPHA_NUMERIC(c)) {
		return make_word_token(self);
	}

	switch(c) {
		case '"':
		case '\'':
			return make_string_token(self);

		case '(':
			return make_token(self, TOKEN_TYPE_LPAREN, 1);
	
		case ')':
			return make_token(self, TOKEN_TYPE_RPAREN, 1);
		
		case '{':
			return make_token(self, TOKEN_TYPE_LCURLY_BRACE, 1);
		
		case '}':
			return make_token(self, TOKEN_TYPE_RCURLY_BRACE, 1);
		
		case '[':
			return make_token(self, TOKEN_TYPE_LSQUARE_BRACE, 1);
		
		case ']':
			return make_token(self, TOKEN_TYPE_RSQUARE_BRACE, 1);
		
		case '+': {
			char next_char = l_peek_next(self);

			if('+' == next_char) {
				return make_token(self, TOKEN_TYPE_PLUS_PLUS, 2);
			}
			if('=' == next_char) {
				return make_token(self, TOKEN_TYPE_PLUS_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_PLUS, 1);
		}
		case '-': {
			char next_char = l_peek_next(self);

			if('-' == next_char) {
				return make_token(self, TOKEN_TYPE_MINUS_MINUS, 2);
			}
			if('=' == next_char) {
				return make_token(self, TOKEN_TYPE_MINUS_EQUAL, 2);
			}
			if('>' == next_char) {
				return make_token(self, TOKEN_TYPE_DASH_ARROW, 2);
			}
			return make_token(self, TOKEN_TYPE_MINUS, 1);
		}
		case '*': {
			char next_char = l_peek_next(self);

			if('=' == next_char) {
				return make_token(self, TOKEN_TYPE_STAR_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_STAR, 1);
		}
		case '/': {
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_SLASH_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_FORWARD_SLASH, 1);
		}
		case '%': {
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_PERCENT_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_PERCENT, 1);
		}

		case '=': {
			char next_char = l_peek_next(self);

			if('=' == next_char) {
				return make_token(self, TOKEN_TYPE_EQUAL_EQUAL, 2);
			}
			if('>' == next_char) {
				return make_token(self, TOKEN_TYPE_EQUAL_ARROW, 2);
			}
			return make_token(self, TOKEN_TYPE_EQUAL, 1);
		}

		case '!': {
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_NOT_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_NOT, 1);
		}

		case '<': {
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_LESS_THAN_EQUALS, 2);
			}
			return make_token(self, TOKEN_TYPE_LESS_THAN, 1);
		}

		case '>': {
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_GREATER_THAN_EQUALS, 2);
			}
			return make_token(self, TOKEN_TYPE_GREATER_THAN, 1);
		}
		
		case '.': return make_token(self, TOKEN_TYPE_DOT, 1);
		case ',': return make_token(self, TOKEN_TYPE_COMMA, 1);
		case ':': return make_token(self, TOKEN_TYPE_COLON, 1);
		case ';': return make_token(self, TOKEN_TYPE_SEMICOLON, 1);

		case '&': {
			if(l_peek_next(self) == '&') {
				return make_token(self, TOKEN_TYPE_LOGICAL_AND, 2);
			}
			else {
				l_flag_error(
					self->line_index + 1,
					self->lines[self->line_index],
					self->char_index, 1,
					"SyntaxError: Unexpecetd character '%c' at line %d.", c, (self->line_index + 1)
				);
				self->had_error = true;

				void *err_token = new(Token, TOKEN_TYPE_ERROR, CUR_CHAR(self), self->char_index, 1, self->line_index);
				skip_chars(self);

				return err_token;
			}			
		}

		case '|': {
			if(l_peek_next(self) == '|') {
				return make_token(self, TOKEN_TYPE_LOGICAL_OR, 2);
			}
			else {
				int line_number = self->line_index + 1;

				l_flag_error(
					line_number,
					self->lines[self->line_index],
					self->char_index, 1,
					"SyntaxError: Unexpecetd character '%c' at line %d.", c, line_number
				);
				self->had_error = true;

				void *err_token = new(Token, TOKEN_TYPE_ERROR, CUR_CHAR(self), self->char_index, 1, self->line_index);
				skip_chars(self);

				return err_token;				
			}
		}

		default: {
			int line_number = self->line_index + 1;

			l_flag_error(
				line_number,
				self->lines[self->line_index],
				self->char_index, 1,
				"SyntaxError: Unexpecetd character '%c' at line %d.", c, line_number
			);
			self->had_error = true;

			void *err_token = new(Token, TOKEN_TYPE_ERROR, CUR_CHAR(self), self->char_index, 1, self->line_index);
			skip_chars(self);

			return err_token;
		}
	}
}




void scan(void *_self) {
	/*
	struct timespec start_time, end_time, delta_time;
	// store current timespec in start_time
	clock_gettime(CLOCK_REALTIME, &start_time);
	*/


	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	void **tokens = NULL;
	uint64_t tokens_count = 0;
	uint64_t tokens_size = 0;

	while(1) {
		if(tokens_count == tokens_size) {
			tokens_size += TOKENS_STEP_SIZE;
			tokens = reallocate(tokens, sizeof(void *), tokens_size);
		}

		struct Token *token = (struct Token *) next_token(self);
		tokens[tokens_count++] = token;

		const uint32_t ordinal = token->token_type.ordinal;

		if(
			(ordinal == TOKEN_TYPE_EOF.ordinal) ||
			(ordinal == TOKEN_TYPE_ERROR.ordinal)
		) {
			break;
		}
	}

	if(tokens_count < tokens_size) {
		// free the unused memory
		tokens = reallocate(tokens, sizeof(void *), tokens_count);
	}

	self->tokens = (const void **) tokens;
	self->tokens_count = tokens_count;


	/*
	// store current timespec in end_time
	clock_gettime(CLOCK_REALTIME, &end_time);

	// calculate time difference
	calc_delta_time(&start_time, &end_time, &delta_time);

	// print the formatted time_string
	char time_string[64];
	put_formatted_time(time_string, &delta_time, 64);

	puts("");
	printf("|====================>\n");
	printf("| Lexer Summary\n");
	printf("|====================>\n");
	printf("| File: '%s'\n", SOURCE_INSTREAM(self->source));
	printf("| Total Scanning time: '%s'\n", time_string);
	printf("| Start time.: %lds %.9ldns\n", (int) start_time.tv_sec, start_time.tv_nsec);
	printf("| End time...: %lds %.9ldns\n", (int) end_time.tv_sec, end_time.tv_nsec);
	printf("| Delta time.: %lds %.9ldns\n", (int) delta_time.tv_sec, delta_time.tv_nsec);
	printf("|========================================>\n\n");
	*/
}

