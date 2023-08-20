#include "expr_bitwise.h"

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



static void *bitwise_expr_constructor(void *self, va_list *ap);
static void *bitwise_expr_destructor(void *self);

static void *bitwise_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_bitwise_expr(const void *object);

static const void *bitwise_expr_get_lmost_token(const void *self);
static const void *bitwise_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_bitwise_expr = {
	.size 					= sizeof(struct BitwiseExpr),
	.constructor 			= bitwise_expr_constructor,
	.destructor 			= bitwise_expr_destructor,

	.accept_visitor 		= bitwise_expr_accept_visitor,
	.is_ast_node 			= is_bitwise_expr,

	.get_leftmost_token 	= bitwise_expr_get_lmost_token,
	.get_rightmost_token 	= bitwise_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *BitwiseExpr = &_type_bitwise_expr;


// ... = new(BitwiseExpr, op_token, left, right);

static void *bitwise_expr_constructor(void *_self, va_list *ap) {
	struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	self->op_token = va_arg(*ap, const struct Token *);

	self->left = va_arg(*ap, const void *);
	assert(is_ast_node(self->left));

	self->right = va_arg(*ap, const void *);
	assert(is_ast_node(self->right));

	return _self;
}

static void *bitwise_expr_destructor(void *_self) {
	struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	delete((void *) self->left);
	delete((void *) self->right);

	return _self;
}

static void *bitwise_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_bitwise_expr);

	return ast_visitor->visit_bitwise_expr(_ast_visitor, _self);
}


static bool is_bitwise_expr(const void *_object) {
	const struct BitwiseExpr *object = _object;
	assert(object);

	return (object->type == BitwiseExpr);
}


static const void *bitwise_expr_get_lmost_token(const void *_self) {
	const struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	return get_leftmost_token(self->left);
}


static const void *bitwise_expr_get_rmost_token(const void *_self) {
	const struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	return get_rightmost_token(self->right);
}



/*
void bitwise_expr_type_printer(const void *_self) {
	const struct BitwiseExpr *self = _self;
	assert(IS_BITWISE_EXPR(self));

	printf("BitwiseExpr <%#x> {\n", self);
	printf("  left: %#x", self->left);
	printf("  right: %#x", self->right);
	printf("}");
}
*/

