#include "stmt_repeat.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol_table.h"
#include "ast_object.h"

#include "ast_visitor.h"


static void *repeat_stmt_constructor(void *self, va_list *ap);
static void *repeat_stmt_destructor(void *self);

static void *repeat_stmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_repeat_stmt(const void *object);

/*
static const struct AstNodeType _type_repeat_stmt = {
	sizeof(struct RepeatStmt),
	repeat_stmt_constructor,
	repeat_stmt_destructor,
	repeat_stmt_accept_visitor,
	is_repeat_stmt
};
// */

static const struct AstNodeType _type_repeat_stmt = {
	.size 					= sizeof(struct RepeatStmt),
	.constructor 			= repeat_stmt_constructor,
	.destructor 			= repeat_stmt_destructor,
	.accept_visitor 		= repeat_stmt_accept_visitor,
	.is_ast_node 			= is_repeat_stmt,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *RepeatStmt = &_type_repeat_stmt;


// ... = new(RepeatStmt, symbol_table, id_token, expr, child_stmts);

static void *repeat_stmt_constructor(void *_self, va_list *ap) {
	struct RepeatStmt *self = _self;
	assert(IS_REPEAT_STMT(self));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->id_token = va_arg(*ap, const struct Token *);

	if(NULL != self->id_token) {
		assert(IS_TOKEN(self->id_token));
	}

	self->expr = va_arg(*ap, void *);
	assert(is_ast_node(self->expr));

	self->child_stmts = va_arg(*ap, void **);

	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++) {
		assert(is_ast_node(self->child_stmts[i]));
	}

	return _self;
}


static void *repeat_stmt_destructor(void *_self) {
	struct RepeatStmt *self = _self;
	assert(IS_REPEAT_STMT(self));

	delete(self->expr);

	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++)
		delete(self->child_stmts[i]);
	
	free(self->child_stmts);

	delete((void *) self->symbol_table);

	return _self;
}


static void *repeat_stmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct RepeatStmt *self = _self;
	assert(IS_REPEAT_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_repeat_stmt);

	return ast_visitor->visit_repeat_stmt(_ast_visitor, _self);
}


static bool is_repeat_stmt(const void *_object) {
	const struct RepeatStmt *object = _object;
	assert(object);

	return (object->type == RepeatStmt);
}

