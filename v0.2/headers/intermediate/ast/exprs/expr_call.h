#if !defined(expr_call_h)
	#define expr_call_h

	#include <stdint.h>

	#include "token.h"

	// v1.0
	// struct CallExpr {
	// 	const void *type;
	// 	const struct Token *fun_id_token;
	// 	const void *fun_symbol;
	// 	uint32_t arg_count;
	// 	void **args;
	// };

	// v1.1
	// struct CallExpr {
	// 	const void *type;
	// 	// the expr that would evaluate to 'struct FunValue' at runtime
	// 	void *left_expr;
	// 	uint32_t arg_count;
	// 	void **args;
	// };


	// v1.2 (02:20pm 01/03/2022)
	struct CallExpr {
		const void *type;

		// the expr that would evaluate to 'struct FunValue' at runtime
		void *left_expr;

		const struct Token *lparen;

		// count and list of arg exprs
		uint32_t arg_count;
		void **args;

		const struct Token *rparen;
	};

	extern const void *CallExpr;

	#define IS_CALL_EXPR(obj) ((obj) && (((struct CallExpr *) obj)->type == CallExpr))

#endif // expr_call_h
