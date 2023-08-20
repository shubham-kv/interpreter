#include "expr_list.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "type.h"
#include "object.h"

#include "ast_object.h"

#include "ast_visitor.h"



static void *list_expr_constructor(void *self, va_list *ap);
static void *list_expr_destructor(void *self);

static bool is_list_expr(const void *self);
static void *list_expr_accept_visitor(const void *self, const void *ast_visitor);

static const void *list_expr_get_lmost_token(const void *self);
static const void *list_expr_get_rmost_token(const void *self);



static const struct AstNodeType _type_list_expr = {
	sizeof(struct ListExpr),
	list_expr_constructor,
	list_expr_destructor,
	list_expr_accept_visitor,
	is_list_expr,
	list_expr_get_lmost_token,
	list_expr_get_rmost_token,
	NULL
};

const void *ListExpr = &_type_list_expr;


// ... = new(LisExpr, expr_count, exprs);

static void *list_expr_constructor(void *_self, va_list *ap) {
	struct ListExpr *self = _self;
	assert(IS_LIST_EXPR(self));

	self->lsbrace = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->lsbrace));

	self->expr_count = va_arg(*ap, int);
	self->exprs = va_arg(*ap, void **);

	for(uint32_t i = 0; i < self->expr_count; i++) {
		assert(is_ast_node(self->exprs[i]));
	}

	self->rsbrace = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->rsbrace));

	return _self;
}

static void *list_expr_destructor(void *_self) {
	struct ListExpr *self = _self;
	assert(IS_LIST_EXPR(self));

	for(uint32_t i = 0; i < self->expr_count; i++) {
		delete(self->exprs[i]);
	}

	free(self->exprs);

	return _self;
}


static void *list_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ListExpr *self = _self;
	assert(IS_LIST_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_list_expr);

	return ast_visitor->visit_list_expr(_ast_visitor, _self);
}

static bool is_list_expr(const void *_object) {
	const struct ListExpr *object = _object;
	assert(object);

	return (object->type == ListExpr);
}


static const void *list_expr_get_lmost_token(const void *_self) {
	const struct ListExpr *self = _self;
	assert(IS_LIST_EXPR(self));

	return self->lsbrace;
}

static const void *list_expr_get_rmost_token(const void *_self) {
	const struct ListExpr *self = _self;
	assert(IS_LIST_EXPR(self));

	return self->rsbrace;
}


