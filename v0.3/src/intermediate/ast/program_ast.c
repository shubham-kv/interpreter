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
#include "headers_intermediate_ast.h"

#include "ast_visitor.h"


static void *program_ast_constructor(void *self, va_list *ap);
static void *program_ast_destructor(void *self);

static void *program_ast_accept_visitor(const void *self, const void *ast_visitor);
bool is_program_ast(const void *object);

static const struct AstNodeType _type_program_ast = {
	.size 					= sizeof(struct ProgramAst),
	.constructor 			= program_ast_constructor,
	.destructor 			= program_ast_destructor,
	.accept_visitor 		= program_ast_accept_visitor,
	.is_ast_node 			= is_program_ast,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};

const void *ProgramAst = &_type_program_ast;


// ... = new(ProgramAst, "foo.svl", symbol_table, imports_count, imports, decls_count, declarations);

static void *program_ast_constructor(void *_self, va_list *ap) {
	struct ProgramAst *self = _self;
	assert(IS_PROGRAM_AST(self));

	self->program_name = va_arg(*ap, char *);

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->imports_count = va_arg(*ap, uint64_t);
	self->imports 		= va_arg(*ap, void **);

	for(uint64_t i = 0; i < self->imports_count; i++) {
		assert(IS_IMPORT_STMT(self->imports[i]));
	}

	self->decls_count 	= va_arg(*ap, uint64_t);
	self->declarations 	= va_arg(*ap, void **);

	for(uint64_t i = 0; i < self->decls_count; i++) {
		assert(is_ast_node(self->declarations[i]));
	}

	return _self;
}


static void *program_ast_destructor(void *_self) {
	struct ProgramAst *self = _self;
	assert(IS_PROGRAM_AST(self));

	for(uint64_t i = 0; i < self->imports_count; i++) {
		delete(self->imports[i]);
	}
	free(self->imports);

	for(uint64_t i = 0; i < self->decls_count; i++) {
		delete(self->declarations[i]);
	}
	free(self->declarations);

	delete(self->symbol_table);
	free(self->program_name);

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

