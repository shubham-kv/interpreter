#if !defined(symbol_h)
	#define symbol_h

	#include <stdbool.h>
	#include <stdint.h>
	#include "def_type.h"


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

	extern const void *Symbol;

	extern const char *SYM_ATTR_KEY_AST;
	extern const char *SYM_ATTR_KEY_FUN;
	extern const char *SYM_ATTR_KEY_INIT_EXPR;


	#define IS_SYMBOL(obj) ((obj) && (((struct Symbol *) obj)->type == Symbol))

	#define SYMBOL_GET_ID(symbol) 			(((struct Symbol *) symbol)->id)
	#define SYMBOL_GET_DEF_TYPE(symbol) 	(((struct Symbol *) symbol)->definition_type)
	#define SYMBOL_GET_SYMBOL_TABLE(symbol) (((struct Symbol *) symbol)->symbol_table)


	uint32_t symbol_get_nesting_level(const void *symbol);

	const void *symbol_get_attr	(const void *symbol, const char *attr_key);
	void symbol_set_attr		(const void *symbol, const char *attr_key, const void *value);
	void symbol_delete_attr		(const void *symbol, const char *attr_key);

#endif // symbol_h
