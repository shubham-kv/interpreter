#if !defined(expr_bitwise_h)
	#define expr_bitwise_h

	#include "token.h"

	struct BitwiseExpr {
		const void *type;

		const struct Token *op_token;

		const void *left;

		const void *right;
	};

	extern const void *BitwiseExpr;

	#define IS_BITWISE_EXPR(ptr) ((ptr) && (((const struct BitwiseExpr *) ptr)->type == BitwiseExpr))

#endif // expr_bitwise_h
