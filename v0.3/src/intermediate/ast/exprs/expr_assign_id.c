#include "expr_assign_id.h"

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



static void *id_assign_expr_constructor(void *self, va_list *ap);
static void *id_assign_expr_destructor(void *self);

static void *id_assign_expr_accept_visitor(const void *_self, const void *_visitor);
static bool is_id_assign_expr(const void *object);

static const void *id_assign_expr_get_lmost_token(const void *self);
static const void *id_assign_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_id_assign_expr = {
	.size 					= sizeof(struct IdentifierAssignExpr),
	.constructor 			= id_assign_expr_constructor,
	.destructor 			= id_assign_expr_destructor,

	.accept_visitor 		= id_assign_expr_accept_visitor,
	.is_ast_node 			= is_id_assign_expr,

	.get_leftmost_token 	= id_assign_expr_get_lmost_token,
	.get_rightmost_token 	= id_assign_expr_get_rmost_token,
	.get_tokens 			= NULL
};

const void *IdentifierAssignExpr = &_type_id_assign_expr;


// ... = new(IdentifierAssignExpr, id_token, id_symbol, equal_token, right_token);

static void *id_assign_expr_constructor(void *_self, va_list *ap) {
	struct IdentifierAssignExpr *self = _self;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(self));

	self->id_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->id_token));

	self->symbol = va_arg(*ap, const struct Symbol *);
	assert(IS_SYMBOL(self->symbol));

	self->equal_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->equal_token));

	self->right_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->right_expr));

	return _self;
}


static void *id_assign_expr_destructor(void *_self) {
	struct IdentifierAssignExpr *self = _self;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(self));

	delete((void *) self->right_expr);

	return _self;
}


static void *id_assign_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct IdentifierAssignExpr *self = _self;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_id_assign_expr);

	return ast_visitor->visit_id_assign_expr(_ast_visitor, _self);
}


static bool is_id_assign_expr(const void *_object) {
	const struct IdentifierAssignExpr *object = _object;
	assert(object);

	return (object->type == IdentifierAssignExpr);
}


static const void *id_assign_expr_get_lmost_token(const void *_self) {
	const struct IdentifierAssignExpr *self = _self;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(self));

	return self->id_token;
}


static const void *id_assign_expr_get_rmost_token(const void *_self) {
	const struct IdentifierAssignExpr *self = _self;
	assert(IS_IDENTIFIER_ASSIGN_EXPR(self));

	return get_rightmost_token(self->right_expr);
}
