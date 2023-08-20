#include "expr_unary.h"

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


static void *unary_expr_constructor(void *self, va_list *ap);
static void *unary_expr_destructor(void *self);

static void *unary_expr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_unary_expr(const void *object);

static const void *unary_expr_get_lmost_token(const void *self);
static const void *unary_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_unaryExpr = {
	sizeof(struct UnaryExpr),
	unary_expr_constructor,
	unary_expr_destructor,
	unary_expr_accept_visitor,
	is_unary_expr,
	unary_expr_get_lmost_token,
	unary_expr_get_rmost_token,
	NULL
};

const void *UnaryExpr = &_type_unaryExpr;


// ... = new(UnaryExpr, op_token, right_expr);

static void *unary_expr_constructor(void *_self, va_list *ap) {
	struct UnaryExpr *self = _self;
	assert(IS_UNARY_EXPR(self));

	self->op_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->op_token));

	self->right = va_arg(*ap, const void *);
	assert(is_ast_node(self->right));

	return _self;
}


static void *unary_expr_destructor(void *_self) {
	struct UnaryExpr *self = _self;
	assert(IS_UNARY_EXPR(self));

	delete((void *) self->right);

	return _self;
}


static void *unary_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct UnaryExpr *self = _self;
	assert(IS_UNARY_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_unary_expr);

	return ast_visitor->visit_unary_expr(_ast_visitor, _self);
}


static bool is_unary_expr(const void *_object) {
	const struct UnaryExpr *object = _object;
	assert(object);

	return (object->type == UnaryExpr);
}



static const void *unary_expr_get_lmost_token(const void *_self) {
	const struct UnaryExpr *self = _self;
	assert(IS_UNARY_EXPR(self));

	return self->op_token;
}

static const void *unary_expr_get_rmost_token(const void *_self) {
	const struct UnaryExpr *self = _self;
	assert(IS_UNARY_EXPR(self));

	return get_rightmost_token(self->right);
}

