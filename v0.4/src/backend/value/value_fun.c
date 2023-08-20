#include "value_fun.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "headers_backend_value.h"

#include "symbol.h"
#include "symbol_table.h"

#include "log.h"


void *fun_value_constructor(void *self, va_list *ap);
void *fun_value_destructor(void *self);

bool is_fun_value(const void *self);
void print_fun_value(const void *self);
void *copy_fun_value(const void *self);

bool is_fun_value_true(const void *self);



static const struct ValueBaseType _type_fun_value = {
	.size 			= sizeof(struct FunValue),
	.constructor 	= fun_value_constructor,
	.destructor 	= fun_value_destructor,

	.is_value 		= is_fun_value,
	.print_value 	= print_fun_value,
	.copy_value 	= copy_fun_value,
	.is_value_true 	= is_fun_value_true,

	.are_values_equal = NULL,
	.add_values 	= NULL,
	.sub_values 	= NULL,
	.mul_values 	= NULL,
	.div_values 	= NULL,
	.mod_values 	= NULL
};

const void *FunValue = &_type_fun_value;


// ... = new(FunValue, fun_type, fun_value_symbol, fun_value_symbol_table, param_count, param_symbols, executor_fun_value, declarations);

void *fun_value_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("fun_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

	self->fun_type = va_arg(*ap, enum FunctionType);
	assert((self->fun_type > FUNCTION_TYPE_NONE) && (self->fun_type <= FUNCTION_TYPE_METHOD));

	self->klass = va_arg(*ap, const void *);
	if(self->klass) {
		assert(IS_CLASS_VALUE(self->klass));
	}

	self->symbol = va_arg(*ap, void *);
	assert(IS_SYMBOL(self->symbol));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->param_count = (uint32_t) va_arg(*ap, uint32_t);
	self->param_symbols = va_arg(*ap, void **);

	for(uint32_t i = 0; i < self->param_count; i++) {
		assert(IS_SYMBOL(self->param_symbols[i]));
	}

	typedef void *(*executor_fun_value)(const void *, void **);
	self->execute = va_arg(*ap, executor_fun_value);

	self->declarations = va_arg(*ap, void **);

	self->reference_count = 0;

	#if defined(DEBUG)
		vi_log("ret fun_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}

void *fun_value_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("fun_value_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

	#if defined(DEBUG)
		vi_log("fun_value_destructor(self: %x): ref_count: %d\n", _self, self->reference_count);
	#endif // DEBUG

	if(self->reference_count > 0) {
		self->reference_count--;

		#if defined(DEBUG)
			vi_log("ret fun_value_constructor(self: %x) returns NULL\n", _self);
		#endif // DEBUG
		return NULL;
	}

	#if defined(DEBUG)
		vi_log("ret fun_value_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


bool is_fun_value(const void *object) {
	return IS_FUN_VALUE(object);
}

void *copy_fun_value(const void *_self) {
	struct FunValue *self = (struct FunValue *) _self;
	assert(IS_FUN_VALUE(self));

	self->reference_count++;

	return self;
}

bool is_fun_value_true(const void *self) {
	return true;
}



void print_fun_value(const void *_self) {
	const struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

	printf("<fun_value '%s'>", SYMBOL_GET_ID(self->symbol));
}




void *method_bind(const void *_self, const void *instance) {
	#if defined(DEBUG)
		vi_log("method_bind(self: %x, instance: %#x)\n", _self, instance);
	#endif // DEBUG

	const struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));
	assert(IS_INSTANCE_VALUE(instance));
	assert(FUNCTION_TYPE_METHOD == self->fun_type);

	void *mib_value = new(MibValue, self, copy_value(instance));

	#if defined(DEBUG)
		vi_log("ret method_bind(self: %x, instance: %#x): mib_value at %#x\n", _self, instance, mib_value);
	#endif // DEBUG

	return mib_value;
}

