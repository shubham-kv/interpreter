#include "stmt_if.h"

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


static void *if_stmt_constructor(void *self, va_list *ap);
static void *if_stmt_destructor(void *self);

static void *if_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_if_stmt(const void *object);

/*
static const struct AstNodeType _type_if_stmt = {
	sizeof(struct IfStmt),
	if_stmt_constructor,
	if_stmt_destructor,
	if_stmt_accept_visitor,
	is_if_stmt
};
// */

static const struct AstNodeType _type_if_stmt = {
	.size 					= sizeof(struct IfStmt),
	.constructor 			= if_stmt_constructor,
	.destructor 			= if_stmt_destructor,
	.accept_visitor 		= if_stmt_accept_visitor,
	.is_ast_node 			= is_if_stmt,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *IfStmt = &_type_if_stmt;


// ... = new(IfStmt, expr, if_block, else_block);

static void *if_stmt_constructor(void *_self, va_list *ap) {
	struct IfStmt *self = _self;
	assert(IS_IF_STMT(self));

	self->expr = va_arg(*ap, void *);
	assert(is_ast_node(self->expr));

	self->if_block = va_arg(*ap, void *);
	assert(is_ast_node(self->if_block));

	self->else_block = va_arg(*ap, void *);

	if(NULL != self->else_block)
		assert(is_ast_node(self->else_block));

	return _self;
}


static void *if_stmt_destructor(void *_self) {
	struct IfStmt *self = _self;
	assert(IS_IF_STMT(self));

	delete((void *) self->expr);
	delete((void *) self->if_block);

	if(NULL != self->else_block)
		delete((void *) self->else_block);

	return _self;
}


static void *if_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct IfStmt *self = _self;
	assert(IS_IF_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_if_stmt);

	return ast_visitor->visit_if_stmt(_ast_visitor, _self);
}


static bool is_if_stmt(const void *_object) {
	const struct IfStmt *object = _object;
	assert(object);

	return (object->type == IfStmt);
}

