#if !defined(stmt_continue_h)
#define stmt_continue_h

	#include "token.h"

	struct ContinueStmt {
		const void *type;

		const struct Token *continue_token;

		const struct Token *semicolon_token;
	};

	extern const void *ContinueStmt;

	#define IS_CONTINUE_STMT(ptr) ((ptr) && (((struct ContinueStmt *) ptr)->type == ContinueStmt))

#endif // stmt_continue_h
