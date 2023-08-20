#if !defined(expr_list_subscript_h)
	#define expr_list_subscript_h

	// v1.0
	// struct ListSubscriptExpr {
	// 	const void *type;
	// 	const void *id_token;
	// 	const void *symbol;
	// 	void *index_expr;
	// };

	// v1.1
	struct ListSubscriptExpr {
		const void *type;

		void *left_expr;

		const struct Token *lsbrace;

		void *index_expr;

		const struct Token *rsbrace;
	};

	extern const void *ListSubscriptExpr;

	#define IS_LIST_SUBSCRIPT_EXPR(obj) ((obj) && (((struct ListSubscriptExpr *) obj)->type == ListSubscriptExpr))

#endif // expr_list_subscript_h
