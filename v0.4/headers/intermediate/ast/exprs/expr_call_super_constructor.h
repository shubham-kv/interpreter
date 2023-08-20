#if !defined(expr_call_super_constructor_h)
#define expr_call_super_constructor_h

	#include <stdint.h>
	#include "token.h"

	struct SuperConstructorCallExpr {
		const void *type;

		const void *cur_instance_ptr_symbol;
		const void *super_class_symbol;

		const struct Token *super;
		const struct Token *lparen;

		uint32_t arg_count;
		void **args;

		const struct Token *rparen;
	};

	extern const void *SuperConstructorCallExpr;

	#define IS_SUPER_CONSTRUCTOR_CALL_EXPR(ptr) ((ptr) && (((const struct SuperConstructorCallExpr *) ptr)->type == SuperConstructorCallExpr))

#endif // expr_call_super_constructor_h
