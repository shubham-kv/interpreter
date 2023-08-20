#include "value_object.h"

#include <stdbool.h>
#include <assert.h>

#include "type.h"



bool is_value(const void *self) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p) && (*p)->is_value);

	return (*p)->is_value(self);
}

void print_value(const void *self) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p) && (*p)->print_value);

	(*p)->print_value(self);
}

void *copy_value(const void *self) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->copy_value) {
		return (*p)->copy_value(self);
	} else {
		return NULL;
	}
}


bool is_value_true(const void *self) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p) && (*p)->is_value_true);

	return (*p)->is_value_true(self);
}

bool are_values_equal(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p) && (*p)->are_values_equal);

	return (*p)->are_values_equal(self, other);
}


void *add_values(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->add_values) {
		return (*p)->add_values(self, other);
	} else {
		return NULL;
	}
}

void *sub_values(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->sub_values) {
		return (*p)->sub_values(self, other);
	} else {
		return NULL;
	}
}

void *mul_values(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->mul_values) {
		return (*p)->mul_values(self, other);
	} else {
		return NULL;
	}
}

void *div_values(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->div_values) {
		return (*p)->div_values(self, other);
	} else {
		return NULL;
	}
}

void *mod_values(const void *self, const void *other) {
	const struct ValueBaseType *const * p = self;
	assert(p && (*p));

	if((*p)->mod_values) {
		return (*p)->mod_values(self, other);
	} else {
		return NULL;
	}
}


