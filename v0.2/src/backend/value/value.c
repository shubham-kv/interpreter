#include "value.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "log.h"
#include "type.h"
#include "object.h"
#include "memory.h"
#include "value_type.h"


static void *normal_value_constructor(void *self, va_list *ap);
static void *normal_value_destructor(void *self);

bool is_normal_value(const void *self);

bool is_normal_value_true(const void *self);
void print_normal_value(const void *self);

void *copy_normal_value(const void *self);

bool are_normal_values_equal(const void *self, const void *other);

void *add_normal_values(const void *self, const void *other);
void *sub_normal_values(const void *self, const void *other);
void *mul_normal_values(const void *self, const void *other);
void *div_normal_values(const void *self, const void *other);
void *mod_normal_values(const void *self, const void *other);


/*
static const struct ValueBaseType _type_Value = {
	sizeof(struct Value),
	normal_value_constructor,
	normal_value_destructor,
	is_normal_value,

	print_normal_value,
	copy_normal_value,
	is_normal_value_true,
	are_normal_values_equal,

	add_normal_values,
	sub_normal_values,
	mul_normal_values,
	div_normal_values,
	mod_normal_values
};
*/


static const struct ValueBaseType _type_normal_value = {
	.size 			= sizeof(struct Value),
	.constructor 	= normal_value_constructor,
	.destructor 	= normal_value_destructor,

	.is_value 		= is_normal_value,
	.print_value 	= print_normal_value,
	.copy_value 	= copy_normal_value,
	.is_value_true 	= is_normal_value_true,

	.are_values_equal = are_normal_values_equal,
	.add_values 	= add_normal_values,
	.sub_values 	= sub_normal_values,
	.mul_values 	= mul_normal_values,
	.div_values 	= div_normal_values,
	.mod_values 	= mod_normal_values
};


const void *Value = &_type_normal_value;


// ... = new(Value, VALUE_TYPE_INTEGER, 78);

