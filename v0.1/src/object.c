#include "object.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#include "type.h"


void *new(const void *_class, ...) {
	const struct Type *class = _class;
	assert(class && class->size && class->constructor);

	void *p = calloc(1, class->size);

	if(NULL == p) {
		fprintf(stderr, "Low in memory, memory allocation failed!");
		exit(EXIT_FAILURE);
	}

	*((const struct Type **) p) = class;

	va_list ap;
	va_start(ap, _class);
	p = class->constructor(p, &ap);
	va_end(ap);

	return p;
}


void delete(void *_self) {
	struct Type **p = _self;

	if(p && (NULL != *p) && (NULL != (*p)->destructor))
		p = (*p)->destructor(_self);

	free((void *) p);
}

