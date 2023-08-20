#if !defined(expr_get_h)
	#define expr_get_h

	#include "token.h"

	struct GetExpr {
		const void *type;

		void *left_expr;

		const struct Token *property_token;
	};

	extern const void *GetExpr;

	#define IS_GET_EXPR(ptr) ((ptr) && (((struct GetExpr *) ptr)->type == GetExpr))

#endif // expr_get_h