static void *normal_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("normal_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Value *self = _self;
	assert(IS_VALUE(self));

	enum ValueType valueType = va_arg(*ap, enum ValueType);
	self->reference_count = 0;

	switch(valueType) {
		case VALUE_TYPE_INTEGER: {
			self->value_type = VALUE_TYPE_INTEGER;
			VALUE_AS_INT(self) = va_arg(*ap, const int);
			break;
		}
		case VALUE_TYPE_DOUBLE: {
			self->value_type = VALUE_TYPE_DOUBLE;
			VALUE_AS_DOUBLE(self) = va_arg(*ap, const double);
			break;
		}

		case VALUE_TYPE_BOOL: {
			self->value_type = VALUE_TYPE_BOOL;
			VALUE_AS_INT(self) = va_arg(*ap, const int);
			break;
		}

		case VALUE_TYPE_STRING: {
			self->value_type = VALUE_TYPE_STRING;
			VALUE_AS_STRING(self) = va_arg(*ap, char *);
			break;
		}
		
		default: {
			#if defined(DEBUG)
				fprintf(stderr, "ValueIntializationError: Unknown Value type '%d'.\n", valueType);
			#endif // DEBUG
			exit(EXIT_FAILURE);
		}
	}

	#if defined(DEBUG)
		vi_log("ret normal_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


static void *normal_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("normal_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Value *self = _self;
	assert(IS_VALUE(self));

	#if defined(DEBUG)
		vi_log("normal_value_destructor(self: %x): ref_count: %d\n", _self, self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret normal_value_destructor(self: %x): returns NULL\n", _self);
		#endif // DEBUG

		return NULL;
	}

	if(VALUE_TYPE_STRING == self->value_type) {
		char *str = self->value.pointer;
		free(str);
	}

	#if defined(DEBUG)
		vi_log("ret normal_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}




bool is_normal_value(const void *object) {
	return IS_VALUE(object);
}



void print_normal_value(const void *_self) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	switch(self->value_type) {
		case VALUE_TYPE_INTEGER: {
			int ival = VALUE_AS_INT(self);
			printf("%d", ival);
			break;
		}

		case VALUE_TYPE_DOUBLE: {
			double dval = VALUE_AS_DOUBLE(self);
			printf("%g", dval);
			break;
		}

		case VALUE_TYPE_BOOL: {
			int ival = VALUE_AS_INT(self);
			printf(ival ? "true" : "false");
			break;
		}

		case VALUE_TYPE_STRING: {
			char *string = (char *) self->value.pointer;

			if(NULL != string)
				printf("%s", string);
			
			break;
		}
	}
}



void *copy_normal_value(const void *_self) {
	struct Value *self = (struct Value *) _self;
	assert(IS_VALUE(self));

	switch(self->value_type) {
		case VALUE_TYPE_BOOL:
		case VALUE_TYPE_INTEGER:
		case VALUE_TYPE_DOUBLE:
		case VALUE_TYPE_STRING: {
			self->reference_count++;
			return self;
		}
	}

	return NULL;
}



bool is_normal_value_true(const void *_self) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	switch(self->value_type) {
		case VALUE_TYPE_BOOL:
		case VALUE_TYPE_INTEGER:
			return (VALUE_AS_INT(self) != 0);

		case VALUE_TYPE_DOUBLE:
			return (VALUE_AS_DOUBLE(self) != 0.0);

		case VALUE_TYPE_STRING: {
			char *str = self->value.pointer;
			return ((NULL != str) && (strlen(str) != 0));
		}
	}

	return false;
}



bool are_normal_values_equal(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if(self->value_type != other->value_type) return false;

	switch(self->value_type) {
		case VALUE_TYPE_BOOL:
		case VALUE_TYPE_INTEGER:
			return (VALUE_AS_INT(self) == VALUE_AS_INT(other));
		
		case VALUE_TYPE_DOUBLE:
			return (VALUE_AS_DOUBLE(self) == VALUE_AS_DOUBLE(other));

		case VALUE_TYPE_STRING:
			return (strcmp(self->value.pointer, other->value.pointer) == 0);
	}

	return false;
}



void *add_normal_values(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if(self->value_type == other->value_type) {
		switch(self->value_type) {
			case VALUE_TYPE_INTEGER:
				return new(Value, VALUE_TYPE_INTEGER, (VALUE_AS_INT(self) + VALUE_AS_INT(other)));

			case VALUE_TYPE_DOUBLE:
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) + VALUE_AS_DOUBLE(other)));

			case VALUE_TYPE_STRING: {
				const char *str1 = VALUE_AS_STRING(self);
				const char *str2 = VALUE_AS_STRING(other);

				size_t l1 = strlen(str1);
				size_t l2 = strlen(str2);

				char *res = allocate(sizeof(char), l1 + l2 + 1);
				strcpy(res, str1);
				strcpy(res + l1, str2);

				return new(Value, VALUE_TYPE_STRING, res);
			}

			default:
				return NULL;
		}
	}

	else {
		if(VALUE_TYPE_INTEGER == self->value_type) {
			if(VALUE_TYPE_DOUBLE == other->value_type) {
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_INT(self) + VALUE_AS_DOUBLE(other)));
			}
			else if(VALUE_TYPE_STRING == other->value_type) {
				const int ival = VALUE_AS_INT(self);
				const char *str = VALUE_AS_STRING(other);

				size_t l1 = snprintf(NULL, 0, "%d", ival);
				size_t l2 = strlen(str);
				size_t size = l1 + l2 + 1;

				char *res = allocate(sizeof(char), size);
				snprintf(res, size, "%d", ival);
				strcpy(res + l1, str);

				return new(Value, VALUE_TYPE_STRING, res);
			}
			return NULL;
		}

		else if(VALUE_TYPE_DOUBLE == self->value_type) {
			if(VALUE_TYPE_INTEGER == other->value_type) {
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) + VALUE_AS_INT(other)));
			}
			else if(VALUE_TYPE_STRING == other->value_type) {
				const double dval = VALUE_AS_DOUBLE(self);
				const char *str = VALUE_AS_STRING(other);
			
				size_t l1 = snprintf(NULL, 0, "%g", dval);
				size_t l2 = strlen(str);
				size_t size = l1 + l2 + 1;

				char *res = allocate(sizeof(char), size);
				snprintf(res, size, "%g", dval);
				strcpy(res + l1, str);

				return new(Value, VALUE_TYPE_STRING, res);
			}
			return NULL;
		}

		else if(VALUE_TYPE_STRING == self->value_type) {
			if(VALUE_TYPE_INTEGER == other->value_type) {
				const char *str = VALUE_AS_STRING(self);
				const int ival = VALUE_AS_INT(other);

				size_t l1 = strlen(str);
				size_t l2 = snprintf(NULL, 0, "%d", ival);	// gets you the length of the printed string
				size_t size = l1 + l2 + 1;

				char *res = allocate(sizeof(char), size);
				strcpy(res, str);
				snprintf(res + l1, l2 + 1, "%d", ival);

				return new(Value, VALUE_TYPE_STRING, res);
			}
			else if(VALUE_TYPE_DOUBLE == other->value_type) {
				const char *str = VALUE_AS_STRING(self);
				double dval = VALUE_AS_DOUBLE(other);

				size_t l1 = strlen(str);
				size_t l2 = snprintf(NULL, 0, "%g", dval);
				size_t size = l1 + l2 + 1;

				char *res = allocate(sizeof(char), size);
				strcpy(res, str);
				snprintf(res + l1, l2 + 1, "%g", dval);

				return new(Value, VALUE_TYPE_STRING, res);
			}
			return NULL;
		}
	}

	return NULL;
}

