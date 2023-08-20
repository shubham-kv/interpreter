#if !defined(expr_binary_h)
	#define expr_binary_h

	#include "token.h"

	struct BinaryExpr {
		const void *type;

		const struct Token *op_token;

		const void *left;

		const void *right;
	};

	extern const void *BinaryExpr;

	#define IS_BINARY_EXPR(ptr) ((ptr) && (((const struct BinaryExpr *) ptr)->type == BinaryExpr))

#endif // expr_binary_h
