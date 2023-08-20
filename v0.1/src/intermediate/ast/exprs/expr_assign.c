#include "expr_assign.h"

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


static void *assign_expr_constructor(void *self, va_list *ap);
static void *assign_expr_destructor(void *self);

static void *assign_expr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_assign_expr(const void *object);

static const void *assignexpr_get_lmost_token(const void *self);
static const void *assignexpr_get_rmost_token(const void *self);

static const struct AstNodeType _type_assign_expr = {
	sizeof(struct AssignExpr),
	assign_expr_constructor,
	assign_expr_destructor,
	assign_expr_accept_visitor,
	is_assign_expr,
	assignexpr_get_lmost_token,
	assignexpr_get_rmost_token,
	NULL
};

const void *AssignExpr = &_type_assign_expr;


// ... = new(AssignExpr, lvalue_type, id_token, equal_token, id_symbol, index_expr, right_expr);

static void *assign_expr_constructor(void *_self, va_list *ap) {
	struct AssignExpr *self = _self;
	assert(IS_ASSIGN_EXPR(self));

	self->lvalue_type = va_arg(*ap, enum LValueType);

	self->id_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->id_token));

	self->equal_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->equal_token));

	self->id_symbol = va_arg(*ap, const struct Symbol *);
	assert(IS_SYMBOL(self->id_symbol));

	self->index_expr = va_arg(*ap, void *);
	if(NULL != self->index_expr)
		assert(is_ast_node(self->index_expr));

	self->right_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->right_expr));

	return _self;
}


static void *assign_expr_destructor(void *_self) {
	struct AssignExpr *self = _self;
	assert(IS_ASSIGN_EXPR(self));

	if(NULL != self->index_expr)
		delete(self->index_expr);

	delete(self->right_expr);

	return _self;
}


static void *assign_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct AssignExpr *self = _self;
	assert(IS_ASSIGN_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_assign_expr);

	return ast_visitor->visit_assign_expr(_ast_visitor, _self);
}


static bool is_assign_expr(const void *_object) {
	const struct AssignExpr *object = _object;
	assert(object);

	return (object->type == AssignExpr);
}


static const void *assignexpr_get_lmost_token(const void *_self) {
	const struct AssignExpr *self = _self;
	assert(IS_ASSIGN_EXPR(self));

	return self->id_token;
}


static const void *assignexpr_get_rmost_token(const void *_self) {
	const struct AssignExpr *self = _self;
	assert(IS_ASSIGN_EXPR(self));

	return get_rightmost_token(self->right_expr);
}



