#include "expr_get_super.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "ast_object.h"
#include "symbol.h"

#include "ast_visitor.h"



static void *super_get_expr_constructor(void *self, va_list *ap);
static void *super_get_expr_destructor(void *self);

static void *super_get_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_super_get_expr(const void *object);

static const void *super_get_expr_get_lmost_token(const void *self);
static const void *super_get_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_super_get_expr = {
	.size 					= sizeof(struct SuperGetExpr),
	.constructor 			= super_get_expr_constructor,
	.destructor 			= super_get_expr_destructor,

	.accept_visitor 		= super_get_expr_accept_visitor,
	.is_ast_node 			= is_super_get_expr,

	.get_leftmost_token 	= super_get_expr_get_lmost_token,
	.get_rightmost_token 	= super_get_expr_get_rmost_token,
	.get_tokens 			= NULL
};

const void *SuperGetExpr = &_type_super_get_expr;


// void *bexpr = new(SuperGetExpr, left_expr, property_token);

static void *super_get_expr_constructor(void *_self, va_list *ap) {
	struct SuperGetExpr *self = _self;
	assert(IS_SUPER_GET_EXPR(self));

	self->cur_instance_ptr_symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->cur_instance_ptr_symbol));

	self->super_class_symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->super_class_symbol));

	self->super_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->super_token));

	self->property_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->property_token));

	return _self;
}


static void *super_get_expr_destructor(void *_self) {
	struct SuperGetExpr *self = _self;
	assert(IS_SUPER_GET_EXPR(self));
	return _self;
}


static void *super_get_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct SuperGetExpr *self = _self;
	assert(IS_SUPER_GET_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_super_get_expr);

	return ast_visitor->visit_super_get_expr(_ast_visitor, _self);
}


static bool is_super_get_expr(const void *_object) {
	const struct SuperGetExpr *object = _object;
	assert(object);

	return (object->type == SuperGetExpr);
}


static const void *super_get_expr_get_lmost_token(const void *_self) {
	const struct SuperGetExpr *self = _self;
	assert(IS_SUPER_GET_EXPR(self));

	return self->super_token;
}


static const void *super_get_expr_get_rmost_token(const void *_self) {
	const struct SuperGetExpr *self = _self;
	assert(IS_SUPER_GET_EXPR(self));

	return self->property_token;
}

