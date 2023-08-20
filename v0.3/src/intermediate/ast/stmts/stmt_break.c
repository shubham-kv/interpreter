#include "stmt_break.h"

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



static void *break_stmt_constructor(void *self, va_list *ap);
static void *break_stmt_destructor(void *self);

static void *break_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_break_stmt(const void *object);

static const struct AstNodeType _type_break_stmt = {
	.size 					= sizeof(struct BreakStmt),
	.constructor 			= break_stmt_constructor,
	.destructor 			= break_stmt_destructor,
	.accept_visitor 		= break_stmt_accept_visitor,
	.is_ast_node 			= is_break_stmt,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *BreakStmt = &_type_break_stmt;


// ... = new(BreakStmt, break_token, semicolon_token);

static void *break_stmt_constructor(void *_self, va_list *ap) {
	struct BreakStmt *self = _self;
	assert(IS_BREAK_STMT(self));

	self->break_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->break_token));

	self->semicolon_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->semicolon_token));

	return _self;
}


static void *break_stmt_destructor(void *_self) {
	return _self;
}


static void *break_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct BreakStmt *self = _self;
	assert(IS_BREAK_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_break_stmt);

	return ast_visitor->visit_break_stmt(_ast_visitor, _self);
}


static bool is_break_stmt(const void *_object) {
	const struct BreakStmt *object = _object;
	assert(object);

	return (object->type == BreakStmt);
}


