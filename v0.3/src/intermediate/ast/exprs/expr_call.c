#include "expr_call.h"

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


static void *call_expr_constructor(void *self, va_list *ap);
static void *call_expr_destructor(void *self);

static void *call_expr_accept_visitor(const void *self, const void *ast_visitor);
static bool is_call_expr(const void *object);

static const void *call_expr_get_lmost_token(const void *self);
static const void *call_expr_get_rmost_token(const void *self);


static const struct AstNodeType _type_call_expr = {
	.size 					= sizeof(struct CallExpr),
	.constructor 			= call_expr_constructor,
	.destructor 			= call_expr_destructor,

	.accept_visitor 		= call_expr_accept_visitor,
	.is_ast_node 			= is_call_expr,

	.get_leftmost_token 	= call_expr_get_lmost_token,
	.get_rightmost_token 	= call_expr_get_rmost_token,
	.get_tokens 			= NULL
};


const void *CallExpr = &_type_call_expr;


// ... = new(CallExpr, left_expr, lparen, arg_count, args, rparen)

static void *call_expr_constructor(void *_self, va_list *ap) {
	struct CallExpr *self = _self;
	assert(IS_CALL_EXPR(self));

	self->left_expr = va_arg(*ap, void *);
	assert(is_ast_node(self->left_expr));

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


static void *call_expr_destructor(void *_self) {
	struct CallExpr *self = _self;
	assert(IS_CALL_EXPR(self));

	delete(self->left_expr);

	for(uint32_t i = 0; i < self->arg_count; i++) {
		delete(self->args[i]);
	}

	free(self->args);

	return _self;
}


static void *call_expr_accept_visitor(const void *_self, const void *_ast_visitor) {
	const struct CallExpr *self = _self;
	assert(IS_CALL_EXPR(self));

	const struct AstVisitor *ast_visitor = _ast_visitor;
	assert(IS_AST_VISITOR(ast_visitor));
	assert(ast_visitor->visit_call_expr);

	return ast_visitor->visit_call_expr(_ast_visitor, _self);
}


static bool is_call_expr(const void *_object) {
	const struct CallExpr *object = _object;
	assert(object);
	return (object->type == CallExpr);
}


static const void *call_expr_get_lmost_token(const void *_self) {
	const struct CallExpr *self = _self;
	assert(IS_CALL_EXPR(self));

	return get_leftmost_token(self->left_expr);
}


static const void *call_expr_get_rmost_token(const void *_self) {
	const struct CallExpr *self = _self;
	assert(IS_CALL_EXPR(self));

	return self->rparen;
}


