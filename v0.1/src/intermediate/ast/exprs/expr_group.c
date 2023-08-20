#include "expr_group.h"

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


static void *groupexpr_constructor(void *self, va_list *ap);
static void *groupexpr_destructor(void *self);

static void *groupexpr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_group_expr(const void *_object);

static const void *group_expr_get_lmost_token(const void *self);
static const void *group_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_group_expr = {
	sizeof(struct GroupExpr),
	groupexpr_constructor,
	groupexpr_destructor,
	groupexpr_accept_visitor,
	is_group_expr,
	group_expr_get_lmost_token,
	group_expr_get_rmost_token,
	NULL
};

const void *GroupExpr = &_type_group_expr;


// ... = new(GroupExpr, lparen, child_expr, rparen);

static void *groupexpr_constructor(void *_self, va_list *ap) {
	struct GroupExpr *self = _self;
	assert(IS_GROUP_EXPR(self));

	self->lparen = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->lparen));

	self->child_expr = va_arg(*ap, const void *);
	assert(is_ast_node(self->child_expr));

	self->rparen = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->rparen));

	return _self;
}


static void *groupexpr_destructor(void *_self) {
	struct GroupExpr *self = _self;
	assert(IS_GROUP_EXPR(self));

	delete((void *) self->child_expr);

	return _self;
}


static void *groupexpr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct GroupExpr *self = _self;
	assert(IS_GROUP_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_group_expr);

	return ast_visitor->visit_group_expr(_ast_visitor, _self);
}


static const bool is_group_expr(const void *_object) {
	const struct GroupExpr *object = _object;
	assert(object);
	
	return (object->type == GroupExpr);
}


static const void *group_expr_get_lmost_token(const void *_self) {
	const struct GroupExpr *self = _self;
	assert(IS_GROUP_EXPR(self));

	return self->lparen;
}


static const void *group_expr_get_rmost_token(const void *_self) {
	const struct GroupExpr *self = _self;
	assert(IS_GROUP_EXPR(self));

	return self->rparen;	
}

