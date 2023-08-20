#include "token.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "token_type.h"
#include "type.h"


static void *token_constructor(void *self, va_list *ap);
static void *token_destructor(void *self);
static void token_printer(const void *self, FILE *stream);


static const struct Type _type_token = {
	.size = sizeof(struct Token),
	.constructor = token_constructor,
	.destructor = token_destructor,

	#if defined(DEBUG)
		.type_printer = token_printer
	#endif // DEBUG
};

const void *Token = &_type_token;

static void *token_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("token_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Token *self = _self;
	assert(IS_TOKEN(self));

	self->token_type 	= va_arg(*ap, struct TokenType);
	self->start 		= va_arg(*ap, const char *);
	self->start_index 	= va_arg(*ap, int);
	self->length 		= va_arg(*ap, int);
	self->line_index 	= va_arg(*ap, int);

	#if defined(DEBUG)
		vi_log("ret token_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

static void *token_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("c&r token_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


/*
<PRINT, 1> <INT, 12, 1> <ADD> <DOUBLE, 3.14>
<EOF>

Token {
	at: 0x787aff,
	token_type: VAR,
	lexeme: 'var',
	start: 'v', length: 3,
	line_number: 3
}

*/

static void token_printer(const void *_self, FILE *stream) {
	const struct Token *self = _self;
	assert(IS_TOKEN(self));

	if(self->token_type.ordinal == TOKEN_TYPE_EOF.ordinal) {
		fprintf(stream, "<EOF>\n");
		return;
	}

	if(self->token_type.ordinal == TOKEN_TYPE_STRING.ordinal) {
		fprintf(stream, 
			"<%s, '%.*s', %d>\n",
			self->token_type.text,
			(self->length - 2),
			(self->start + 1),
			(self->line_index + 1) 
		);
		return;
	}

	fprintf(stream, 
		"<%s, '%.*s', %d>\n",
		self->token_type.text,
		self->length, self->start,
		(self->line_index + 1)
	);	
}


/*
void print_token(const void *_self, FILE *stream) {
	const struct Token *self = _self;
	assert(IS_TOKEN(self));

	if(self->token_type.ordinal == TOKEN_TYPE_EOF.ordinal) {
		fprintf(stream, "<EOF>\n");
		return;
	}

	if(self->token_type.ordinal == TOKEN_TYPE_STRING.ordinal) {
		fprintf(stream, 
			"<%s, '%.*s', %d>\n",
			self->token_type.text,
			(self->length - 2),
			(self->start + 1),
			(self->line_index + 1) 
		);
		return;
	}

	fprintf(stream, 
		"<%s, '%.*s', %d>\n",
		self->token_type.text,
		self->length, self->start,
		(self->line_index + 1)
	);
}
// */
