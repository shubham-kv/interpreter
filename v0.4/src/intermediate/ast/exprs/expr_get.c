#include "expr_get.h"

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



static void *get_expr_constructor(void *self, va_list *ap);
static void *get_expr_destructor(void *self);

static void *get_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_get_expr(const void *object);

static const void *get_expr_get_lmost_token(const void *self);
static const void *get_expr_get_rmost_token(const void *self);

/*
static const struct AstNodeType _type_get_expr = {
	sizeof(struct GetExpr),
	get_expr_constructor,
	get_expr_destructor,
	get_expr_accept_visitor,
	is_get_expr,
	get_expr_get_lmost_token,
	get_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_get_expr = {
	.size 					= sizeof(struct GetExpr),
	.constructor 			= get_expr_constructor,
	.destructor 			= get_expr_destructor,

	.accept_visitor 		= get_expr_accept_visitor,
	.is_ast_node 			= is_get_expr,

	.get_leftmost_token 	= get_expr_get_lmost_token,
	.get_rightmost_token 	= get_expr_get_rmost_token,
	.get_tokens 			= NULL
};

const void *GetExpr = &_type_get_expr;


// void *bexpr = new(GetExpr, left_expr, property_token);

static void *get_expr_constructor(void *_self, va_list *ap) {
	struct GetExpr *self = _self;
	assert(IS_GET_EXPR(self));

	self->left_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->left_expr));

	self->property_token = va_arg(*ap, const void *);
	assert(IS_TOKEN(self->property_token));

	return _self;
}


static void *get_expr_destructor(void *_self) {
	struct GetExpr *self = _self;
	assert(IS_GET_EXPR(self));

	delete(self->left_expr);

	return _self;
}


static void *get_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct GetExpr *self = _self;
	assert(IS_GET_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_get_expr);

	return ast_visitor->visit_get_expr(_ast_visitor, _self);
}


static bool is_get_expr(const void *_object) {
	const struct GetExpr *object = _object;
	assert(object);

	return (object->type == GetExpr);
}


static const void *get_expr_get_lmost_token(const void *_self) {
	const struct GetExpr *self = _self;
	assert(IS_GET_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *get_expr_get_rmost_token(const void *_self) {
	const struct GetExpr *self = _self;
	assert(IS_GET_EXPR(self));

	return self->property_token;
}

