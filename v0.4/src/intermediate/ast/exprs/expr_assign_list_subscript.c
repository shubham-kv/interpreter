#include "expr_assign_list_subscript.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol.h"
#include "ast_object.h"

#include "ast_visitor.h"


static void *ls_assign_expr_constructor(void *self, va_list *ap);
static void *ls_assign_expr_destructor(void *self);

static void *ls_assign_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_ls_assign_expr(const void *object);

static const void *ls_assign_expr_get_lmost_token(const void *self);
static const void *ls_assign_expr_get_rmost_token(const void *self);

/*
static const struct AstNodeType _type_ls_assign_expr = {
	sizeof(struct ListSubscriptAssignExpr),
	ls_assign_expr_constructor,
	ls_assign_expr_destructor,
	ls_assign_expr_accept_visitor,
	is_ls_assign_expr,
	ls_assign_expr_get_lmost_token,
	ls_assign_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_ls_assign_expr = {
	.size 					= sizeof(struct ListSubscriptAssignExpr),
	.constructor 			= ls_assign_expr_constructor,
	.destructor 			= ls_assign_expr_destructor,

	.accept_visitor 		= ls_assign_expr_accept_visitor,
	.is_ast_node 			= is_ls_assign_expr,

	.get_leftmost_token 	= ls_assign_expr_get_lmost_token,
	.get_rightmost_token 	= ls_assign_expr_get_rmost_token,
	.get_tokens 			= NULL
};

const void *ListSubscriptAssignExpr = &_type_ls_assign_expr;


// ... = new(ListSubscriptAssignExpr, left_expr, index_expr, equal_token, right_token);

static void *ls_assign_expr_constructor(void *_self, va_list *ap) {
	struct ListSubscriptAssignExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(self));

	self->left_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->left_expr));

	self->index_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->index_expr));

	self->equal_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->equal_token));

	self->right_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->right_expr));

	return _self;
}


static void *ls_assign_expr_destructor(void *_self) {
	struct ListSubscriptAssignExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(self));

	delete((void *) self->left_expr);
	delete((void *) self->index_expr);

	delete((void *) self->right_expr);

	return _self;
}


static void *ls_assign_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ListSubscriptAssignExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_ls_assign_expr);

	return ast_visitor->visit_ls_assign_expr(_ast_visitor, _self);
}


static bool is_ls_assign_expr(const void *_object) {
	const struct ListSubscriptAssignExpr *object = _object;
	assert(object);

	return (object->type == ListSubscriptAssignExpr);
}


static const void *ls_assign_expr_get_lmost_token(const void *_self) {
	const struct ListSubscriptAssignExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *ls_assign_expr_get_rmost_token(const void *_self) {
	const struct ListSubscriptAssignExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_ASSIGN_EXPR(self));

	return get_rightmost_token(self->right_expr);
}


