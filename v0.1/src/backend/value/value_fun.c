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

#include "symbol.h"
#include "symbol_table.h"


void *fun_value_constructor(void *self, va_list *ap);
void *fun_value_destructor(void *self);

bool is_fun_value(const void *self);

void *fun_value_copy_value(const void *self);
bool fun_value_is_value_true(const void *self);

void fun_value_print_value(const void *self);


static const struct ValueBaseType _type_fun_value = {
	sizeof(struct FunValue),
	fun_value_constructor,
	fun_value_destructor,
	is_fun_value,

	fun_value_print_value, fun_value_copy_value, fun_value_is_value_true, NULL,
	NULL, NULL, NULL, NULL
};

const void *FunValue = &_type_fun_value;


// ... = new(FunValue, fun_value_symbol, fun_value_symbol_table, param_count, param_symbols, executor_fun_value, declarations);

void *fun_value_constructor(void *_self, va_list *ap) {
	struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

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

	return _self;
}

void *fun_value_destructor(void *_self) {
	struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

	if(self->reference_count > 0) {
		self->reference_count--;
		return NULL;
	}

	return _self;
}


bool is_fun_value(const void *object) {
	return IS_FUN_VALUE(object);
}

void *fun_value_copy_value(const void *_self) {
	struct FunValue *self = (struct FunValue *) _self;
	assert(IS_FUN_VALUE(self));

	self->reference_count++;

	return self;
}

bool fun_value_is_value_true(const void *self) {
	return true;
}



void fun_value_print_value(const void *_self) {
	const struct FunValue *self = _self;
	assert(IS_FUN_VALUE(self));

	printf("<fun_value '%s'>", SYMBOL_GET_ID(self->symbol));
}

