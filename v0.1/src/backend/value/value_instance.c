#include "value_instance.h"

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

#include "value_class.h"



void *ivalue_constructor(void *self, va_list *ap);
void *ivalue_destructor(void *self);

bool is_instance_value(const void *self);

void *ivalue_copy_value(const void *self);
bool ivalue_is_value_true(const void *self);

void ivalue_print_value(const void *self);


static const struct ValueBaseType _type_instance_value = {
	sizeof(struct InstanceValue),
	ivalue_constructor,
	ivalue_destructor,
	is_instance_value,

	ivalue_print_value, ivalue_copy_value, ivalue_is_value_true, NULL,
	NULL, NULL, NULL, NULL
};

const void *InstanceValue = &_type_instance_value;


// ... = new(InstanceValue, belonging_class, properties_hashtable);

void *ivalue_constructor(void *_self, va_list *ap) {
	struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	self->klass = va_arg(*ap, void *);
	assert(IS_CLASS_VALUE(self->klass));

	self->properties = va_arg(*ap, void *);
	assert(IS_HASH_TABLE(self->properties));

	self->reference_count = 0;

	return _self;
}

void *ivalue_destructor(void *_self) {
	struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	if(self->reference_count > 0) {
		self->reference_count--;
		return NULL;
	}

	delete(self->properties);

	return _self;
}


bool is_instance_value(const void *object) {
	return IS_INSTANCE_VALUE(object);
}


void *ivalue_copy_value(const void *_self) {
	struct InstanceValue *self = (struct InstanceValue *) _self;
	assert(IS_INSTANCE_VALUE(self));

	self->reference_count++;

	return self;
}

bool ivalue_is_value_true(const void *self) {
	return true;
}



/*

class Point {
	var x;
	var y;
};

var point = Point(12, 34);

outln(point);		// <class 'Point'> instance
outln(point.x);		// 12
outln(point.y);		// 34

*/

void ivalue_print_value(const void *_self) {
	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	const struct ClassValue *klass = self->klass;

	printf("<class '%s'> instance", SYMBOL_GET_ID(klass->symbol));
}



const void *instance_get_property(const void *_self, const char *property_name) {
	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	return hashtable_get(self->properties, property_name);
}

const void *instance_set_property(const void *_self, const char *property_name, const void *value) {
	const struct InstanceValue *self = _self;
	assert(IS_INSTANCE_VALUE(self));

	return hashtable_set(self->properties, property_name, value);
}


