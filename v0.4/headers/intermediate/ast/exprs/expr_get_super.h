#if !defined(expr_get_super_h)
#define expr_get_super_h

	#include <stdint.h>
	#include "token.h"

	struct SuperGetExpr {
		const void *type;

		const void *cur_instance_ptr_symbol;
		const void *super_class_symbol;

		const struct Token *super_token;
		const struct Token *property_token;
	};

	extern const void *SuperGetExpr;

	#define IS_SUPER_GET_EXPR(ptr) ((ptr) && (((const struct SuperGetExpr *) ptr)->type == SuperGetExpr))

#endif // expr_get_super_h
