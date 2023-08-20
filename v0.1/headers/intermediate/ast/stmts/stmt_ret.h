#if !defined(stmt_ret_h)
	#define stmt_ret_h

	#include <stdbool.h>

	struct RetStmt {
		const void *type;

		void *expression;
	};

	extern const void *RetStmt;

	#define IS_RET_STMT(obj) ((obj) && (((struct RetStmt *) obj)->type == RetStmt))
	// bool is_ret_stmt(const void *object);

#endif // stmt_ret_h
