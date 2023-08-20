#if !defined(expr_identifier)
	#define expr_identifier

	#include <stdbool.h>

	#include "token.h"

	struct IdentifierExpr {
		const void *type;

		const struct Token *id_token;

		const void *symbol;
	};

	extern const void *IdentifierExpr;

	#define IS_IDENTIFIER_EXPR(ptr) ((ptr) && (((const struct IdentifierExpr *) ptr)->type == IdentifierExpr))

	#define IDENTIFIER_EXPR_GET_ID_TOKEN(iexpr) (((const struct IdentifierExpr *) iexpr)->id_token)
	#define IDENTIFIER_EXPR_GET_SYMBOL(iexpr) 	(((const struct IdentifierExpr *) iexpr)->symbol)

#endif // expr_identifier
