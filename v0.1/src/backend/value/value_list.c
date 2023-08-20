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



static void *listvalue_constructor(void *self, va_list *ap);
static void *listvalue_destructor(void *self);

bool is_list_value(const void *object);

void lv_print_value(const void *self);
void *lv_copy_value(const void *self);

bool lv_is_value_true(const void *self);
bool lv_are_values_equal(const void *self, const void *other);



static const struct ValueBaseType _type_listValue = {
	sizeof(struct ListValue),
	listvalue_constructor,
	listvalue_destructor,
	is_list_value,

	lv_print_value,
	lv_copy_value,
	lv_is_value_true,
	lv_are_values_equal,

	NULL, NULL, NULL, NULL, NULL
};

const void *ListValue = &_type_listValue;


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


// ... = new(ListValue, child_value_type, element_count, elements);

static void *listvalue_constructor(void *_self, va_list *ap) {
	struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	// self->value_type = VALUE_TYPE_LIST;
	self->count = (uint32_t) va_arg(*ap, int);
	self->values = va_arg(*ap, void **);

	self->reference_count = 0;

	return _self;
}


static void *listvalue_destructor(void *_self) {
	struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	if(self->reference_count > 0) {
		self->reference_count--;
		return NULL;
	}

	for(uint32_t i = 0; i < self->count; i++)
		delete(self->values[i]);
	
	free(self->values);

	return _self;
}



bool is_list_value(const void *object) {
	return IS_LIST_VALUE(object);
}


// For DEV Purposes only
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


void lv_print_value(const void *_self) {
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



void *lv_copy_value(const void *_self) {
	struct ListValue *self = (struct ListValue *) _self;
	assert(IS_LIST_VALUE(self));

	self->reference_count++;

	return self;
}



bool lv_is_value_true(const void *_self) {
	const struct ListValue *self = _self;
	assert(IS_LIST_VALUE(self));

	return (self->count > 0) ? true : false;
}





bool lv_are_values_equal(const void *_self, const void *_other) {
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



