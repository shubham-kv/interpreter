#include "symbol_table_stack.h"

#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol_table.h"
#include "symbol.h"

#include "memory.h"


static void *symtabstack_constructor(void *self, va_list *ap);
static void *symtabstack_destructor(void *self);

static const struct Type _type_symtabstack = {
	sizeof(struct SymbolTableStack),
	symtabstack_constructor,
	symtabstack_destructor
};

const void *SymbolTableStack = &_type_symtabstack;


static void *symtabstack_constructor(void *_self, va_list *ap) {
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	self->stack_size = 0;
	self->stack = NULL;

	return self;
}

static void *symtabstack_destructor(void *_self) {
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(0 != self->stack_size) {
		fprintf(stderr, "Symbol table stack not freed!\n");
		exit(EXIT_FAILURE);
	}

	if(NULL != self->stack)
		free(self->stack);
	
	return self;
}

// bool is_sym_tab_stack(const void *_object) {
// 	const struct SymbolTableStack *object = _object;
// 	assert(object);
// 	return (object->class == SymbolTableStack);
// }

// uint32_t symtabstack_get_size(const void *_self) {
// 	const struct SymbolTableStack *self = _self;
// 	assert(IS_SYMBOL_TABLE_STACK(self));
// 	return self->stack_size;
// }



const void *symtabstack_get_local_table(const void *_self) {
	const struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1) return NULL;

	return self->stack[self->stack_size - 1];
}


const void *symtabstack_push(void *_self) {
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	self->stack = reallocate(self->stack, sizeof(void *), ++self->stack_size);

	// void *symbol_table = (void *) new(SymbolTable, self->stack_size - 2);	// -1 for getting index and another -1 for predefined symbol table
	void *symbol_table = (void *) new(SymbolTable, self->stack_size - 1);
	self->stack[self->stack_size - 1] = symbol_table;

	return symbol_table;
}


const void *symtabstack_pop(void *_self) {
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1)
		return NULL;

	const void *symbol_table = self->stack[self->stack_size - 1];
	self->stack = reallocate(self->stack, sizeof(void *), --self->stack_size);

	return symbol_table;
}


void symtabstack_define(void *_self, const char *id, const void *symbol) {
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));
	assert(IS_SYMBOL(symbol));
	assert(id);

	void *local_symbol_table = self->stack[self->stack_size - 1];
	symbol_table_define(local_symbol_table, id, symbol);
}


const void *symtabstack_lookup(const void *_self, const char *id) {
	const struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1) return NULL;

	const void *symbol = NULL;

	for(int i = self->stack_size - 1; i > -1; i--) {
		symbol = symbol_table_lookup(self->stack[i], id);

		if(NULL != symbol)
			return symbol;
	}

	return NULL;
}



