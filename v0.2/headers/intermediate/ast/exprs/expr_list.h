#if !defined(expr_list_h)
	#define expr_list_h

	#include <stdint.h>
	#include "token.h"

	struct ListExpr {
		const void *type;

		const struct Token *lsbrace;

		uint32_t expr_count;
		void **exprs;

		const struct Token *rsbrace;
	};

	extern const void *ListExpr;

	#define IS_LIST_EXPR(obj) ((obj) && (((struct ListExpr *) obj)->type == ListExpr))

#endif // expr_list_h

