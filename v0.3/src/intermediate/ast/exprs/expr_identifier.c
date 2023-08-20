#include "expr_identifier.h"

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


static void *id_expr_constructor(void *self, va_list *ap);
static void *id_expr_destructor(void *self);

static void *id_expr_accept_visitor(const void *self, const void *ast_visitor);
bool is_id_expr(const void *object);

static const void *id_expr_get_lmost_token(const void *self);
static const void *id_expr_get_rmost_token(const void *self);


/*
static const struct AstNodeType _type_id_expr = {
	sizeof(struct IdentifierExpr),
	id_expr_constructor,
	id_expr_destructor,
	id_expr_accept_visitor,
	is_id_expr,
	id_expr_get_lmost_token,
	id_expr_get_rmost_token,
	NULL
};
// */

static const struct AstNodeType _type_id_expr = {
	.size 					= sizeof(struct IdentifierExpr),
	.constructor 			= id_expr_constructor,
	.destructor 			= id_expr_destructor,

	.accept_visitor 		= id_expr_accept_visitor,
	.is_ast_node 			= is_id_expr,

	.get_leftmost_token 	= id_expr_get_lmost_token,
	.get_rightmost_token 	= id_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *IdentifierExpr = &_type_id_expr;


// ... = new(IdentifierExpr, id-token, symbol);

static void *id_expr_constructor(void *_self, va_list *ap) {
	struct IdentifierExpr *self = _self;
	assert(IS_IDENTIFIER_EXPR(self));

	self->id_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->id_token));

	self->symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->symbol));

	return _self;
}


static void *id_expr_destructor(void *_self) {
	return _self;
}


static void *id_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct IdentifierExpr *self = _self;
	assert(IS_IDENTIFIER_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_identifier_expr);

	return ast_visitor->visit_identifier_expr(_ast_visitor, _self);
}


bool is_id_expr(const void *_object) {
	const struct IdentifierExpr *object = _object;
	assert(object);
	return (object->type == IdentifierExpr);
}


static const void *id_expr_get_lmost_token(const void *_self) {
	const struct IdentifierExpr *self = _self;
	assert(IS_IDENTIFIER_EXPR(self));

	return self->id_token;
}


static const void *id_expr_get_rmost_token(const void *_self) {
	const struct IdentifierExpr *self = _self;
	assert(IS_IDENTIFIER_EXPR(self));

	return self->id_token;
}
