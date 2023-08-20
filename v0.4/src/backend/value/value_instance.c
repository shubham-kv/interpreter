#include "value_instance.h"

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

#include "symbol.h"
#include "symbol_table.h"

#include "value_class.h"
#include "value_fun.h"
#include "value_object.h"

#include "log.h"


void *instance_value_constructor(void *self, va_list *ap);
void *instance_value_destructor(void *self);

bool is_instance_value(const void *self);
void print_instance_value(const void *self);
void *copy_instance_value(const void *self);

bool is_instance_value_true(const void *self);
bool are_instance_values_equal(const void *self, const void *other);


static const struct ValueBaseType _type_instance_value = {
	.size 				= sizeof(struct InstanceValue),
	.constructor 		= instance_value_constructor,
	.destructor 		= instance_value_destructor,

	.is_value 			= is_instance_value,
	.print_value 		= print_instance_value,
	.copy_value 		= copy_instance_value,
	.is_value_true 		= is_instance_value_true,

	.are_values_equal	= are_instance_values_equal,
	.add_values 		= NULL,
	.sub_values 		= NULL,
	.mul_values 		= NULL,
	.div_values 		= NULL,
	.mod_values 		= NULL
};

const void *InstanceValue = &_type_instance_value;


// ... = new(InstanceValue, class, fields_hashtable);

void *instance_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("instance_value_constructor(self: %#x)\n", _self);
	#endif // DEBUG

	struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	self->klass = va_arg(*ap, void *);
	assert(IS_CLASS_VALUE(self->klass));

	self->fields = va_arg(*ap, void *);
	assert(IS_HASH_TABLE(self->fields));

	self->reference_count = 0;

	#if defined(DEBUG)
		vi_log("ret instance_value_constructor(self: %#x)\n\n", _self);
	#endif // DEBUG

	return _self;
}

void *instance_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("instance_value_destructor(self: %#x)\n", _self);
	#endif // DEBUG

	struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	#if defined(DEBUG)
		vi_log("instance_value_destructor: (ref_count: %d)\n", self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret instance_value_destructor(self: %#x): returns NULL\n\n", _self);
		#endif // DEBUG

		return NULL;
	}

	delete(self->fields);

	#if defined(DEBUG)
		vi_log("ret instance_value_destructor(self: %#x)\n\n", _self);
	#endif // DEBUG

	return _self;
}


bool is_instance_value(const void *object) {
	return IS_INSTANCE_VALUE(object);
}


void print_instance_value(const void *_self) {
	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	const struct ClassValue *klass = self->klass;

	printf("<class '%s'> instance", SYMBOL_GET_ID(klass->symbol));
}


void *copy_instance_value(const void *_self) {
	struct InstanceValue *self = (struct InstanceValue *) _self;
	assert(IS_INSTANCE_VALUE(self));

	self->reference_count++;

	return self;
}


bool is_instance_value_true(const void *self) {
	return true;
}


bool are_instance_values_equal(const void *_self, const void *_other) {
	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	if(!IS_INSTANCE_VALUE(_other))
		return false;
	
	const struct InstanceValue *other = _other;

	if(self->klass != other->klass)
		return false;

	return true;
}



void instance_put_field(void *_self, const char *name, const void *value) {
	#if defined(DEBUG)
		vi_log("instance_put_field(self: %#x, name: '%s', value: %#x)\n", _self, name, value);
	#endif // DEBUG

	struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	hashtable_put(self->fields, name, value);

	#if defined(DEBUG)
		vi_log("ret instance_put_field(self: %#x, name: '%s', value: %#x)\n\n", _self, name, value);
	#endif // DEBUG
}

// void instance_set_field(const void *_self, const char *name, const void *value) {
// 	const struct InstanceValue *self = _self;
// 	assert(IS_INSTANCE_VALUE(self));
// 	hashtable_set(self->fields, name, value);
// }

const void *instance_get_field(const void *_self, const char *name) {
	#if defined(DEBUG)
		vi_log("instance_get_field(self: %#x, name: '%s')\n", _self, name);
	#endif // DEBUG

	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	const void *final_value = hashtable_get(self->fields, name);

	#if defined(DEBUG)
		vi_log("ret instance_get_field(self: %#x, name: '%s'): value at %x\n\n", _self, name, final_value);
	#endif // DEBUG

	return final_value;
}


void *instance_get_method(const void *_self, const void *_klass, const char *name) {
	#if defined(DEBUG)
		vi_log("instance_get_method(self: %#x, name: '%s')\n", _self, name);
	#endif // DEBUG

	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	const struct ClassValue *klass = _klass;
	assert(IS_CLASS_VALUE(klass));

	const void *method = NULL;

	do {
		method = hashtable_get(klass->methods, name);
		klass = klass->super_class;
	} while((method == NULL) && (NULL != klass));


	void *final_value = NULL;

	if(NULL != method) {
		final_value = method_bind(method, self);
	}

	#if defined(DEBUG)
		vi_log("ret instance_get_method(self: %#x, name: '%s'): value at %#x\n\n", _self, name, final_value);
	#endif // DEBUG

	return final_value;
}


