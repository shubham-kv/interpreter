#if !defined(expr_logical_h)
	#define expr_logical_h

	#include "token.h"

	struct LogicalExpr {
		const void *type;

		const struct Token *op_token;

		const void *left_expr;

		const void *right_expr;
	};

	extern const void *LogicalExpr;

	#define IS_LOGICAL_EXPR(obj) ((obj) && (((struct LogicalExpr *) obj)->type == LogicalExpr))

#endif // expr_logical_h
