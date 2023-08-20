#if !defined(value_fun_h)
	#define value_fun_h

	#include <stdint.h>

	struct FunValue {
		const void *type;

		const void *symbol;
		const void *symbol_table;

		uint32_t param_count;
		void **param_symbols;

		void *(*execute)(const void *self, void **args);

		// the list of statements to execute
		void **declarations;

		uint32_t reference_count;
	};

	extern const void *FunValue;

	#define IS_FUN_VALUE(ptr) ((ptr) && ((FunValue) == ((struct FunValue *) ptr)->type))


	#define FUN_VALUE_GET_SYMBOL(fun_value_ptr) 		(((struct FunValue *) fun_value_ptr)->symbol)

	#define FUN_VALUE_GET_SYMBOL_TABLE(fun_value_ptr) 	(((struct FunValue *) fun_value_ptr)->symbol_table)

	#define FUN_VALUE_GET_PARAM_COUNT(fun_value_ptr) 	(((struct FunValue *) fun_value_ptr)->param_count)

	#define FUN_VALUE_GET_PARAM_SYMBOLS(fun_value_ptr)	(((struct FunValue *) fun_value_ptr)->param_symbols)

	#define FUN_VALUE_GET_DECLARATIONS(fun_value_ptr) 	(((struct FunValue *) fun_value_ptr)->declarations)

	#define FUN_VALUE_GET_EXECUTE_FUN(fun_value_ptr) 	(((struct FunValue *) fun_value_ptr)->execute)

#endif // value_fun_h

