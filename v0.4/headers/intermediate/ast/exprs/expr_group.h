#if !defined(expr_group_h)
	#define expr_group_h

	#include "token.h"

	struct GroupExpr {
		const void *type;

		const struct Token *lparen;

		const void *child_expr;

		const struct Token *rparen;
	};

	extern const void *GroupExpr;

	#define IS_GROUP_EXPR(ptr) ((ptr) && (((const struct GroupExpr *) ptr)->type == GroupExpr))

#endif // expr_group_h
