#include "expr_set.h"

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


static void *set_expr_constructor(void *self, va_list *ap);
static void *set_expr_destructor(void *self);

static void *set_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_set_expr(const void *object);

static const void *set_expr_get_lmost_token(const void *self);
static const void *set_expr_get_rmost_token(const void *self);



static const struct AstNodeType _type_set_expr = {
	.size 					= sizeof(struct SetExpr),
	.constructor 			= set_expr_constructor,
	.destructor 			= set_expr_destructor,

	.accept_visitor 		= set_expr_accept_visitor,
	.is_ast_node 			= is_set_expr,

	.get_leftmost_token 	= set_expr_get_lmost_token,
	.get_rightmost_token 	= set_expr_get_rmost_token,
	.get_tokens 			= NULL
};

const void *SetExpr = &_type_set_expr;


// ... = new(SetExpr, left_expr, property_token, equal_token, right_expr);

static void *set_expr_constructor(void *_self, va_list *ap) {
	struct SetExpr *self = _self;
	assert(IS_SET_EXPR(self));

	self->left_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->left_expr));

	self->property_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->property_token));

	self->equal_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->equal_token));

	self->right_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->right_expr));

	return _self;
}


static void *set_expr_destructor(void *_self) {
	struct SetExpr *self = _self;
	assert(IS_SET_EXPR(self));

	delete((void *) self->left_expr);
	delete((void *) self->right_expr);

	return _self;
}


static void *set_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct SetExpr *self = _self;
	assert(IS_SET_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_set_expr);

	return ast_visitor->visit_set_expr(_ast_visitor, _self);
}


static bool is_set_expr(const void *_object) {
	const struct SetExpr *object = _object;
	assert(object);

	return (object->type == SetExpr);
}


static const void *set_expr_get_lmost_token(const void *_self) {
	const struct SetExpr *self = _self;
	assert(IS_SET_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *set_expr_get_rmost_token(const void *_self) {
	const struct SetExpr *self = _self;
	assert(IS_SET_EXPR(self));

	return get_rightmost_token(self->right_expr);
}


