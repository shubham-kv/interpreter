#include "expr_list_subscript.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>


#include "type.h"
#include "object.h"

#include "token.h"

#include "symbol.h"
#include "ast_object.h"

#include "ast_visitor.h"



static void *lse_constructor(void *self, va_list *ap);
static void *lse_destructor(void *self);

static void *lse_accept_visitor(const void *self, const void *ast_visitor);
bool is_lse(const void *object);

static const void *lse_get_lmost_token(const void *self);
static const void *lse_get_rmost_token(const void *self);


/*
static const struct AstNodeType _type_lse = {
	sizeof(struct ListSubscriptExpr),
	lse_constructor,
	lse_destructor,
	lse_accept_visitor,
	is_lse,
	lse_get_lmost_token,
	lse_get_rmost_token,
	NULL
};
// */


static const struct AstNodeType _type_lse = {
	.size 					= sizeof(struct ListSubscriptExpr),
	.constructor 			= lse_constructor,
	.destructor 			= lse_destructor,

	.accept_visitor 		= lse_accept_visitor,
	.is_ast_node 			= is_lse,

	.get_leftmost_token 	= lse_get_lmost_token,
	.get_rightmost_token 	= lse_get_rmost_token,
	.get_tokens 			= NULL
};



const void *ListSubscriptExpr = &_type_lse;


// ... = new(ListSubscriptExpr, left_expr, lsbrace, index_expr, rsbrace);

static void *lse_constructor(void *_self, va_list *ap) {
	struct ListSubscriptExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_EXPR(self));

	self->left_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->left_expr));

	self->lsbrace = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->lsbrace));

	self->index_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->index_expr));

	self->rsbrace = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->rsbrace));

	return _self;
}


static void *lse_destructor(void *_self) {
	struct ListSubscriptExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_EXPR(self));

	delete((void *) self->left_expr);
	delete((void *) self->index_expr);

	return _self;
}


static void *lse_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ListSubscriptExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_list_subscript_expr);

	return ast_visitor->visit_list_subscript_expr(_ast_visitor, _self);
}


bool is_lse(const void *_object) {
	const struct ListSubscriptExpr *object = _object;
	assert(object);

	return (object->type == ListSubscriptExpr);
}


static const void *lse_get_lmost_token(const void *_self) {
	const struct ListSubscriptExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *lse_get_rmost_token(const void *_self) {
	const struct ListSubscriptExpr *self = _self;
	assert(IS_LIST_SUBSCRIPT_EXPR(self));

	return self->rsbrace;
}
