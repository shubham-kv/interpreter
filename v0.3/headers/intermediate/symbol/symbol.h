#if !defined(symbol_h)
#define symbol_h

	#include <stdbool.h>
	#include <stdint.h>
	#include "def_type.h"

	enum SymbolAttributeKey {
		KEY_AST,
		KEY_FUN,
		KEY_INIT_EXPR,
		// This is not a key, it's the total count of Symbol attribute keys and
		// shouldn't be used for indexing the arrays
		TOTAL_NO_OF_KEYS
	};

	// v0.2
	struct Symbol {
		const void* type;

		// the identifier of the symbol
		const char* id;

		// symbol_table to which this symbol belongs
		const void* symbol_table;

		// the definition type of the symbol
		enum DefinitionType definition_type;

		// the array of various attributes of the symbol
		// the indices of which are from 'enum SymbolAttributeKey'
		void** attributes;
	};

	extern const void* Symbol;

	#define IS_SYMBOL(ptr) ((ptr) && (((const struct Symbol *) ptr)->type == Symbol))

	#define SYMBOL_GET_ID(symbol) 			(((const struct Symbol *) symbol)->id)
	#define SYMBOL_GET_DEF_TYPE(symbol) 	(((const struct Symbol *) symbol)->definition_type)
	#define SYMBOL_GET_SYMBOL_TABLE(symbol) (((const struct Symbol *) symbol)->symbol_table)

	uint32_t symbol_get_nesting_level(const void* symbol);

	const void* symbol_get_attr(const void* symbol, enum SymbolAttributeKey key);
	void symbol_set_attr(const void* symbol, enum SymbolAttributeKey key, void* value);
	void symbol_delete_attr(const void* symbol, enum SymbolAttributeKey key);

#endif // symbol_h
