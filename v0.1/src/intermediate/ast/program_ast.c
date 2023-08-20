#include "program_ast.h"

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
#include "ast_node_headers.h"

#include "ast_visitor.h"


static void *program_ast_constructor(void *self, va_list *ap);
static void *program_ast_destructor(void *self);

static void *program_ast_accept_visitor(const void *self, const void *ast_visitor);
bool is_program_ast(const void *object);


static const struct AstNodeType _type_program_ast = {
	sizeof(struct ProgramAst),
	program_ast_constructor,
	program_ast_destructor,
	program_ast_accept_visitor,
	is_program_ast
};

const void *ProgramAst = &_type_program_ast;


// ... = new(ProgramAst, symbol_table, declarations);

static void *program_ast_constructor(void *_self, va_list *ap) {
	struct ProgramAst *self = _self;
	assert(IS_PROGRAM_AST(self));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->declarations = va_arg(*ap, void **);

	for(uint32_t i = 0; NULL != self->declarations[i]; i++) {
		assert(is_ast_node(self->declarations[i]));
	}

	return _self;
}


static void *program_ast_destructor(void *_self) {
	struct ProgramAst *self = _self;
	assert(IS_PROGRAM_AST(self));

	for(uint32_t i = 0; NULL != self->declarations[i]; i++)
		delete(self->declarations[i]);
	
	free(self->declarations);
	delete(self->symbol_table);

	return _self;
}


static void *program_ast_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct ProgramAst *self = _self;
	assert(IS_PROGRAM_AST(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_program_ast);

	return ast_visitor->visit_program_ast(_ast_visitor, _self);
}


bool is_program_ast(const void *_object) {
	const struct ProgramAst *object = _object;
	assert(object);

	return (object->type == ProgramAst);
}

