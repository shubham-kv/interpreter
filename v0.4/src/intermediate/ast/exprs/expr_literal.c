#include "expr_literal.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "ast_object.h"

#include "ast_visitor.h"


static void *literal_expr_constructor(void *self, va_list *ap);
static void *literal_expr_destructor(void *self);

static void *literal_expr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_literal_expr(const void *object);

static const void *literal_expr_get_lmost_token(const void *self);
static const void *literal_expr_get_rmost_token(const void *self);

/*
static const struct AstNodeType _type_literal_expr = {
	sizeof(struct LiteralExpr),
	literal_expr_constructor,
	literal_expr_destructor,
	literal_expr_accept_visitor,
	is_literal_expr,
	literal_expr_get_lmost_token,
	literal_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_literal_expr = {
	.size 					= sizeof(struct LiteralExpr),
	.constructor 			= literal_expr_constructor,
	.destructor 			= literal_expr_destructor,

	.accept_visitor 		= literal_expr_accept_visitor,
	.is_ast_node 			= is_literal_expr,

	.get_leftmost_token 	= literal_expr_get_lmost_token,
	.get_rightmost_token 	= literal_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *LiteralExpr = &_type_literal_expr;


// ... = new(LiteralExpr, literal_token);

static void *literal_expr_constructor(void *_self, va_list *ap) {
	struct LiteralExpr *self = _self;
	assert(IS_LITERAL_EXPR(self));

	self->literal_token = va_arg(*ap, const struct Token *);

	return _self;
}


static void *literal_expr_destructor(void *_self) {
	return _self;
}


static void *literal_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct LiteralExpr *self = _self;
	assert(IS_LITERAL_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_literal_expr);

	return ast_visitor->visit_literal_expr(_ast_visitor, _self);
}


static bool is_literal_expr(const void *_object) {
	const struct LiteralExpr *object = _object;
	assert(object);

	return (object->type == LiteralExpr);
}


static const void *literal_expr_get_lmost_token(const void *_self) {
	const struct LiteralExpr *self = _self;
	assert(IS_LITERAL_EXPR(self));

	return self->literal_token;
}

static const void *literal_expr_get_rmost_token(const void *_self) {
	const struct LiteralExpr *self = _self;
	assert(IS_LITERAL_EXPR(self));

	return self->literal_token;
}

/*
void literal_expr_type_printer(const void *_self) {
	const struct LiteralExpr *self = _self;
	assert(IS_LITERAL_EXPR(self));

	printf("LiteralExpr {\n");
	printf("  at: %#x", self);
	printf("}");
}
*/


