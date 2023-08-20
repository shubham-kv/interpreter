#include "value.h"

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


static void *value_constructor(void *self, va_list *ap);
static void *value_destructor(void *self);

bool v_is_value(const void *self);

bool v_is_value_true(const void *self);
void v_print_value(const void *self);

void *v_copy_value(const void *self);

bool v_are_values_equal(const void *self, const void *other);

void *v_add_values(const void *self, const void *other);
void *v_sub_values(const void *self, const void *other);
void *v_mul_values(const void *self, const void *other);
void *v_div_values(const void *self, const void *other);
void *v_mod_values(const void *self, const void *other);


static const struct ValueBaseType _type_Value = {
	sizeof(struct Value),
	value_constructor,
	value_destructor,
	v_is_value,

	v_print_value,
	v_copy_value,
	v_is_value_true,
	v_are_values_equal,

	v_add_values,
	v_sub_values,
	v_mul_values,
	v_div_values,
	v_mod_values
};

const void *Value = &_type_Value;


// ... = new(Value, VALUE_TYPE_INTEGER, 78);

static void *value_constructor(void *_self, va_list *ap) {
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
			VALUE_AS_STRING(self) = va_arg(*ap, void *);
			break;
		}
		
		default: {
			// must be unreachable
			fprintf(stderr, "ValueIntializationError: Unknown Value type '%d'.\n", valueType);
			exit(EXIT_FAILURE);
		}
	}

	return _self;
}


static void *value_destructor(void *_self) {
	struct Value *self = _self;
	assert(IS_VALUE(self));

	if(self->reference_count > 0) {
		self->reference_count--;
		return NULL;
	}

	if(VALUE_TYPE_STRING == self->value_type) {
		char *str = self->value.pointer;
		free(str);
	}

	return _self;
}




bool v_is_value(const void *object) {
	return IS_VALUE(object);
}



void v_print_value(const void *_self) {
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
			char *str = (char *) self->value.pointer;

			if(NULL != str)
				printf("'%s'", str);

			break;
		}
	}
}



void *v_copy_value(const void *_self) {
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



bool v_is_value_true(const void *_self) {
	const struct Value *self = _self;
	assert(IS_VALUE(self));

	switch(self->value_type) {
		case VALUE_TYPE_BOOL:
		case VALUE_TYPE_INTEGER:
			return (VALUE_AS_INT(self) != 0);

		case VALUE_TYPE_DOUBLE:
			return (VALUE_AS_DOUBLE(self) != 0);

		case VALUE_TYPE_STRING: {
			char *str = self->value.pointer;
			return ((NULL != str) && (strlen(str) != 0));
		}

		default:
			return false;
	}
}



bool v_are_values_equal(const void *_self, const void *_other) {
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



void *v_add_values(const void *_self, const void *_other) {
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

void *v_sub_values(const void *_self, const void *_other) {
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

void *v_mul_values(const void *_self, const void *_other) {
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

void *v_div_values(const void *_self, const void *_other) {
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

void *v_mod_values(const void *_self, const void *_other) {
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




