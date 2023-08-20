#include "stmt_expr.h"

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


static void *expr_stmt_constructor(void *self, va_list *ap);
static void *expr_stmt_destructor(void *self);

static void *expr_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_expr_stmt(const void *object);

/*
static const struct AstNodeType _type_expr_stmt = {
	sizeof(struct ExprStmt),
	expr_stmt_constructor,
	expr_stmt_destructor,
	expr_stmt_accept_visitor,
	is_expr_stmt
};
// */

static const struct AstNodeType _type_expr_stmt = {
	.size 					= sizeof(struct ExprStmt),
	.constructor 			= expr_stmt_constructor,
	.destructor 			= expr_stmt_destructor,
	.accept_visitor 		= expr_stmt_accept_visitor,
	.is_ast_node 			= is_expr_stmt,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *ExprStmt = &_type_expr_stmt;


// ... = new(ExprStmt, expression);

static void *expr_stmt_constructor(void *_self, va_list *ap) {
	struct ExprStmt *self = _self;
	assert(IS_EXPR_STMT(self));

	self->expr = va_arg(*ap, const void *);
	assert(is_ast_node(self->expr));

	return _self;
}


static void *expr_stmt_destructor(void *_self) {
	struct ExprStmt *self = _self;
	assert(IS_EXPR_STMT(self));

	delete((void *) self->expr);

	return _self;
}


static void *expr_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ExprStmt *self =_self;
	assert(IS_EXPR_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_expr_stmt);

	return ast_visitor->visit_expr_stmt(_ast_visitor, _self);
}


static bool is_expr_stmt(const void *_object) {
	const struct ExprStmt *object = _object;
	assert(object);

	return (object->type == ExprStmt);
}

