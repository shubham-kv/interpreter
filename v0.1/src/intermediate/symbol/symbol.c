#include "symbol.h"

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
#include "memory.h"

#include "symbol_table.h"
#include "def_type.h"




const char *SYM_ATTR_KEY_AST				= "AST";
const char *SYM_ATTR_KEY_FUN				= "FUN";
const char *SYM_ATTR_KEY_INIT_EXPR			= "INIT_EXPR";

/*
var number = 17;

Symbol {
	symbol_table: SymbolTable {
		nesting_level: 0
	}
	id: "number",
	def_type: DEF_TYPE_VARIABLE,
	attributes: {
		SYM_ATTR_KEY_INIT_EXPR: LiteralExpr {
			literal_token: Token { start: "1", length: 2 }
		}
	}
}


const add = (a, b) -> {
	ret a + b;
}

fun add(a, b) {
	ret a + b;
}

Symbol {
	symbol_table: SymbolTable<0> { nesting_level: 0 },
	id: "add",
	def_type: DEF_TYPE_FUN,
	attributes: {
		SYMBOL_AST: FunDecl {
			symbol_table: SymbolTable<1> {
				nesting_level: 1,
				entries: {
					"a": Symbol {
						symbol_table: SymbolTable<1>,
						id: "a",
						def_type: DEF_TYPE_VAR,
						attrs: {
							SYMBOL_EXPR: LiteralExpr {
								literal_token: NULL,
							}
						}
					}
					"b": Symbol {
						symbol_table: SymbolTable<1>,
						id: "b",
						def_type: DEF_TYPE_VAR,
						attrs: {
							SYMBOL_NESTING_LEVEL: 1,
							SYMBOL_EXPR: LiteralExpr {
								literal_token: NULL,
							}
						}
					}
				}
			}
		}
	}
}
*/


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


static void *symbol_constructor(void *_self, va_list *ap) {
	struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	self->id = va_arg(*ap, char *);
	assert(self->id && (strlen(self->id) > 0));

	self->symbol_table = va_arg(*ap, const void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->definition_type = va_arg(*ap, enum DefinitionType);
	self->attributes = new(HashTable);

	return _self;
}

static void *symbol_destructor(void *_self) {
	struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	free((char *) self->id);
	delete((void *) self->attributes);

	return _self;
}



uint32_t symbol_get_nesting_level(const void *_self) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	return SYMBOL_TABLE_GET_NESTING_LEVEL(self->symbol_table);
}


void symbol_set_attr(const void *_self, const char *attr_key, const void *value) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	hashtable_put(self->attributes, attr_key, value);
}


const void *symbol_get_attr(const void *_self, const char *attr_key) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	return hashtable_get(self->attributes, attr_key);
}

void symbol_delete_attr(const void *_self, const char *attr_key) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	hashtable_delete(self->attributes, attr_key);
}





void symbol_printer(const void *_self) {
	const struct Symbol *self = _self;
	assert(IS_SYMBOL(self));

	struct Entry *entries = hashtable_get_entries(self->attributes);

	printf("Symbol {\n");
	printf("    id: '%s',\n", self->id);
	printf("    symbol_table: %x,\n", self->symbol_table);
	printf("    def_type: %d,\n", self->definition_type);

	if(NULL == entries->key) {
		printf("    attributes: { }\n");
	}
	else {
		printf("    attributes: {\n");

		for(uint32_t i = 0; NULL != (entries + i)->key; i++)
			printf("        '%s': %x\n", (entries + i)->key, (entries + i)->value);

		printf("    }\n");
	}

	printf("}\n");
}


