#include "symbol_table.h"

#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "type.h"
#include "hash_table.h"
#include "object.h"
#include "ast_object.h"
#include "symbol.h"


static void *symbol_table_constructor(void *self, va_list *ap);
static void *symbol_table_destructor(void *self);


static const struct Type _type_symbol_table = {
	sizeof(struct SymbolTable),
	symbol_table_constructor,
	symbol_table_destructor
};

const void *SymbolTable = &_type_symbol_table;


// ... = new(SymbolTable, 0);
// ... = new(SymbolTable, nesting_level);

static void *symbol_table_constructor(void *_self, va_list *ap) {
	struct SymbolTable *self = _self;
	assert(IS_SYMBOL_TABLE(self));

	self->nesting_level = (uint8_t) va_arg(*ap, int);
	self->entries = new(HashTable);

	return _self;
}

static void *symbol_table_destructor(void *_self) {
	struct SymbolTable *self = _self;
	assert(IS_SYMBOL_TABLE(self));

	delete((void *) self->entries);

	return _self;
}


// bool is_symbol_table(const void *_object) {
// 	const struct SymbolTable *object = _object;
// 	assert(object);
// 	return (object->type == SymbolTable);
// }

// uint8_t symbol_table_get_nesting_level(const void *_self) {
// 	const struct SymbolTable *self = _self;
// 	assert(IS_SYMBOL_TABLE(self));
// 	return self->nesting_level;
// }



void symbol_table_define(const void *_self, const char *id, const void *symbol) {
	const struct SymbolTable *self = _self;
	assert(IS_SYMBOL_TABLE(self));
	assert(IS_SYMBOL(symbol));
	assert(id);

	hashtable_put(self->entries, id, symbol);
}


const void *symbol_table_lookup(const void *_self, const char *id) {
	const struct SymbolTable *self = _self;
	assert(IS_SYMBOL_TABLE(self));
	assert(id);

	return hashtable_get(self->entries, id);
}


struct Entry *symbol_table_get_symbols(const void *_self) {
	const struct SymbolTable *self = _self;
	assert(IS_SYMBOL_TABLE(self));

	return hashtable_get_entries(self->entries);
}


