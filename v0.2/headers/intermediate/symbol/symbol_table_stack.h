#if !defined(symbol_table_stack_h)
	#define symbol_table_stack_h

	#include <stdbool.h>
	#include <stdint.h>

	struct SymbolTableStack {
		const void *type;

		uint8_t stack_size;

		void **stack;
	};

	extern const void *SymbolTableStack;

	#define IS_SYMBOL_TABLE_STACK(obj) ((obj) && (((struct SymbolTableStack *) obj)->type == SymbolTableStack))
	#define SYMBOL_TABLE_STACK_GET_SIZE(obj) (((struct SymbolTableStack *) obj)->stack_size)

	const void *symtabstack_get_local_table(const void *sym_tab_stack);

	const void *symtabstack_push(void *sym_tab_stack);
	const void *symtabstack_pop(void *sym_tab_stack);

	void symtabstack_define(void *sym_tab_stack, const char *id, const void *symbol);
	const void *symtabstack_lookup(const void *sym_tab_stack, const char *id);

#endif // symbol_table_stack_h
