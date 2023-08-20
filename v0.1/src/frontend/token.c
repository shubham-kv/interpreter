#include "token.h"

#include <assert.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "token_type.h"
#include "type.h"


static void *token_constructor(void *self, va_list *ap);
static void *token_destructor(void *self);

static const struct Type _type_token = {
	sizeof(struct Token),
	token_constructor,
	token_destructor
};

const void *Token = &_type_token;

static void *token_constructor(void *_self, va_list *ap) {
	struct Token *self = _self;
	assert(IS_TOKEN(self));

	self->token_type 	= va_arg(*ap, struct TokenType);
	self->start 		= va_arg(*ap, const char *);
	self->start_index 	= va_arg(*ap, int);
	self->length 		= va_arg(*ap, int);
	self->line_index 	= va_arg(*ap, int);

	return _self;
}

static void *token_destructor(void *_self) {
	return _self;
}


/*
<PRINT, 1> <INT, 12, 1> <ADD> <DOUBLE, 3.14>
<EOF>
*/


void print_token(const void *_self) {
	const struct Token *self = _self;
	assert(IS_TOKEN(self));

	if(self->token_type.ordinal == TOKEN_TYPE_EOF.ordinal) {
		printf("<EOF>\n");
		return;
	}

	if(self->token_type.ordinal == TOKEN_TYPE_STRING.ordinal) {
		printf(
			"<%s, '%.*s', %d>\n",
			self->token_type.text,
			(self->length - 2),
			(self->start + 1),
			(self->line_index + 1) 
		);
		return;
	}

	printf(
		"<%s, '%.*s', %d>\n",
		self->token_type.text,
		self->length, self->start,
		(self->line_index + 1)
	);
}
