#if !defined(stmt_expr_h)
	#define stmt_expr_h

	struct ExprStmt {
		const void *type;

		const void *expr;
	};

	extern const void *ExprStmt;

	#define IS_EXPR_STMT(obj) ((obj) && (((struct ExprStmt *) obj)->type == ExprStmt))

#endif // stmt_expr_h
