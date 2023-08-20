#include "decl_func.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol.h"
#include "symbol_table.h"
#include "ast_object.h"

#include "ast_visitor.h"



static void *fun_decl_constructor(void *self, va_list *ap);
static void *fun_decl_destructor(void *self);

static void *fun_decl_accept_visitor(const void *self, const void *visitor);
bool is_fun_decl(const void *self);

/*
static const struct AstNodeType _type_fun_decl = {
	sizeof(struct FunDecl),
	fun_decl_constructor,
	fun_decl_destructor,
	fun_decl_accept_visitor,
	is_fun_decl
};
// */

static const struct AstNodeType _type_fun_decl = {
	.size 					= sizeof(struct FunDecl),
	.constructor 			= fun_decl_constructor,
	.destructor 			= fun_decl_destructor,
	.accept_visitor 		= fun_decl_accept_visitor,
	.is_ast_node 			= is_fun_decl,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL,
};

const void *FunDecl = &_type_fun_decl;


// ... = new(FunDecl, fun_symbol, fun_symbol_table, param_count, param_symbols, statements);

static void *fun_decl_constructor(void *_self, va_list *ap) {
	struct FunDecl *self = _self;
	assert(IS_FUN_DECL(self));

	self->symbol = va_arg(*ap, void *);
	assert(IS_SYMBOL(self->symbol));

	self->symbol_table = va_arg(*ap, void *);
	assert(IS_SYMBOL_TABLE(self->symbol_table));

	self->param_count = va_arg(*ap, uint32_t);
	self->param_symbols = va_arg(*ap, void **);

	for(uint32_t i = 0; i < self->param_count; i++) {
		assert(IS_SYMBOL(self->param_symbols[i]));
	}
	
	self->declarations = va_arg(*ap, void **);

	if(NULL != self->declarations) {
		for(uint32_t i = 0; NULL != self->declarations[i]; i++) {
			assert(is_ast_node(self->declarations[i]));
		}
	}

	return _self;
}


static void *fun_decl_destructor(void *_self) {
	struct FunDecl *self = _self;
	assert(IS_FUN_DECL(self));

	// symbol_delete_attr(self->symbol, SYM_ATTR_KEY_AST);

	free(self->param_symbols);

	for(uint32_t i = 0; NULL != self->declarations[i]; i++)
		delete(self->declarations[i]);

	free(self->declarations);

	delete(self->symbol_table);

	return _self;
}


static void *fun_decl_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct FunDecl *self = _self;
	assert(IS_FUN_DECL(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_fun_decl);

	return ast_visitor->visit_fun_decl(_ast_visitor, _self);
}


bool is_fun_decl(const void *_object) {
	const struct FunDecl *object = _object;
	assert(object);

	return (object->type == FunDecl);
}



void fun_decl_set_declarations(void *_self, void **declarations) {
	struct FunDecl *self = _self;
	assert(IS_FUN_DECL(self));

	self->declarations = declarations;

	for(uint32_t i = 0; NULL != self->declarations[i]; i++) {
		assert(is_ast_node(self->declarations[i]));
	}
}



