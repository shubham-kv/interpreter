#if !defined(expr_var_assign_h)
	#define expr_var_assign_h

	#include "symbol.h"
	#include "token.h"

	enum LValueType {
		LVALUE_VARIABLE,
		LVALUE_LIST_SUBSCRIPT
	};

	// struct AssignExpr {
	// 	const void *type;
	// 	const struct Symbol *id_symbol;
	// 	const struct Token *equal_token;
	// 	const void *right_expr;
	// };

	struct AssignExpr {
		const void *type;

		enum LValueType lvalue_type;

		const struct Token *id_token;
		const struct Token *equal_token;

		const struct Symbol *id_symbol;

		void *index_expr;

		void *right_expr;
	};

	extern const void *AssignExpr;

	#define IS_ASSIGN_EXPR(obj) ((obj) && (((struct AssignExpr *) obj)->type == AssignExpr))
	
#endif // expr_var_assign_h
