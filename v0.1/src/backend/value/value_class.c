#include "value_class.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"
#include "hash_table.h"

#include "symbol.h"
#include "symbol_table.h"


void *class_value_constructor(void *self, va_list *ap);
void *class_value_destructor(void *self);

bool is_class_value(const void *self);

void *class_value_copy_value(const void *self);
bool class_value_is_value_true(const void *self);

void class_value_print_value(const void *self);


static const struct ValueBaseType _type_class = {
	sizeof(struct ClassValue),
	class_value_constructor,
	class_value_destructor,
	is_class_value,
	
	class_value_print_value, class_value_copy_value, class_value_is_value_true, NULL,
	NULL, NULL, NULL, NULL
};

const void *ClassValue = &_type_class;



// ... = new(ClassValue, class_value_symbol, class_value_symbol_table, property_count, properties, methods, constructor);

void *class_value_constructor(void *_self, va_list *ap) {
	struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	self->symbol = va_arg(*ap, void *);
	assert(IS_SYMBOL(self->symbol));

	self->symbol_table = va_arg(*ap, const void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->property_count = (uint32_t) va_arg(*ap, int);
	self->properties = va_arg(*ap, void **);

	for(int i = 0; i < self->property_count; i++) {
		assert(IS_SYMBOL(self->properties[i]));
	}

	self->methods = va_arg(*ap, void *);
	assert(IS_HASH_TABLE(self->methods));

	typedef void *(*constr_type)(const void *, void **);
	self->constructor = va_arg(*ap, constr_type);

	return _self;
}

void *class_value_destructor(void *_self) {
	struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	if(self->reference_count > 0) {
		self->reference_count--;
		return NULL;
	}

	return _self;
}


bool is_class_value(const void *object) {
	return IS_CLASS_VALUE(object);
}


void *class_value_copy_value(const void *_self) {
	struct ClassValue *self = (struct ClassValue *) _self;
	assert(IS_CLASS_VALUE(self));

	self->reference_count++;

	return self;
}

bool class_value_is_value_true(const void *self) {
	return true;
}

void class_value_print_value(const void *_self) {
	const struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	printf("<class '%s'>", SYMBOL_GET_ID(self->symbol));
}

