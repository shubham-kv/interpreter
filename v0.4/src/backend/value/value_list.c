#include "value_list.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"
#include "memory.h"

#include "value_type.h"
#include "value.h"
#include "value_object.h"

#include "log.h"


static void *list_value_constructor(void *self, va_list *ap);
static void *list_value_destructor(void *self);

bool is_list_value(const void *object);

void print_list_value(const void *self);
void *copy_list_value(const void *self);

bool is_list_value_true(const void *self);
bool are_list_values_equal(const void *self, const void *other);



static const struct ValueBaseType _type_list_value = {
	.size 			= sizeof(struct ListValue),
	.constructor 	= list_value_constructor,
	.destructor 	= list_value_destructor,

	.is_value 		= is_list_value,
	.print_value 	= print_list_value,
	.copy_value 	= copy_list_value,
	.is_value_true 	= is_list_value_true,

	.are_values_equal = are_list_values_equal,
	.add_values 	= NULL,
	.sub_values 	= NULL,
	.mul_values 	= NULL,
	.div_values 	= NULL,
	.mod_values 	= NULL
};



const void *ListValue = &_type_list_value;


/*
val nums = [ 2, 4, 6, 7 ];
typeof elements = 'int *'
new(ListValue, VALUE_TYPE_INTEGER, 4, elements);

var names = [ 'Aria', 'Riya', 'Prince' ];

typeof elements = 'char **'
new(ListValue, VALUE_TYPE_STRING, 3, [ Value(VALUE_TYEP_STRING, 'Jade'), Value(VALUE_TYPE_INT, 34) ]);

outln(names[0]);

names[0] = 'Grace';
names[0] = 0;
*/


// ... = new(ListValue, count, values);

static void *list_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("list_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	self->count = (uint32_t) va_arg(*ap, int);
	self->values = va_arg(*ap, void **);
	self->reference_count = 0;

	#if defined(DEBUG)
		vi_log("ret list_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


static void *list_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("list_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	#if defined(DEBUG)
		vi_log("list_value_destructor(self: %x): ref_count: %d\n", _self, self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret list_value_destructor(self: %x): returns NULL\n", _self);
		#endif // DEBUG

		return NULL;
	}

	for(uint32_t i = 0; i < self->count; i++)
		delete(self->values[i]);
	
	free(self->values);

	#if defined(DEBUG)
		vi_log("ret list_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}



bool is_list_value(const void *object) {
	return IS_LIST_VALUE(object);
}


// For DEV Purposes only
/*
void print_list_value(const void *_self) {
	const struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	printf("ListValue {\n");
	printf("    count: %d,\n", self->count);
	printf("    values_address: %x,\n", self->values);

	printf("    values: ");

	printf("[ ");

	print_value(self->values[0]);

	for(uint32_t i = 1; i < self->count; i++) {
		printf(", ");
		print_value(self->values[i]);
	}
	
	printf(" ]\n");

	printf("    ref_count: %d,\n", self->reference_count);
	printf("}\n");
}
// */


void print_list_value(const void *_self) {
	const struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	if(0 == self->count) {
		printf("[]");
	}
	else {
		printf("[ ");

		print_value(self->values[0]);

		for(uint32_t i = 1; i < self->count; i++) {
			printf(", ");
			print_value(self->values[i]);
		}
		
		printf(" ]");
	}
}



void *copy_list_value(const void *_self) {
	struct ListValue *self = (struct ListValue *) _self;
	assert(IS_LIST_VALUE(self));

	self->reference_count++;

	return self;
}



bool is_list_value_true(const void *_self) {
	const struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	return (self->count > 0) ? true : false;
}





bool are_list_values_equal(const void *_self, const void *_other) {
	const struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	if(!IS_LIST_VALUE(_other))
		return false;

	const struct ListValue *other = _other;

	if(self->count != other->count) {
		return false;
	}

	for(uint32_t i = 0; i < self->count; i++) {
		if(!are_values_equal(self->values[i], other->values[i])) {
			return false;
		}
	}

	return true;
}



