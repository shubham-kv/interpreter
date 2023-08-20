#if !defined(stmt_if_h)
	#define stmt_if_h

	struct IfStmt {
		const void *type;

		const void *expr;

		const void *if_block;

		const void *else_block;
	};

	extern const void *IfStmt;

	#define IS_IF_STMT(obj) ((obj) && (((struct IfStmt *) obj)->type == IfStmt))

#endif // stmt_if_h
