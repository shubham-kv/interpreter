#include "my_lexer.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>

#include "type.h"
#include "object.h"
#include "memory.h"

#include "token_type.h"
#include "token.h"
#include "parse_err_handler.h"


#define TOKENS_STEP_SIZE 32




static void *lexer_constructor(void *self, va_list* ap);
static void *lexer_destructor(void *self);

static const struct Type _type_lexer = {
	sizeof(struct Lexer),
	lexer_constructor,
	lexer_destructor
};

const void *Lexer = &_type_lexer;


// ... = new(Lexer, lines);

static void *lexer_constructor(void *_self, va_list* ap) {
	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	const char *const *lines = va_arg(*ap, const char *const *);
	assert(lines);

	self->lines = lines;

	self->cur_line_index = 0;
	self->cur_char_index = 0;

	self->tokens_size = 0;
	self->tokens = NULL;

	self->had_error = false;

	return _self;
}

static void *lexer_destructor(void *_self) {
	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	for(uint32_t i = 0; NULL != self->tokens[i]; i++) {
		delete((void *) self->tokens[i]);
	}

	free(self->tokens);

	return _self;
}




static bool l_is_alpha(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static bool l_is_digit(char c) {
	return (c >= '0' && c <= '9');
}

static bool l_is_alpha_numeric(char c) {
	return l_is_alpha(c) || l_is_digit(c) || (c == '_');
}



static const char *cur_char_ptr(const struct Lexer *self) {
	const char *cur_line = self->lines[self->cur_line_index];
	return (NULL == cur_line) ? NULL : (cur_line + self->cur_char_index);
}


static bool l_has_chars(const struct Lexer *self) {
	return (cur_char_ptr(self) != NULL);
}

static char l_peek(const struct Lexer *self) {
	const char *ch = cur_char_ptr(self);
	return (NULL == ch) ? ('\0') : (*ch);
}

static char l_peek_next(const struct Lexer *self) {
	const char *cur_line = self->lines[self->cur_line_index];
	if(NULL == cur_line) return '\0';

	int i = self->cur_char_index;

	if((i + 1) < strlen(cur_line)) {
		i = i + 1;
	} 
	else {
		cur_line = self->lines[self->cur_line_index + 1];
		i = 0;

		if(NULL == cur_line) return '\0';
	}

	return cur_line[i];
}

static char l_advance(struct Lexer *self) {
	const char *cur_line = self->lines[self->cur_line_index];
	if(NULL == cur_line) return '\0';

	int i = self->cur_char_index;

	if(i < (strlen(cur_line) - 1)) {
		self->cur_char_index++;
	} else {
		self->cur_line_index++;
		self->cur_char_index = 0;
	}

	return cur_line[i];
}

#define L_MATCH(lexer, e) (l_peek(lexer) == e)

static bool l_match_any(const struct Lexer *self, int count, ...) {
	va_list ap;
	va_start(ap, count);

	for(int i = 0; i < count; i++) {
		char e = (char) va_arg(ap, int);

		if(l_peek(self) == e) {
			va_end(ap);
			return true;
		}
	}

	va_end(ap);
	return false;
}

static bool l_consume(struct Lexer *self, const char e) {
	if(L_MATCH(self, e)) {
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

	fprintf(stderr, "\n");
	vfprintf(stderr, err_msg, ap);
	fprintf(stderr, "\n");
	va_end(ap);

	fprintf(stderr, "-%03d-| ", line_number);

	for(uint32_t i = 0; err_line[i] != '\0'; i++) {
		char c = err_line[i];
		
		if('\t' == c)
			c = ' ';

		fprintf(stderr, "%c", c);
	}

	// err_pos is assumed to be index
	for(uint32_t i = 0; i < (err_pos + 7); i++)
		fprintf(stderr, " ");

	for(uint32_t i = 0; i < err_length; i++)
		fprintf(stderr, "^");
	
	fprintf(stderr, "\n\n");
}










// skips characters until the next space character
static void skip_chars(struct Lexer *self) {
	while(!l_match_any(self, 4, ' ', '\t', '\r', '\n'))
		l_advance(self);
}



#define ERR_TOKEN(lexer) (new(Token, TOKEN_TYPE_ERROR, cur_char_ptr(self), self->cur_char_index, 1, self->cur_line_index))


static struct Token *make_number_token(struct Lexer *self) {
	assert(self);

	const char *start = cur_char_ptr(self);
 	int start_index = self->cur_char_index;
	int line_index = self->cur_line_index;

	struct TokenType token_type = TOKEN_TYPE_INTEGER;

	// bcuz first char is already checked and matched
	l_advance(self);

	while(l_is_digit(l_peek(self))) l_advance(self);

	int length = self->cur_char_index - start_index;

	if(l_consume(self, '.')) {
		while(l_is_digit(l_peek(self)))
			l_advance(self);

		token_type = TOKEN_TYPE_DOUBLE;
		length = self->cur_char_index - start_index;
	}

	// if(!l_match_any(self, 4, ' ', '\t', '\r', '\n')) {
	// if(!l_match_any(self, 17, ' ', '\t', '\r', '\n', '+', '-', '*', '/', '%', ',', '(', ')', '[', ']', '{', '}', ';')) {
	// 	l_flag_error(
	// 		(line_index + 1),
	// 		self->lines[line_index],
	// 		start_index,
	//		length,
	// 		"SyntaxError: Invalid Number literal at line %d.", (line_index + 1)
	// 	);
	// 	self->had_error = true;
	// 	void *err_token = ERR_TOKEN(self);
	// 	// skip characters until the next space char
	// 	skip_chars(self);
	// 	return err_token;
	// }

	return new(Token, token_type, start, start_index, length, line_index);
}


static struct Token *make_string_token(struct Lexer *self) {
	const char *start = cur_char_ptr(self);
 	int start_index = self->cur_char_index;
	int line_index = self->cur_line_index;

	// consume the first '|" char
	l_advance(self);

	while(!l_match_any(self, 3, *start, '\n', '\0')) {
		l_advance(self);
	}

	if(l_consume(self, *start)) {
		int cur = self->cur_char_index;
		return new(Token, TOKEN_TYPE_STRING, start, start_index, (cur - start_index), line_index);
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

	void *err_token = ERR_TOKEN(self);
	skip_chars(self);

	return err_token;
}


static struct Token *make_word_token(struct Lexer *self) {
	const char *start = cur_char_ptr(self);
 	int start_index = self->cur_char_index;
	int line_index = self->cur_line_index;

	bool is_keyword = true;

	// bcuz first char is already checked and matched
	l_advance(self);

	while(1) {
		if(l_is_alpha(l_peek(self)) || L_MATCH(self, '_')) {
			l_advance(self);
			continue;
		}
		else if(l_is_digit(l_peek(self))) {
			is_keyword = false;
			l_advance(self);
			continue;
		}
		else {
			break;
		}
	}
	// while(l_is_alpha_numeric(l_peek(self))) l_advance(self);
	// if(!l_match_any(self, 12, ' ', '\t', '\r', '\n', ',', '(', ')', '[', ']', '{', '}', ';')) {
	// 	l_flag_error(
	// 		(line_index + 1),
	// 		self->lines[line_index],
	// 		start_index,
	// 		"SyntaxError: Invalid identifier at line %d.", (line_index + 1)
	// 	);
	// 	self->had_error = true;
	// 	void *err_token = ERR_TOKEN(self);
	// 	// consume the character because of which this was an err token
	// 	l_advance(self);
	// 	// skip characters until the next space char
	// 	skip_chars(self);
	// 	return err_token;
	// }

	int token_len = self->cur_char_index - start_index;

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
			break;
		}

		case 'l': {
			if((token_len == 4) && (strncmp(start + 1, "oop", 3) == 0)) {
				return new(Token, TOKEN_TYPE_LOOP, start, start_index, token_len, line_index);
			}
			break;
		}

		case 'p': {
			if((token_len == 5) && (strncmp(start + 1, "rint", 4) == 0)) {
				return new(Token, TOKEN_TYPE_PRINT, start, start_index, token_len, line_index);
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

		case 's': {
			if((token_len == 5) && (strncmp(start + 1, "uper", 4) == 0)) {
				return new(Token, TOKEN_TYPE_SUPER, start, start_index, token_len, line_index);
			}
			break;
		}

		case 't': {
			if(token_len == 4) {
				switch(start[1]) {
					case 'h': {
						if(strncmp(start + 2, "is", 2) == 0) {
							return new(Token, TOKEN_TYPE_THIS, start, start_index, token_len, line_index);
						}
						break;
					}

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
	assert(self);

	while(!l_match_any(self, 2, '*', '\0')) l_advance(self);

	if(l_consume(self, '*')) {
		if(l_consume(self, '/')) {
			return true;
		}
		return skip_multi_line_comments(self);
	}

	return false;
}



static void skip_spaces(struct Lexer *self) {
	assert(self);

	char c;

	while(1) {
		c = l_peek(self);

		switch(c) {
			case ' ':
			case '\r':
			case '\t': {
				l_advance(self);
				break;
			}

			case '\n': {
				self->cur_line_index++;
				self->cur_char_index = 0;
				break;
			}

			case '/': {
				if(l_peek_next(self) == '/') {
					l_advance(self);	// consumes the 1st '/'
					l_advance(self);	// consumes the 2nd '/'

					while(!L_MATCH(self, '\n')) l_advance(self);
				}
				else if(l_peek_next(self) == '*') {
					int start_pos = self->cur_char_index;
					l_advance(self);
					l_advance(self);

					int line = self->cur_line_index;

					if (!skip_multi_line_comments(self)) {
						l_flag_error(
							line + 1,
							self->lines[line],
							start_pos,
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
	const char *start = cur_char_ptr(self);
	int start_index = self->cur_char_index;
	int line_index = self->cur_line_index;

	void *token = new(Token, type, start, start_index, token_len, line_index);

	for(uint32_t i = 0; i < token_len; i++)
		l_advance(self);

	return token;
}



static void *next_token(void *_self) {
	struct Lexer *self = (struct Lexer *)_self;
	assert(IS_LEXER(self));

	skip_spaces(self);

	if(!l_has_chars(self)) {
		return new(Token, TOKEN_TYPE_EOF, NULL, 0, 0, self->cur_line_index);
	}

	char c = l_peek(self);

	if(l_is_digit(c))
		return make_number_token(self);

	if(l_is_alpha_numeric(c))
		return make_word_token(self);
	

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
			if(l_peek_next(self) == '+') {
				return make_token(self, TOKEN_TYPE_PLUS_PLUS, 2);
			}
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_PLUS_EQUAL, 2);
			}
			return make_token(self, TOKEN_TYPE_PLUS, 1);
		}
		case '-': {
			if(l_peek_next(self) == '-') {
				return make_token(self, TOKEN_TYPE_MINUS_MINUS, 2);
			}
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_MINUS_EQUAL, 2);
			}
			if(l_peek_next(self) == '>') {
				return make_token(self, TOKEN_TYPE_DASH_ARROW, 2);
			}
			return make_token(self, TOKEN_TYPE_MINUS, 1);
		}

		case '*': {
			if(l_peek_next(self) == '=') {
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
			if(l_peek_next(self) == '=') {
				return make_token(self, TOKEN_TYPE_EQUAL_EQUAL, 2);
			}
			if(l_peek_next(self) == '>') {
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

			int line = self->cur_line_index;

			// raise an error
			l_flag_error(
				(line + 1),
				self->lines[line],
				self->cur_char_index,
				1,
				"SyntaxError: Unexpecetd character '%c' at line %d.", l_peek(self), (line + 1)
			);
			self->had_error = true;

			void *err_token = ERR_TOKEN(self);
			skip_chars(self);

			return err_token;
		}

		case '|': {
			if(l_peek_next(self) == '|') {
				return make_token(self, TOKEN_TYPE_LOGICAL_OR, 2);
			}

			int line = self->cur_line_index;

			l_flag_error(
				(line + 1),
				self->lines[line],
				self->cur_char_index,
				1,
				"SyntaxError: Unexpecetd character '%c' at line %d.", l_peek(self), (line + 1)
			);
			self->had_error = true;

			void *err_token = ERR_TOKEN(self);
			skip_chars(self);

			return err_token;
		}

		default: {
			int line = self->cur_line_index;
			int i = self->cur_char_index;

			l_flag_error(
				(line + 1),
				self->lines[line],
				i,
				1,
				"SyntaxError: Unexpecetd character '%c' at line %d.", self->lines[line][i], (line + 1)
			);
			self->had_error = true;

			void *err_token = ERR_TOKEN(self);
			skip_chars(self);

			return err_token;
		}
	}
}




void scan(void *_self) {
	// time_t start_time;
	// time(&start_time);

	// struct tm *start_tm = localtime(&start_time);

	struct Lexer *self = _self;
	assert(IS_LEXER(self));

	uint32_t i = 0;

	while(1) {
		if(i == self->tokens_size) {
			self->tokens_size += TOKENS_STEP_SIZE;
			self->tokens = reallocate(self->tokens, sizeof(void *), self->tokens_size);
		}

		const struct Token *token = next_token(self);
		self->tokens[i++] = token;

		if(token->token_type.ordinal == TOKEN_TYPE_EOF.ordinal)
			break;
	}

	if(i == self->tokens_size)
		self->tokens = reallocate(self->tokens, sizeof(void *), ++self->tokens_size);

	self->tokens[i] = NULL;

	// time_t end_time;
	// time(&end_time);

	// printf("Total Scanning time: %fs\n", difftime(end_time, start_time));
}



