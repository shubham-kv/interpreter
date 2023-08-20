#if !defined(expr_call_h)
	#define expr_call_h

	#include <stdint.h>
	#include "token.h"

	struct CallExpr {
		const void *type;

		// the expr that would evaluate to 'struct FunValue' at runtime
		void *left_expr;

		const struct Token *lparen;

		uint32_t arg_count;
		void **args;

		const struct Token *rparen;
	};

	extern const void *CallExpr;

	#define IS_CALL_EXPR(ptr) ((ptr) && (((const struct CallExpr *) ptr)->type == CallExpr))

#endif // expr_call_h
