#if !defined(expr_list_subscript_h)
	#define expr_list_subscript_h

	struct ListSubscriptExpr {
		const void *type;

		void *left_expr;

		const struct Token *lsbrace;

		void *index_expr;

		const struct Token *rsbrace;
	};

	extern const void *ListSubscriptExpr;

	#define IS_LIST_SUBSCRIPT_EXPR(ptr) ((ptr) && (((const struct ListSubscriptExpr *) ptr)->type == ListSubscriptExpr))

#endif // expr_list_subscript_h
