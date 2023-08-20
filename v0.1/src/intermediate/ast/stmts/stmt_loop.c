#include "stmt_loop.h"

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


static void *loopstmt_constructor(void *self, va_list *ap);
static void *loopstmt_destructor(void *self);

static void *loopstmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_loop_stmt(const void *object);


static const struct AstNodeType _type_loop_stmt = {
	sizeof(struct LoopStmt),
	loopstmt_constructor,
	loopstmt_destructor,
	loopstmt_accept_visitor,
	is_loop_stmt
};

const void *LoopStmt = &_type_loop_stmt;


// ... = new(LoopStmt, symbol_table, loop_type, id_token, start_expr, end_expr, step_expr, child_stmts);

static void *loopstmt_constructor(void *_self, va_list *ap) {
	struct LoopStmt *self = _self;
	assert(IS_LOOP_STMT(self));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->loop_type = va_arg(*ap, enum LoopStmtType);

	self->id_token = va_arg(*ap, const struct Token *);
	if(NULL != self->id_token) {
		assert(IS_TOKEN(self->id_token));
	}

	self->start_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->start_expr));

	self->arrow_token = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->arrow_token));

	self->end_expr = va_arg(*ap, void *);
	if(NULL != self->end_expr) {
		assert(is_ast_node(self->end_expr));
	}

	self->step_expr = va_arg(*ap, void *);
	if(NULL != self->step_expr) {
		assert(is_ast_node(self->step_expr));
	}

	self->child_stmts = va_arg(*ap, void **);
	
	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++) {
		assert(is_ast_node(self->child_stmts[i]));
	}

	return _self;
}


static void *loopstmt_destructor(void *_self) {
	struct LoopStmt *self = _self;
	assert(IS_LOOP_STMT(self));

	delete(self->start_expr);

	if(NULL != self->end_expr)
		delete(self->end_expr);

	if(NULL != self->step_expr)
		delete(self->step_expr);
	
	for(uint32_t i = 0; NULL != self->child_stmts[i]; i++)
		delete(self->child_stmts[i]);
	
	free(self->child_stmts);

	delete((void *) self->symbol_table);

	return _self;
}


static void *loopstmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct LoopStmt *self = _self;
	assert(IS_LOOP_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_loop_stmt);

	return ast_visitor->visit_loop_stmt(_ast_visitor, _self);
}


static bool is_loop_stmt(const void *_object) {
	const struct LoopStmt *object = _object;
	assert(object);

	return (object->type == LoopStmt);
}

