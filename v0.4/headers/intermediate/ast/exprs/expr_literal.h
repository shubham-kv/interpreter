#if !defined(expr_literal_h)
	#define expr_literal_h

	#include "token.h"

	struct LiteralExpr {
		const void *type;

		const struct Token *literal_token;
	};

	extern const void *LiteralExpr;

	#define IS_LITERAL_EXPR(ptr) ((ptr) && (((const struct LiteralExpr *) ptr)->type == LiteralExpr))

#endif // expr_literal_h
