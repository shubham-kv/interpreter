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


static void *vardecl_constructor(void *self, va_list *ap);
static void *vardecl_destructor(void *self);

static void *vardecl_accept_visitor(const void *self, const void *ast_visitor);
static bool is_var_decl(const void *object);


static const struct AstNodeType _type_var_decl = {
	sizeof(struct VarDecl),
	vardecl_constructor,
	vardecl_destructor,
	vardecl_accept_visitor,
	is_var_decl
};

const void *VarDecl = &_type_var_decl;


// ... = new(VarDecl, symbols);

static void *vardecl_constructor(void *_self, va_list *ap) {
	struct VarDecl *self = _self;
	assert(IS_VAR_DECL(self));

	void **symbols = va_arg(*ap, void **);

	for(uint32_t i = 0; NULL != symbols[i]; i++) {
		assert(IS_SYMBOL(symbols[i]));
	}

	self->symbols = symbols;

	return _self;
}


static void *vardecl_destructor(void *_self) {
	struct VarDecl *self = _self;
	assert(IS_VAR_DECL(self));

	free(self->symbols);

	return _self;
}


static void *vardecl_accept_visitor(const void *_self, const void *_ast_visitor) {
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

