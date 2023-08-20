#if !defined(expr_assign_id_h)
#define expr_assign_id_h

	#include "token.h"

	struct IdentifierAssignExpr {
		const void *type;

		const struct Token *id_token;
		const void *symbol;

		const struct Token *equal_token;

		void *right_expr;
	};

	extern const void *IdentifierAssignExpr;

	#define IS_IDENTIFIER_ASSIGN_EXPR(ptr) ((ptr) && (((struct IdentifierAssignExpr *) ptr)->type == IdentifierAssignExpr))

#endif // expr_assign_id_h
