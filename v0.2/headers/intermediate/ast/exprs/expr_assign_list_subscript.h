#if !defined(expr_assign_list_subscript_h)
#define expr_assign_list_subscript_h

	#include "token.h"

	struct ListSubscriptAssignExpr {
		const void *type;

		void *left_expr;
		void *index_expr;

		const struct Token *equal_token;

		void *right_expr;
	};

	extern const void *ListSubscriptAssignExpr;

	#define IS_LIST_SUBSCRIPT_ASSIGN_EXPR(ptr) ((ptr) && (((struct ListSubscriptAssignExpr *) ptr)->type == ListSubscriptAssignExpr))

#endif // expr_assign_list_subscript_h
