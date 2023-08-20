#include "stmt_continue.h"

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


static void *continue_stmt_constructor(void *self, va_list *ap);
static void *continue_stmt_destructor(void *self);

static void *continue_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_continue_stmt(const void *object);


static const struct AstNodeType _type_continue_stmt = {
	sizeof(struct ContinueStmt),
	continue_stmt_constructor,
	continue_stmt_destructor,
	continue_stmt_accept_visitor,
	is_continue_stmt
};

const void *ContinueStmt = &_type_continue_stmt;


// ... = new(ContinueStmt, continue_token, semicolon_token);

static void *continue_stmt_constructor(void *_self, va_list *ap) {
	struct ContinueStmt *self = _self;
	assert(IS_CONTINUE_STMT(self));

	self->continue_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->continue_token));

	self->semicolon_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->semicolon_token));

	return _self;
}


static void *continue_stmt_destructor(void *_self) {
	return _self;
}


static void *continue_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ContinueStmt *self = _self;
	assert(IS_CONTINUE_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_continue_stmt);

	return ast_visitor->visit_continue_stmt(_ast_visitor, _self);
}


static bool is_continue_stmt(const void *_object) {
	const struct ContinueStmt *object = _object;
	assert(object);

	return (object->type == ContinueStmt);
}



