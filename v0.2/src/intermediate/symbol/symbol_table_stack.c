#include "symbol_table_stack.h"

#include <assert.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#include "log.h"
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
	#if defined(DEBUG)
		vi_log("symtabstack_constructor(self: %x)\n", _self);
	#endif // DEBUG
	
	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	self->stack_size = 0;
	self->stack = NULL;

	#if defined(DEBUG)
		vi_log("ret symtabstack_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return self;
}

static void *symtabstack_destructor(void *_self) {

	#if defined(DEBUG)
		vi_log("symtabstack_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(0 != self->stack_size) {
		#if defined(DEBUG)
			vi_log("symtabstack_destructor(): Symbol table stack not freed!\n");
		#endif // DEBUG

		exit(EXIT_FAILURE);
	}

	if(NULL != self->stack)
		free(self->stack);
	
	#if defined(DEBUG)
		vi_log("ret symtabstack_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return self;
}





const void *symtabstack_get_local_table(const void *_self) {
	#if defined(DEBUG)
		vi_log("symtabstack_get_local_table(self: %x)\n", _self);
	#endif // DEBUG

	const struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1) return NULL;

	const void *symbol_table = self->stack[self->stack_size - 1];

	#if defined(DEBUG)
		vi_log("ret symtabstack_get_local_table(self: %x): symbol table at %x\n\n", _self, symbol_table);
	#endif // DEBUG

	return symbol_table;
}


const void *symtabstack_push(void *_self) {

	#if defined(DEBUG)
		vi_log("symtabstack_push(selF: %x)\n", _self);
	#endif // DEBUG

	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	self->stack = reallocate(self->stack, sizeof(void *), ++self->stack_size);

	// void *symbol_table = (void *) new(SymbolTable, self->stack_size - 2);	// -1 for getting index and another -1 for predefined symbol table
	void *symbol_table = (void *) new(SymbolTable, self->stack_size - 1);
	self->stack[self->stack_size - 1] = symbol_table;

	#if defined(DEBUG)
		vi_log("ret symtabstack_push(selF: %x): symbol table at %x\n\n", _self, symbol_table);
	#endif // DEBUG

	return symbol_table;
}


const void *symtabstack_pop(void *_self) {

	#if defined(DEBUG)
		vi_log("symtabstack_pop(self: %x)\n", _self);
	#endif // DEBUG

	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1)
		return NULL;

	const void *symbol_table = self->stack[self->stack_size - 1];
	self->stack = reallocate(self->stack, sizeof(void *), --self->stack_size);

	#if defined(DEBUG)
		vi_log("ret symtabstack_pop(self: %x): symbol table at %x\n\n", _self, symbol_table);
	#endif // DEBUG

	return symbol_table;
}


void symtabstack_define(void *_self, const char *id, const void *symbol) {

	#if defined(DEBUG)
		vi_log("symtabstack_define(self: %x)\n", _self);
	#endif // DEBUG

	struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));
	assert(IS_SYMBOL(symbol));
	assert(id);

	void *local_symbol_table = self->stack[self->stack_size - 1];
	symbol_table_define(local_symbol_table, id, symbol);

	#if defined(DEBUG)
		vi_log("ret symtabstack_define(self: %x)\n\n", _self);
	#endif // DEBUG

	return;
}


const void *symtabstack_lookup(const void *_self, const char *id) {

	#if defined(DEBUG)
		vi_log("symtabstack_lookup(self: %x)\n", _self);
	#endif // DEBUG

	const struct SymbolTableStack *self = _self;
	assert(IS_SYMBOL_TABLE_STACK(self));

	if(self->stack_size < 1) return NULL;

	const void *symbol = NULL;

	for(int i = self->stack_size - 1; i > -1; i--) {
		symbol = symbol_table_lookup(self->stack[i], id);

		if(NULL != symbol) {

			#if defined(DEBUG)
				vi_log("ret symtabstack_lookup(self: %x): symbol at %x\n\n", _self, symbol);
			#endif // DEBUG

			return symbol;
		}
	}

	#if defined(DEBUG)
		vi_log("ret symtabstack_lookup(self: %x): NULL\n\n", _self);
	#endif // DEBUG

	return NULL;
}



