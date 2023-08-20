#include "object.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
#include "type.h"


void *new(const void *_type, ...) {

	const struct Type *type = _type;
	assert(type && type->size && type->constructor);

	#if defined(DEBUG)
		vi_log("new(type: %x)\n", _type);
	#endif // DEBUG

	void *p = calloc(1, type->size);

	if(NULL == p) {
		#if defined(DEBUG)
			vi_log("new(): Memory allocation failed!");
			fprintf(stderr, "Memory allocation failed!");
		#endif // DEBUG

		exit(EXIT_FAILURE);
	}

	*((const struct Type **) p) = type;

	va_list ap;
	va_start(ap, _type);

	p = type->constructor(p, &ap);
	va_end(ap);

	#if defined(DEBUG)
		vi_log("ret new(type: %x)\n\n", _type);
	#endif // DEBUG

	return p;
}


void delete(void *_self) {
	struct Type **p = _self;

	#if defined(DEBUG)
		vi_log("delete(self: %x)\n", _self);
	#endif // DEBUG

	if(p && (NULL != *p) && (NULL != (*p)->destructor)) {
		p = (*p)->destructor(_self);
	}

	free((void *) p);

	#if defined(DEBUG)
		vi_log("ret delete(self: %x)\n\n", _self);
	#endif // DEBUG	
}

