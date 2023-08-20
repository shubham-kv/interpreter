#include "stmt_when.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "ast_object.h"

#include "ast_visitor.h"


static void *when_case_constructor(void *self, va_list *ap);
static void *when_case_destructor(void *self);


static const struct Type _type_when_case = {
	sizeof(struct WhenCase),
	when_case_constructor,
	when_case_destructor
};

const void *WhenCase = &_type_when_case;


// ... = new(WhenCase, exprs, stmt);

static void *when_case_constructor(void *_self, va_list *ap) {
	struct WhenCase *self = _self;
	assert(IS_WHEN_CASE(self));

	self->exprs = va_arg(*ap, void **);

	for(uint32_t i = 0; NULL != self->exprs[i]; i++)
		assert(is_ast_node(self->exprs[i]));

	self->stmt = va_arg(*ap, void *);
	assert(is_ast_node(self->stmt));

	return _self;
}


static void *when_case_destructor(void *_self) {
	struct WhenCase *self = _self;
	assert(IS_WHEN_CASE(self));

	for(uint32_t i = 0; NULL != self->exprs[i]; i++)
		delete(self->exprs[i]);

	free(self->exprs);

	delete(self->stmt);

	return _self;
}






static void *whenstmt_constructor(void *self, va_list *ap);
static void *whenstmt_destructor(void *self);

static void *whenstmt_accept_visitor(const void *self, const void *ast_visitor);
static bool is_when_stmt(const void *object);

static const struct AstNodeType _type_when_stmt = {
	sizeof(struct WhenStmt),
	whenstmt_constructor,
	whenstmt_destructor,
	whenstmt_accept_visitor,
	is_when_stmt
};

const void *WhenStmt = &_type_when_stmt;


// ... = new(WhenStmt, expr, cases, else_stmt)

static void *whenstmt_constructor(void *_self, va_list *ap) {
	struct WhenStmt *self = _self;
	assert(IS_WHEN_STMT(self));

	self->expr = va_arg(*ap, void *);
	assert(is_ast_node(self->expr));

	self->cases = va_arg(*ap, struct WhenCase **);
	assert(self->cases);

	for(uint32_t i = 0; self->cases[i] != NULL; i++) {
		assert(IS_WHEN_CASE(self->cases[i]));
	}

	self->else_stmt = va_arg(*ap, void *);

	if(NULL != self->else_stmt)
		assert(is_ast_node(self->else_stmt));

	return _self;
}


static void *whenstmt_destructor(void *_self) {
	struct WhenStmt *self = _self;
	assert(IS_WHEN_STMT(self));

	delete(self->expr);

	for(uint32_t i = 0; self->cases[i] != NULL; i++)
		delete(self->cases[i]);

	free(self->cases);

	if(NULL != self->else_stmt)
		delete(self->else_stmt);
	
	return _self;
}


static void *whenstmt_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct WhenStmt *self = _self;
	assert(IS_WHEN_STMT(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_when_stmt);

	return ast_visitor->visit_when_stmt(_ast_visitor, _self);
}


static bool is_when_stmt(const void *_object) {
	const struct WhenStmt *object = _object;
	assert(object);

	return (object->type == WhenStmt);
}

