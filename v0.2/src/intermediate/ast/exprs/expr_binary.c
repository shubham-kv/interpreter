#include "expr_binary.h"

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



static void *binary_expr_constructor(void *self, va_list *ap);
static void *binary_expr_destructor(void *self);

static void *binary_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_binary_expr(const void *object);

static const void *binary_expr_get_lmost_token(const void *self);
static const void *binary_expr_get_rmost_token(const void *self);

/*
static const struct AstNodeType _type_binary_expr = {
	sizeof(struct BinaryExpr),
	binary_expr_constructor,
	binary_expr_destructor,
	binary_expr_accept_visitor,
	is_binary_expr,
	binary_expr_get_lmost_token,
	binary_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_binary_expr = {
	.size 					= sizeof(struct BinaryExpr),
	.constructor 			= binary_expr_constructor,
	.destructor 			= binary_expr_destructor,

	.accept_visitor 		= binary_expr_accept_visitor,
	.is_ast_node 			= is_binary_expr,

	.get_leftmost_token 	= binary_expr_get_lmost_token,
	.get_rightmost_token 	= binary_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *BinaryExpr = &_type_binary_expr;


// ... = new(BinaryExpr, op_token, left, right);

static void *binary_expr_constructor(void *_self, va_list *ap) {
	struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	self->op_token = va_arg(*ap, const struct Token *);

	self->left = va_arg(*ap, const void *);
	assert(is_ast_node(self->left));

	self->right = va_arg(*ap, const void *);
	assert(is_ast_node(self->right));

	return _self;
}


static void *binary_expr_destructor(void *_self) {
	struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	delete((void *) self->left);
	delete((void *) self->right);

	return _self;
}


static void *binary_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_binary_expr);

	return ast_visitor->visit_binary_expr(_ast_visitor, _self);
}


static bool is_binary_expr(const void *_object) {
	const struct BinaryExpr *object = _object;
	assert(object);

	return (object->type == BinaryExpr);
}


static const void *binary_expr_get_lmost_token(const void *_self) {
	const struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	return get_leftmost_token(self->left);
}


static const void *binary_expr_get_rmost_token(const void *_self) {
	const struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	return get_rightmost_token(self->right);
}



/*
void binary_expr_type_printer(const void *_self) {
	const struct BinaryExpr *self = _self;
	assert(IS_BINARY_EXPR(self));

	printf("BinaryExpr <%#x> {\n", self);
	printf("  left: %#x", self->left);
	printf("  right: %#x", self->right);
	printf("}");
}
*/

