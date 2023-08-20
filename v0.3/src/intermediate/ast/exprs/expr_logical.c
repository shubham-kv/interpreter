#include "expr_logical.h"

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



static void *logical_expr_constructor(void *self, va_list *ap);
static void *logical_expr_destructor(void *self);

static void *logical_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_logical_expr(const void *object);

static const void *logical_expr_get_lmost_token(const void *self);
static const void *logical_expr_get_rmost_token(const void *self);

/*
static const struct AstNodeType _type_logical_expr = {
	sizeof(struct LogicalExpr),
	logical_expr_constructor,
	logical_expr_destructor,
	logical_expr_accept_visitor,
	is_logical_expr,
	logical_expr_get_lmost_token,
	logical_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_logical_expr = {
	.size 					= sizeof(struct LogicalExpr),
	.constructor 			= logical_expr_constructor,
	.destructor 			= logical_expr_destructor,

	.accept_visitor 		= logical_expr_accept_visitor,
	.is_ast_node 			= is_logical_expr,

	.get_leftmost_token 	= logical_expr_get_lmost_token,
	.get_rightmost_token 	= logical_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *LogicalExpr = &_type_logical_expr;


// void *bexpr = new(LogicalExpr, op_token, left, right);

static void *logical_expr_constructor(void *_self, va_list *ap) {
	struct LogicalExpr *self = _self;
	assert(IS_LOGICAL_EXPR(self));

	self->op_token = va_arg(*ap, const struct Token *);

	self->left_expr = va_arg(*ap, const void *);
	assert(is_ast_node(self->left_expr));

	self->right_expr = va_arg(*ap, const void *);
	assert(is_ast_node(self->right_expr));

	return _self;
}


static void *logical_expr_destructor(void *_self) {
	struct LogicalExpr *self = _self;
	assert(IS_LOGICAL_EXPR(self));

	delete((void *) self->left_expr);
	delete((void *) self->right_expr);

	return _self;
}


static void *logical_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct LogicalExpr *self = _self;
	assert(IS_LOGICAL_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_logical_expr);

	return ast_visitor->visit_logical_expr(_ast_visitor, _self);
}


static bool is_logical_expr(const void *_object) {
	const struct LogicalExpr *object = _object;
	assert(object);

	return (object->type == LogicalExpr);
}



static const void *logical_expr_get_lmost_token(const void *_self) {
	const struct LogicalExpr *self = _self;
	assert(IS_LOGICAL_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *logical_expr_get_rmost_token(const void *_self) {
	const struct LogicalExpr *self = _self;
	assert(IS_LOGICAL_EXPR(self));

	return get_rightmost_token(self->right_expr);
}