void *sub_normal_values(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if(self->value_type == other->value_type) {
		switch(self->value_type) {
			case VALUE_TYPE_INTEGER:
				return new(Value, VALUE_TYPE_INTEGER, (VALUE_AS_INT(self) - VALUE_AS_INT(other)));

			case VALUE_TYPE_DOUBLE:
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) - VALUE_AS_DOUBLE(other)));
		}
	}
	else {
		if((VALUE_TYPE_INTEGER == self->value_type) && (VALUE_TYPE_DOUBLE == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_INT(self) - VALUE_AS_DOUBLE(other)));
		}
		else if((VALUE_TYPE_DOUBLE == self->value_type) && (VALUE_TYPE_INTEGER == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) - VALUE_AS_INT(other)));
		}
	}

	return NULL;
}

void *mul_normal_values(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if(self->value_type == other->value_type) {
		switch(self->value_type) {
			case VALUE_TYPE_INTEGER:
				return new(Value, VALUE_TYPE_INTEGER, (VALUE_AS_INT(self) * VALUE_AS_INT(other)));

			case VALUE_TYPE_DOUBLE:
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) * VALUE_AS_DOUBLE(other)));
		}
	}
	else {
		if((VALUE_TYPE_INTEGER == self->value_type) && (VALUE_TYPE_DOUBLE == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_INT(self) * VALUE_AS_DOUBLE(other)));
		}
		if((VALUE_TYPE_DOUBLE == self->value_type) && (VALUE_TYPE_INTEGER == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) * VALUE_AS_INT(other)));
		}
	}

	return NULL;
}

void *div_normal_values(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if(self->value_type == other->value_type) {
		switch(self->value_type) {
			case VALUE_TYPE_INTEGER:
				return new(Value, VALUE_TYPE_INTEGER, (VALUE_AS_INT(self) / VALUE_AS_INT(other)));

			case VALUE_TYPE_DOUBLE:
				return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) / VALUE_AS_DOUBLE(other)));
		}
	}
	else {
		if((VALUE_TYPE_INTEGER == self->value_type) && (VALUE_TYPE_DOUBLE == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_INT(self) / VALUE_AS_DOUBLE(other)));
		}
		if((VALUE_TYPE_DOUBLE == self->value_type) && (VALUE_TYPE_INTEGER == other->value_type)) {
			return new(Value, VALUE_TYPE_DOUBLE, (VALUE_AS_DOUBLE(self) / VALUE_AS_INT(other)));
		}
	}

	return NULL;
}

void *mod_normal_values(const void *_self, const void *_other) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	const struct Value *other = _other;
	assert(IS_VALUE(other));

	if((VALUE_TYPE_INTEGER == self->value_type) && (VALUE_TYPE_INTEGER == other->value_type)) {
		int val1 = VALUE_AS_INT(self);
		int val2 = VALUE_AS_INT(other);

		return new(Value, VALUE_TYPE_INTEGER, (val1 % val2));
	}

	return NULL;
}




