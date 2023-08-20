#if !defined(expr_set_h)
#define expr_set_h

	#include "token.h"

	struct SetExpr {
		const void *type;

		void *left_expr;
		const struct Token *property_token;

		const struct Token *equal_token;

		void *right_expr;
	};

	extern const void *SetExpr;

	#define IS_SET_EXPR(ptr) ((ptr) && (((const struct SetExpr *) ptr)->type == SetExpr))

#endif // expr_set_h

