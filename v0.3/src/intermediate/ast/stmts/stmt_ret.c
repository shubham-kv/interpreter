#include "stmt_ret.h"

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


static void *ret_stmt_constructor(void *self, va_list *ap);
static void *ret_stmt_destructor(void *self);
static void *ret_stmt_accept_visitor(const void *self, const void *ast_visitor);
bool is_ret_stmt(const void *object);



static const struct AstNodeType _type_ret_stmt = {
	.size 					= sizeof(struct RetStmt),
	.constructor 			= ret_stmt_constructor,
	.destructor 			= ret_stmt_destructor,
	.accept_visitor 		= ret_stmt_accept_visitor,
	.is_ast_node 			= is_ret_stmt,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *RetStmt = &_type_ret_stmt;


// ... = new(RetStmt, expr);

static void *ret_stmt_constructor(void *_self, va_list *ap) {
	struct RetStmt *self = _self;
	assert(IS_RET_STMT(self));

	self->expression = va_arg(*ap, void *);

	if(NULL != self->expression) {
		assert(is_ast_node(self->expression));
	}

	return _self;
}

static void *ret_stmt_destructor(void *_self) {
	struct RetStmt *self = _self;
	assert(IS_RET_STMT(self));

	if(NULL != self->expression)
		delete(self->expression);

	return _self;
}

static void *ret_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct RetStmt *self = _self;
	assert(IS_RET_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_ret_stmt);

	return ast_visitor->visit_ret_stmt(_ast_visitor, _self);
}

bool is_ret_stmt(const void *_object) {
	const struct RetStmt *object = _object;
	assert(object);

	return (object->type == RetStmt);
}

