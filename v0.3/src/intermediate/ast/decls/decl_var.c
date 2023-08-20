#include "decl_var.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol.h"
#include "ast_object.h"

#include "ast_visitor.h"


static void *var_decl_constructor(void *self, va_list *ap);
static void *var_decl_destructor(void *self);

static void *var_decl_accept_visitor(const void *self, const void *ast_visitor);
static bool is_var_decl(const void *object);


static const struct AstNodeType _type_var_decl = {
	.size 					= sizeof(struct VarDecl),
	.constructor 			= var_decl_constructor,
	.destructor 			= var_decl_destructor,
	.accept_visitor 		= var_decl_accept_visitor,
	.is_ast_node 			= is_var_decl,
	.get_leftmost_token 	= NULL,
	.get_rightmost_token 	= NULL,
	.get_tokens 			= NULL
};


const void *VarDecl = &_type_var_decl;


// ... = new(VarDecl, symbols);

static void *var_decl_constructor(void *_self, va_list *ap) {
	struct VarDecl *self = _self;
	assert(IS_VAR_DECL(self));

	void **symbols = va_arg(*ap, void **);

	for(uint32_t i = 0; NULL != symbols[i]; i++) {
		assert(IS_SYMBOL(symbols[i]));
	}

	self->symbols = symbols;

	return _self;
}


static void *var_decl_destructor(void *_self) {
	struct VarDecl *self = _self;
	assert(IS_VAR_DECL(self));

	free(self->symbols);

	return _self;
}


static void *var_decl_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct VarDecl *self = _self;
	assert(IS_VAR_DECL(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_var_decl);

	return ast_visitor->visit_var_decl(_ast_visitor, _self);
}


static bool is_var_decl(const void *_object) {
	const struct VarDecl *object = _object;
	assert(object);

	return (object->type == VarDecl);
}

