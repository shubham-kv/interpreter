#include "expr_call_super_constructor.h"

#include <assert.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "type.h"
#include "object.h"

#include "symbol.h"
#include "ast_object.h"

#include "ast_visitor.h"


static void *super_call_expr_constructor(void *self, va_list *ap);
static void *super_call_expr_destructor(void *self);

static void *super_call_expr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_super_call_expr(const void *object);

static const void *super_call_expr_get_lmost_token(const void *self);
static const void *super_call_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_super_call_expr = {
	.size 					= sizeof(struct SuperConstructorCallExpr),
	.constructor 			= super_call_expr_constructor,
	.destructor 			= super_call_expr_destructor,

	.accept_visitor 		= super_call_expr_accept_visitor,
	.is_ast_node 			= is_super_call_expr,

	.get_leftmost_token 	= super_call_expr_get_lmost_token,
	.get_rightmost_token 	= super_call_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *SuperConstructorCallExpr = &_type_super_call_expr;


// ... = new(SuperConstructorCallExpr, cur_instance_ptr_symbol,  super, lparen, arg_count, args, rparen)

static void *super_call_expr_constructor(void *_self, va_list *ap) {
	struct SuperConstructorCallExpr *self = _self;
	assert(IS_SUPER_CONSTRUCTOR_CALL_EXPR(self));

	self->cur_instance_ptr_symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->cur_instance_ptr_symbol));

	self->super_class_symbol = va_arg(*ap, const void *);
	assert(IS_SYMBOL(self->super_class_symbol));

	self->super = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->super));

	self->lparen = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->lparen));

	self->arg_count = (uint32_t) va_arg(*ap, uint32_t);
	self->args = va_arg(*ap, void **);

	for(uint32_t i = 0; i < self->arg_count; i++) {
		assert(is_ast_node(self->args[i]));
	}

	self->rparen = va_arg(*ap, const struct Token *);
	assert(IS_TOKEN(self->rparen));

	return _self;
}


static void *super_call_expr_destructor(void *_self) {
	struct SuperConstructorCallExpr *self = _self;
	assert(IS_SUPER_CONSTRUCTOR_CALL_EXPR(self));

	for(int i = 0; i < self->arg_count; i++) {
		delete(self->args[i]);
	}

	free(self->args);

	return _self;
}


static void *super_call_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct SuperConstructorCallExpr *self = _self;
	assert(IS_SUPER_CONSTRUCTOR_CALL_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_super_constructor_call_expr);

	return ast_visitor->visit_super_constructor_call_expr(_ast_visitor, _self);
}


static bool is_super_call_expr(const void *_object) {
	const struct SuperConstructorCallExpr *object = _object;
	assert(object);
	return (object->type == SuperConstructorCallExpr);
}


static const void *super_call_expr_get_lmost_token(const void *_self) {
	const struct SuperConstructorCallExpr *self = _self;
	assert(IS_SUPER_CONSTRUCTOR_CALL_EXPR(self));

	return self->super;
}


static const void *super_call_expr_get_rmost_token(const void *_self) {
	const struct SuperConstructorCallExpr *self = _self;
	assert(IS_SUPER_CONSTRUCTOR_CALL_EXPR(self));

	return self->rparen;
}


