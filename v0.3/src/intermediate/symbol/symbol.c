#include "symbol.h"

#include <assert.h>

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdbool.h>

#include "hash_table.h"
#include "my_memory.h"
#include "object.h"
#include "type.h"

#include "def_type.h"
#include "symbol_table.h"

#include "log.h"


static void *symbol_constructor(void *self, va_list *ap);
static void *symbol_destructor(void *self);

static const struct Type _type_symbol = {
	sizeof(struct Symbol),
	symbol_constructor,
	symbol_destructor
};

const void *Symbol = &_type_symbol;



/*
const void *symbol_table = symtabstack_push();

void *symbol = new(Symbol, "name", symbol_table, DEF_TYPE_VARIABLE);
symbol_set_attr(symbol, SYM_ATTR_KEY_INIT_EXPR, init_expr);

symbol_table_define(symbol_table, "name", symbol);

symtabstack_pop();
*/

// void *id_symbol = new(Symbol, id_in_heap, symbol_table, def_type);
// symbol_set_attr(id_symbol, SYMBOL_ATTR_KEY_EXPR, literal_expr);

// v0.2
static void *symbol_constructor(void *_self, va_list *ap) {
	#if defined(DEBUG)
		vi_log("symbol_constructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	self->id = va_arg(*ap, char *);
	assert(self->id && (strlen(self->id) > 0));

	self->symbol_table = va_arg(*ap, const void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->definition_type = va_arg(*ap, enum DefinitionType);
	self->attributes = va_arg(*ap, void **);

	#if defined(DEBUG)
		vi_log("ret symbol_constructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}


// v0.1
static void *symbol_destructor(void *_self) {
	#if defined(DEBUG)
		vi_log("symbol_destructor(self: %x)\n", _self);
	#endif // DEBUG

	struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	free((char *) self->id);

	if(NULL != self->attributes) {
		for(int i = 0; i < TOTAL_NO_OF_KEYS; i++) {
			if(NULL != self->attributes[i]) {
				delete(self->attributes[i]);
			}
		}
	}

	#if defined(DEBUG)
		vi_log("ret symbol_destructor(self: %x)\n", _self);
	#endif // DEBUG

	return _self;
}



uint32_t symbol_get_nesting_level(const void *_self) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	return SYMBOL_TABLE_GET_NESTING_LEVEL(self->symbol_table);
}

const void *symbol_get_attr(const void *_self, enum SymbolAttributeKey key) {
	#if defined(DEBUG)
		vi_log("symbol_get_attr(self: %x, key: %d)\n", _self, key);
	#endif // DEBUG

	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));
	assert(key >= 0 && key < TOTAL_NO_OF_KEYS);

	if((NULL == self->attributes) || (NULL == self->attributes[key])) {
		#if defined(DEBUG)
			vi_log("ret symbol_get_attr(): NULL\n\n");
		#endif // DEBUG

		return NULL;
	}

	const void *attribute = self->attributes[key];

	#if defined(DEBUG)
		vi_log("ret symbol_get_attr(): Attribute at %x\n\n", attribute);
	#endif // DEBUG
	
	return attribute;
}


void symbol_set_attr(const void *_self, enum SymbolAttributeKey key, void *value) {
	#if defined(DEBUG)
		vi_log("symbol_set_attr(self: %x, key: %d, value: %x)\n", _self, key, value);
	#endif // DEBUG

	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));
	assert(key >= 0 && key < TOTAL_NO_OF_KEYS);

	if(NULL == self->attributes) {
		#if defined(DEBUG)
			vi_log("ret symbol_set_attr(): Attributes array was null.\n\n");
		#endif // DEBUG

		return;
	}
	
	if(NULL != self->attributes[key])
		delete(self->attributes[key]);
	
	self->attributes[key] = value;

	#if defined(DEBUG)
		vi_log("ret symbol_set_attr()\n\n");
	#endif // DEBUG
}

void symbol_delete_attr(const void *_self, enum SymbolAttributeKey key) {
	#if defined(DEBUG)
		vi_log("symbol_delete_attr(self: %x, key: %d)\n", _self, key);
	#endif // DEBUG

	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));
	assert(key >= 0 && key < TOTAL_NO_OF_KEYS);

	if(NULL == self->attributes) {
		#if defined(DEBUG)
			vi_log("ret symbol_delete_attr(): Attributes array was null.\n\n");
		#endif // DEBUG

		return;
	}

	if(NULL != self->attributes[key])
		delete(self->attributes[key]);
	
	self->attributes[key] = NULL;

	#if defined(DEBUG)
		vi_log("ret symbol_delete_attr(self: %x, key: %d): success\n\n", _self, key);
	#endif // DEBUG
}




// void print_symbol(const void *_self) {
// 	const struct Symbol *self = _self;
// 	assert(IS_SYMBOL(self));

// 	struct Entry *entries = hashtable_get_entries(self->attributes);

// 	printf("Symbol {\n");
// 	printf("    id: '%s',\n", self->id);
// 	printf("    symbol_table: %x,\n", self->symbol_table);
// 	printf("    def_type: %d,\n", self->definition_type);

// 	if(NULL == entries->key) {
// 		printf("    attributes: { }\n");
// 	}
// 	else {
// 		printf("    attributes: {\n");

// 		for(uint32_t i = 0; NULL != (entries + i)->key; i++)
// 			printf("        '%s': %x\n", (entries + i)->key, (entries + i)->value);

// 		printf("    }\n");
// 	}

// 	printf("}\n");
// }


