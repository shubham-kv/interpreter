#include "value_mib.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "type.h"
#include "object.h"

#include "symbol.h"

#include "value_class.h"
#include "value_fun.h"
#include "value_instance.h"


void *mib_value_constructor(void *self, va_list *ap);
void *mib_value_destructor(void *self);

bool is_mib_value(const void *self);
void print_mib_value(const void *self);
void *copy_mib_value(const void *self);

bool is_mib_value_true(const void *self);


/*
static const struct ValueBaseType _type_mib_value = {
	sizeof(struct MibValue),
	mib_value_constructor,
	mib_value_destructor,

	is_mib_value, print_mib_value, copy_mib_value,
	is_mib_value_true, NULL,
	NULL, NULL, NULL, NULL, NULL
};
// */

static const struct ValueBaseType _type_mib_value = {
	.size 			= sizeof(struct MibValue),
	.constructor 	= mib_value_constructor,
	.destructor 	= mib_value_destructor,

	.is_value 		= is_mib_value,
	.print_value 	= print_mib_value,
	.copy_value 	= copy_mib_value,
	.is_value_true 	= is_mib_value_true,

	.are_values_equal = NULL,
	.add_values 	= NULL,
	.sub_values 	= NULL,
	.mul_values 	= NULL,
	.div_values 	= NULL,
	.mod_values 	= NULL
};



const void *MibValue = &_type_mib_value;


// ... = new(MibValue, class, fields_hashtable);

void *mib_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("mib_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct MibValue *self = _self;
	assert(IS_MIB_VALUE(self));

	self->method = va_arg(*ap, const void *);
	assert(IS_FUN_VALUE(self->method));

	self->instance = va_arg(*ap, const void *);
	assert(IS_INSTANCE_VALUE(self->instance));
	
	self->reference_count = 0;

	#if defined(DEBUG)
		vi_log("ret mib_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

void *mib_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("mib_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct MibValue *self = _self;
	assert(IS_MIB_VALUE(self));

	#if defined(DEBUG)
		vi_log("mib_value_destructor(self: %x): ref_count: %d\n", _self, self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret mib_value_destructor(self: %x): returns NULL\n", _self);
		#endif // DEBUG

		return NULL;
	}

	// delete((void *) self->instance);

	#if defined(DEBUG)
		vi_log("mib_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


bool is_mib_value(const void *object) {
	return IS_MIB_VALUE(object);
}


void print_mib_value(const void *_self) {
	const struct MibValue *self = _self;
	assert(IS_MIB_VALUE(self));

	const struct InstanceValue *instance = self->instance;
	assert(IS_INSTANCE_VALUE(instance));

	const struct ClassValue *klass = instance->klass;
	assert(IS_CLASS_VALUE(klass));

	const struct FunValue *method = self->method;
	assert(IS_FUN_VALUE(method));

	printf("<method '%s' of '%s'>", SYMBOL_GET_ID(method->symbol), SYMBOL_GET_ID(klass->symbol));
}


void *copy_mib_value(const void *_self) {
	struct MibValue *self = (struct MibValue *) _self;
	assert(IS_MIB_VALUE(self));

	self->reference_count++;

	return self;
}


bool is_mib_value_true(const void *self) {
	return true;
}




