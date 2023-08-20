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


static void *exprstmt_constructor(void *self, va_list *ap);
static void *exprstmt_destructor(void *self);

static void *exprstmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_expr_stmt(const void *object);


static const struct AstNodeType _type_expr_stmt = {
	sizeof(struct ExprStmt),
	exprstmt_constructor,
	exprstmt_destructor,
	exprstmt_accept_visitor,
	is_expr_stmt
};

const void *ExprStmt = &_type_expr_stmt;


// ... = new(ExprStmt, expression);

static void *exprstmt_constructor(void *_self, va_list *ap) {
	struct ExprStmt *self = _self;
	assert(IS_EXPR_STMT(self));

	self->expr = va_arg(*ap, const void *);
	assert(is_ast_node(self->expr));

	return _self;
}


static void *exprstmt_destructor(void *_self) {
	struct ExprStmt *self = _self;
	assert(IS_EXPR_STMT(self));

	delete((void *) self->expr);

	return _self;
}


static void *exprstmt_accept_visitor(const void *_self, const void *_ast_visitor) {
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

