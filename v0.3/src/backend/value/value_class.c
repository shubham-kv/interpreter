#include "value_class.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "object.h"
#include "type.h"
#include "ast_object.h"

#include "symbol.h"
#include "symbol_table.h"

#include "log.h"


void *class_value_constructor(void *self, va_list *ap);
void *class_value_destructor(void *self);

bool is_class_value(const void *self);

void *copy_class_value(const void *self);
bool is_class_value_true(const void *self);

void print_class_value(const void *self);


static const struct ValueBaseType _type_class = {
	.size 			= sizeof(struct ClassValue),
	.constructor 	= class_value_constructor,
	.destructor 	= class_value_destructor,

	.is_value 		= is_class_value,
	.print_value 	= print_class_value,
	.copy_value 	= copy_class_value,
	.is_value_true 	= is_class_value_true,

	.are_values_equal = NULL,
	.add_values 	= NULL,
	.sub_values 	= NULL,
	.mul_values 	= NULL,
	.div_values 	= NULL,
	.mod_values 	= NULL
};

const void *ClassValue = &_type_class;



// ... = new(ClassValue, symbol, symbol_table, super_class_value, constr_decl, instance_creator, methods);

void *class_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("class_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	self->symbol = va_arg(*ap, void *);
	assert(IS_SYMBOL(self->symbol));

	self->symbol_table = va_arg(*ap, const void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->super_class = va_arg(*ap, const void *);

	if(NULL != self->super_class) {
		assert(IS_CLASS_VALUE(self->super_class));
	}

	self->constr_decl = va_arg(*ap, const void *);

	if(NULL != self->constr_decl)
		assert(is_ast_node(self->constr_decl));
	
	typedef void *(*instance_initializer_type)(const void *, void *, void **);
	self->instance_initializer = va_arg(*ap, instance_initializer_type);

	self->methods = va_arg(*ap, void *);
	assert(IS_HASH_TABLE(self->methods));

	// self->property_count = (uint32_t) va_arg(*ap, int);
	// self->properties = va_arg(*ap, void **);
	// for(int i = 0; i < self->property_count; i++) {
	// 	assert(IS_SYMBOL(self->properties[i]));
	// }

	#if defined(DEBUG)
		vi_log("ret class_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

void *class_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("class_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	#if defined(DEBUG)
		vi_log("class_value_destructor(self: %x): ref_count: %d\n", _self, self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret class_value_destructor(self: %x): returns NULL\n", _self);
		#endif // DEBUG

		return NULL;
	}

	#if defined(DEBUG)
		vi_log("ret class_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


bool is_class_value(const void *object) {
	return IS_CLASS_VALUE(object);
}


void *copy_class_value(const void *_self) {
	struct ClassValue *self = (struct ClassValue *) _self;
	assert(IS_CLASS_VALUE(self));

	self->reference_count++;

	return self;
}

bool is_class_value_true(const void *self) {
	return true;
}

void print_class_value(const void *_self) {
	const struct ClassValue *self = _self;
	assert(IS_CLASS_VALUE(self));

	printf("<class '%s'>", SYMBOL_GET_ID(self->symbol));
}

