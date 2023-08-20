#if !defined(symbol_h)
	#define symbol_h

	#include <stdbool.h>
	#include <stdint.h>
	#include "def_type.h"


	/*
	// v0.1
	struct Symbol {
		const void *type;

		// the identifier of the symbol
		const char *id;

		// symbol_table to which this symbol belongs
		const void *symbol_table;

		// the definition type of the symbol
		enum DefinitionType definition_type;

		// the hashtable of various attributes of the symbol
		void *attributes;
	};
	*/

	// v0.2
	struct Symbol {
		const void *type;

		// the identifier of the symbol
		const char *id;

		// symbol_table to which this symbol belongs
		const void *symbol_table;

		// the definition type of the symbol
		enum DefinitionType definition_type;

		// the array of various attributes of the symbol
		void **attributes;
	};

	extern const void *Symbol;

	// Note:
	// you barely use the attributes hashtable as a hashtable
	// you do not use it to store more than 1 or 2 attributes
	// so why dont you use a list instead and directly index the list to get those one or two values


	// extern const char *SYM_ATTR_KEY_AST;
	// extern const char *SYM_ATTR_KEY_FUN;
	// extern const char *SYM_ATTR_KEY_INIT_EXPR;



	enum SymbolAttributeKey {
		KEY_AST,
		KEY_FUN,
		KEY_INIT_EXPR,

		// This is not a key, it's the total count of Symbol attribute keys and
		// shouldn't be used for indexing the arrays
		TOTAL_NO_OF_KEYS
	};
	


	#define IS_SYMBOL(obj) ((obj) && (((struct Symbol *) obj)->type == Symbol))

	#define SYMBOL_GET_ID(symbol) 			(((struct Symbol *) symbol)->id)
	#define SYMBOL_GET_DEF_TYPE(symbol) 	(((struct Symbol *) symbol)->definition_type)
	#define SYMBOL_GET_SYMBOL_TABLE(symbol) (((struct Symbol *) symbol)->symbol_table)


	uint32_t symbol_get_nesting_level(const void *symbol);

	// const void *symbol_get_attr(const void *symbol, const char *attr_key);
	// void symbol_set_attr(const void *symbol, const char *attr_key, const void *value);
	// void symbol_delete_attr(const void *symbol, const char *attr_key);


	const void *symbol_get_attr(const void *symbol, enum SymbolAttributeKey key);
	void symbol_set_attr(const void *symbol, enum SymbolAttributeKey key, void *value);
	void symbol_delete_attr(const void *symbol, enum SymbolAttributeKey key);

	// void print_symbol(const void *symbol);

#endif // symbol_h
