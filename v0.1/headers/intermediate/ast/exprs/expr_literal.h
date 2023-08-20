#if !defined(expr_literal_h)
	#define expr_literal_h

	// #include "value.h"
	#include "token.h"

	struct LiteralExpr {
		const void *type;

		const struct Token *literal_token;

		// const struct Value *value;
	};

	extern const void *LiteralExpr;

	#define IS_LITERAL_EXPR(obj) ((obj) && (((struct LiteralExpr *) obj)->type == LiteralExpr))

#endif // expr_literal_h
