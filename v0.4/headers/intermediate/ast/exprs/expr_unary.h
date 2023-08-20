#if !defined(expr_unary_h)
	#define expr_unary_h

	#include "token.h"

	struct UnaryExpr {
		const void *type;

		const struct Token *op_token;

		const void *right;
	};

	extern const void *UnaryExpr;

	#define IS_UNARY_EXPR(ptr) ((ptr) && (((const struct UnaryExpr *) ptr)->type == UnaryExpr))

#endif // expr_unary_h
