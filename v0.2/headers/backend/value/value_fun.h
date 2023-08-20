#if !defined(value_fun_h)
	#define value_fun_h

	#include <stdint.h>

	// An Enum to differentiate between normal funs and class' methods
	enum FunctionType {
		FUNCTION_TYPE_NONE,
		FUNCTION_TYPE_FUN,
		FUNCTION_TYPE_METHOD
	};

	struct FunValue {
		const void *type;

		enum FunctionType fun_type;

		const void *symbol;
		const void *symbol_table;

		uint32_t param_count;
		void **param_symbols;

		void *(*execute)(const void *self, void **args);

		void **declarations;

		uint32_t reference_count;
	};

	extern const void *FunValue;

	#define IS_FUN_VALUE(ptr) ((ptr) && ((FunValue) == ((struct FunValue *) ptr)->type))

	#define FUN_VALUE_GET_FUN_TYPE(ptr) (((struct FunValue *) ptr)->fun_type)

	void *method_bind(const void *method, const void *instance);

#endif // value_fun_h

